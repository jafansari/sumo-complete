/****************************************************************************/
/// @file    MSRouteHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Parser and container for routes during their loading
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
#include <microsim/MSRoute.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEmitControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSLane.h>
#include "MSRouteHandler.h"
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/options/OptionsCont.h>
#include "MSNet.h"

#include <microsim/trigger/MSTriggerControl.h>
#include <microsim/trigger/MSBusStop.h>
#include <microsim/MSGlobals.h>
#include <utils/xml/SUMOVehicleParserHelper.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
MSRouteHandler::MSRouteHandler(const std::string &file,
                               MSVehicleControl &vc,
                               bool addVehiclesDirectly,
                               int incDUABase,
                               int incDUAStage)
        : SUMOSAXHandler(file),
        myVehicleControl(vc), myVehicleParameter(0),
        myLastDepart(0), myLastReadVehicle(0),
        myAddVehiclesDirectly(addVehiclesDirectly),
        myAmUsingIncrementalDUA(incDUAStage>0),
        myRunningVehicleNumber(0),
        myIncrementalBase(incDUABase),
        myIncrementalStage(incDUAStage)
{
    myActiveRoute.reserve(100);
}


MSRouteHandler::~MSRouteHandler() throw()
{}


SUMOTime
MSRouteHandler::getLastDepart() const
{
    return myLastDepart;
}


MSVehicle *
MSRouteHandler::retrieveLastReadVehicle()
{
    MSVehicle *v = myLastReadVehicle;
    myLastReadVehicle = 0;
    return v;
}


void
MSRouteHandler::myStartElement(SumoXMLTag element,
                               const SUMOSAXAttributes &attrs) throw(ProcessError)
{
    switch (element) {
    case SUMO_TAG_VEHICLE:
        myVehicleParameter = SUMOVehicleParserHelper::parseVehicleAttributes(attrs);
        break;
    case SUMO_TAG_VTYPE:
        addVehicleType(attrs);
        break;
    case SUMO_TAG_ROUTE:
        openRoute(attrs);
        break;
    default:
        break;
    }

    if (element==SUMO_TAG_STOP) {
        MSVehicle::Stop stop;
        stop.lane = 0;
        stop.busstop = 0;
        // try to parse the assigne bus stop
        string bus_stop = attrs.getStringSecure(SUMO_ATTR_BUS_STOP, "");
        if (bus_stop!="") {
            // ok, we have obviously a bus stop
            MSBusStop *bs =
                (MSBusStop*) MSNet::getInstance()->getTriggerControl().getTrigger(bus_stop);
            if (bs!=0) {
                const MSLane &l = bs->getLane();
                stop.lane = &((MSLane &) l);
                stop.busstop = bs;
                stop.pos = bs->getEndLanePosition();
            } else {
                MsgHandler::getErrorInstance()->inform("The bus stop '" + bus_stop + "' is not known.");
                return;
            }
        } else {
            // no, the lane and the position should be given
            // get the lane
            string laneS = attrs.getStringSecure(SUMO_ATTR_LANE, "");
            if (laneS!="") {
                MSLane *l = MSLane::dictionary(laneS);
                if (l==0) {
                    MsgHandler::getErrorInstance()->inform("The lane '" + laneS + "' for a stop is not known.");
                    return;
                }
                stop.lane = l;
            } else {
                MsgHandler::getErrorInstance()->inform("A stop must be placed on a bus stop or a lane.");
                return;
            }
            // get the position
            try {
                stop.pos = attrs.getFloat(SUMO_ATTR_POSITION);
            } catch (EmptyData&) {
                MsgHandler::getErrorInstance()->inform("The position of a stop is not defined.");
                return;
            } catch (NumberFormatException&) {
                MsgHandler::getErrorInstance()->inform("The position of a stop is not numeric.");
                return;
            }
        }

        // get the standing duration
        stop.until = -1;
        stop.duration = -1;
        if (!attrs.hasAttribute(SUMO_ATTR_DURATION) && !attrs.hasAttribute(SUMO_ATTR_UNTIL)) {
            MsgHandler::getErrorInstance()->inform("The duration of a stop is not defined.");
            return;
        } else {
            try {
                stop.duration = (SUMOTime) attrs.getFloatSecure(SUMO_ATTR_DURATION, -1); // time-parser
            } catch (EmptyData&) {
                MsgHandler::getErrorInstance()->inform("The duration of a stop is empty.");
                return;
            } catch (NumberFormatException&) {
                MsgHandler::getErrorInstance()->inform("The duration of a stop is not numeric.");
                return;
            }
            try {
                stop.until = (SUMOTime) attrs.getFloatSecure(SUMO_ATTR_UNTIL, -1); // time-parser
            } catch (EmptyData&) {
                MsgHandler::getErrorInstance()->inform("The end time of a stop is empty.");
                return;
            } catch (NumberFormatException&) {
                MsgHandler::getErrorInstance()->inform("The end time of a stop is not numeric.");
                return;
            }
            if(stop.duration<0&&stop.until<0) {
                MsgHandler::getErrorInstance()->inform("Neither the duration nor the end time is given for a stop.");
                return;
            }
        }
        stop.reached = false;
        myVehicleStops.push_back(stop);
    }
}


void
MSRouteHandler::addVehicleType(const SUMOSAXAttributes &attrs)
{
    string id;
    if(attrs.setIDFromAttributes("vtype", id)) {
        try {
            addParsedVehicleType(id,
                                 attrs.getFloatSecure(SUMO_ATTR_LENGTH, DEFAULT_VEH_LENGTH),
                                 attrs.getFloatSecure(SUMO_ATTR_MAXSPEED, DEFAULT_VEH_MAXSPEED),
                                 attrs.getFloatSecure(SUMO_ATTR_ACCEL, DEFAULT_VEH_A),
                                 attrs.getFloatSecure(SUMO_ATTR_DECEL, DEFAULT_VEH_B),
                                 attrs.getFloatSecure(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                 attrs.getFloatSecure(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                 SUMOVehicleParserHelper::parseVehicleClass(attrs, "vtype", id),
                                 attrs.getFloatSecure(SUMO_ATTR_PROB, (SUMOReal) 1.));
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("Missing attribute in a vehicletype-object.");
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("One of an vehtype's attributes must be numeric but is not.");
        }
    }
}


void
MSRouteHandler::addParsedVehicleType(const string &id, const SUMOReal length,
                                     const SUMOReal maxspeed, const SUMOReal bmax,
                                     const SUMOReal dmax, const SUMOReal sigma,
                                     SUMOReal tau,
                                     SUMOVehicleClass vclass, SUMOReal prob)
{
    myCurrentVehicleType = new MSVehicleType(id, length, maxspeed, bmax, dmax, sigma, tau, vclass);
    if (!MSNet::getInstance()->getVehicleControl().addVType(myCurrentVehicleType, prob)) {
        delete myCurrentVehicleType;
        myCurrentVehicleType = 0;
#ifdef HAVE_MESOSIM
        if (!MSGlobals::gStateLoaded) {
#endif
            throw ProcessError("Another vehicle type with the id '" + id + "' exists.");
#ifdef HAVE_MESOSIM
        }
#endif
    }
}


void
MSRouteHandler::openRoute(const SUMOSAXAttributes &attrs)
{
    // get the id
    try {
        // check whether the id is really necessary
        if (myVehicleParameter!=0) {
            // ok, a vehicle is wrapping the route,
            //  we may use this vehicle's id as default
            myActiveRouteID = "!" + myVehicleParameter->id; // !!! document this
        } else {
            myActiveRouteID = attrs.getString(SUMO_ATTR_ID);
        }
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("Missing id of a route-object.");
        return;
    }
    if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        addRouteElements(attrs.getString(SUMO_ATTR_EDGES));
    }
}


// ----------------------------------


void
MSRouteHandler::myCharacters(SumoXMLTag element,
                             const std::string &chars) throw(ProcessError)
{
    switch (element) {
    case SUMO_TAG_ROUTE:
        addRouteElements(chars);
        break;
    default:
        break;
    }
}


void
MSRouteHandler::addRouteElements(const std::string &chars)
{
    StringTokenizer st(chars);
    MSEdge *edge = 0;
    while (st.hasNext()) {
        string set = st.next();
        edge = MSEdge::dictionary(set);
        // check whether the edge exists
        if (edge==0) {
            throw ProcessError("The edge '" + set + "' within route '" + myActiveRouteID + "' is not known."
                               + "\n The route can not be build.");
        }
        myActiveRoute.push_back(edge);
    }
}


// ----------------------------------

void
MSRouteHandler::myEndElement(SumoXMLTag element) throw(ProcessError)
{
    if (element == SUMO_TAG_ROUTE) {
        closeRoute();
    } else if (element == SUMO_TAG_VEHICLE) {
        closeVehicle();
    }
}


void
MSRouteHandler::closeRoute() throw(ProcessError)
{
    if (myActiveRoute.size()==0) {
        if (myVehicleParameter!=0) {
            throw ProcessError("Vehicle's '" + myVehicleParameter->id + "' route has no edges.");
        } else {
            throw ProcessError("Route '" + myActiveRouteID + "' has no edges.");
        }
    }
    // check whether the route is long enough
#ifndef NEW_SPEC
    if (myActiveRoute.size()<2) {
        throw ProcessError("SUMO assumes each route to be at least two edges long ('" + myActiveRouteID + "' has " + toString(myActiveRoute.size()) + ").");
    }
#endif
    MSRoute *route = new MSRoute(myActiveRouteID, myActiveRoute, myVehicleParameter==0||myVehicleParameter->repetitionNumber>=1);
    myActiveRoute.clear();
    if (!MSRoute::dictionary(myActiveRouteID, route)) {
        delete route;
#ifdef HAVE_MESOSIM
        if (!MSGlobals::gStateLoaded) {
#endif
            if (myVehicleParameter!=0) {
                if (myVehicleControl.getVehicle(myVehicleParameter->id)==0) {
                    throw ProcessError("Another route for vehicle '" + myVehicleParameter->id + "' exists.");
                } else {
                    throw ProcessError("A vehicle with id '" + myVehicleParameter->id + "' already exists.");
                }
            } else {
                throw ProcessError("Another route with the id '" + myActiveRouteID + "' exists.");
            }
#ifdef HAVE_MESOSIM
        }
#endif
    }
}


bool
MSRouteHandler::closeVehicle() throw(ProcessError)
{
    myLastDepart = myVehicleParameter->depart;
    // let's check whether this vehicle had to be emitted before the simulation starts
    if (myVehicleParameter->depart<OptionsCont::getOptions().getInt("begin")) {
        // yes, but maybe it's a repeating vehicle...
        if (myVehicleParameter->repetitionNumber>=0&&myVehicleParameter->repetitionOffset>=0) {
            while (myVehicleParameter->depart<OptionsCont::getOptions().getInt("begin") && myVehicleParameter->repetitionNumber>=0) {
                --myVehicleParameter->repetitionNumber;
                myVehicleParameter->depart += myVehicleParameter->repetitionOffset;
            }
        }
        if (myVehicleParameter->depart<OptionsCont::getOptions().getInt("begin")) {
            myVehicleStops.clear();
            return false;
        }
    }
    // get the vehicle's type
    MSVehicleType *vtype = 0;
    if (myVehicleParameter->vtypeid!="") {
        vtype = MSNet::getInstance()->getVehicleControl().getVType(myVehicleParameter->vtypeid);
        if (vtype==0) {
            throw ProcessError("The vehicle type '" + myVehicleParameter->vtypeid + "' for vehicle '" + myVehicleParameter->id + "' is not known.");

        }
    } else {
        // there should be one (at least the default one)
        vtype = MSNet::getInstance()->getVehicleControl().getRandomVType();
    }
    // get the vehicle's route
    //  maybe it was explicitely assigned to the vehicle
    MSRoute *route = MSRoute::dictionary(myActiveRouteID);
    if (route==0) {
        // if not, try via the (hopefully) given route-id
        route = MSRoute::dictionary(myVehicleParameter->routeid);
    }
    if (route==0) {
        // nothing found? -> error
        if (myVehicleParameter->routeid!="") {
            throw ProcessError("The route '" + myVehicleParameter->routeid + "' for vehicle '" + myVehicleParameter->id + "' is not known.");
        } else {
            throw ProcessError("Vehicle '" + myVehicleParameter->id + "' has no route.");
        }
    }
    myActiveRouteID = "";

    // try to build the vehicle
    MSVehicle *vehicle = 0;
    if (myVehicleControl.getVehicle(myVehicleParameter->id)==0) {
        // ok there was no other vehicle with the same id, yet
        // maybe we do not want this vehicle to be emitted due to using incremental dua
        bool add = true;
        if (myAmUsingIncrementalDUA) {
            if ((int)(myRunningVehicleNumber%myIncrementalBase)>=(int) myIncrementalStage) {
                add = false;
            }
            myRunningVehicleNumber++;
        }
        if (add) {
            vehicle =
                MSNet::getInstance()->getVehicleControl().buildVehicle(*myVehicleParameter, route, vtype);
            // add the vehicle to the vehicle control
            myVehicleControl.addVehicle(myVehicleParameter->id, vehicle);
        }
    } else {
        // strange: another vehicle with the same id already exists
#ifdef HAVE_MESOSIM
        if (!MSGlobals::gStateLoaded) {
#endif
            // and was not loaded while loading a simulation state
            // -> error
            throw ProcessError("Another vehicle with the id '" + myVehicleParameter->id + "' exists.");
#ifdef HAVE_MESOSIM
        } else {
            // ok, it seems to be loaded previously while loading a simulation state
            vehicle = 0;
        }
#endif
    }
    // check whether the vehicle shall be added directly to the network or
    //  shall stay in the internal buffer
    if (myAddVehiclesDirectly&&vehicle!=0) {
        MSNet::getInstance()->myEmitter->add(vehicle);
    } else {
        myLastReadVehicle = vehicle;
    }
    if (vehicle!=0) {
        for (std::vector<MSVehicle::Stop>::iterator i=myVehicleStops.begin(); i!=myVehicleStops.end(); ++i) {
            vehicle->addStop(*i);
        }
    }
    myVehicleStops.clear();
    delete myVehicleParameter;
    myVehicleParameter = 0;
    return true;
}



/****************************************************************************/
