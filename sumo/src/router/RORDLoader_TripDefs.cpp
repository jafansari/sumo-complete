/****************************************************************************/
/// @file    RORDLoader_TripDefs.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The basic class for loading trip definitions
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
#include <utils/options/OptionsCont.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/RGBColor.h>
#include <utils/common/GfxConvHelper.h>
#include "RORouteDef.h"
#include "RONet.h"
#include "RORouteDef_OrigDest.h"
#include "RORDLoader_TripDefs.h"
#include "RORouteDefList.h"
#include "ROVehicle.h"
#include "RORunningVehicle.h"
#include "RORouteDef_Complete.h"
#include "ROAbstractRouteDefLoader.h"
#include "ROVehicleBuilder.h"
#include "ROVehicleType_Krauss.h"

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
RORDLoader_TripDefs::RORDLoader_TripDefs(ROVehicleBuilder &vb, RONet &net,
        SUMOTime begin, SUMOTime end,
        bool emptyDestinationsAllowed,
        const std::string &fileName)
        : ROTypedXMLRoutesLoader(vb, net, begin, end, fileName),
        myEmptyDestinationsAllowed(emptyDestinationsAllowed),
        myDepartureTime(0), myCurrentVehicleType(0)
{}


RORDLoader_TripDefs::~RORDLoader_TripDefs() throw()
{}


void
RORDLoader_TripDefs::myStartElement(SumoXMLTag element,
                                    const Attributes &attrs) throw(ProcessError)
{
    // check whether a trip definition shall be parsed
    if (element==SUMO_TAG_TRIPDEF) {
        // get the vehicle id, the edges, the speed and position and
        //  the departure time and other information
        myID = getVehicleID(attrs);
        myDepartureTime = getTime(attrs, SUMO_ATTR_DEPART, myID);
        myBeginEdge = getEdge(attrs, "origin",
                              SUMO_ATTR_FROM, myID, false);
        myEndEdge = getEdge(attrs, "destination",
                            SUMO_ATTR_TO, myID, myEmptyDestinationsAllowed);
        myType = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
        myPos = getOptionalFloat(attrs, "pos", SUMO_ATTR_POS, myID);
        mySpeed = getOptionalFloat(attrs, "speed", SUMO_ATTR_SPEED, myID);
        myPeriodTime = getPeriod(attrs, myID);
        myNumberOfRepetitions = getRepetitionNumber(attrs, myID);
        myLane = getLane(attrs);
        myColor = getRGBColorReporting(attrs, myID);
        // recheck attributes
        if (myDepartureTime<0) {
            MsgHandler::getErrorInstance()->inform("The departure time must be positive.");
            return;
        }
    }
    // check whether a vehicle type shall be parsed
    if (element==SUMO_TAG_VTYPE) {
        // get and check the vtype-id
        string id = getStringSecure(attrs, SUMO_ATTR_ID, "");
        if (id=="") {
            MsgHandler::getErrorInstance()->inform("A vehicle type with an unknown id occured.");
            return;
        }
        // get the rest of the parameter
        try {
            SUMOReal a = getFloatSecure(attrs, SUMO_ATTR_ACCEL, DEFAULT_VEH_A);
            SUMOReal b = getFloatSecure(attrs, SUMO_ATTR_DECEL, DEFAULT_VEH_B);
            SUMOReal vmax = getFloatSecure(attrs, SUMO_ATTR_MAXSPEED, DEFAULT_VEH_MAXSPEED);
            SUMOReal length = getFloatSecure(attrs, SUMO_ATTR_LENGTH, DEFAULT_VEH_LENGTH);
            SUMOReal eps = getFloatSecure(attrs, SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA);
            SUMOReal tau = getFloatSecure(attrs, SUMO_ATTR_TAU, DEFAULT_VEH_TAU);

            RGBColor col(-1,-1,-1);
            string colordef = getStringSecure(attrs, SUMO_ATTR_COLOR, "");
            if (colordef!="") {
                try {
                    col = GfxConvHelper::parseColor(colordef);
                } catch (NumberFormatException &) {
                    MsgHandler::getErrorInstance()->inform("The color information for vehicle type '" + id + "' is not numeric.");
                } catch (...) {
                    MsgHandler::getErrorInstance()->inform("The color information for vehicle type '" + id + "' is malicious.");
                }
            }

            SUMOVehicleClass vclass = SVC_UNKNOWN;
            string classdef = getStringSecure(attrs, SUMO_ATTR_VCLASS, "");
            if (classdef!="") {
                try {
                    vclass = getVehicleClassID(classdef);
                } catch (...) {
                    MsgHandler::getErrorInstance()->inform("The vehicle class for vehicle type '" + id + "' is malicious.");
                }
            }
            myCurrentVehicleType = new ROVehicleType_Krauss(id, col, length, vclass, a, b, eps, vmax, tau);
            myNet.addVehicleType(myCurrentVehicleType);
        } catch (NumberFormatException&) {
            MsgHandler::getErrorInstance()->inform("One of the parameter for vehicle type '" + id + "' is not numeric.");
            return;
        } catch (EmptyData&) {
            MsgHandler::getErrorInstance()->inform("One of the parameter for vehicle type '" + id + "' is not given.");
            return;
        }
    }
}


std::string
RORDLoader_TripDefs::getVehicleID(const Attributes &attrs)
{
    string id;
    try {
        id = getString(attrs, SUMO_ATTR_ID);
    } catch (EmptyData &) {}
    // get a valid vehicle id
    while (id.length()==0) {
        string tmp = myIdSupplier.getNext();
        if (!myNet.isKnownVehicleID(tmp)) {
            id = tmp;
        }
    }
    // and save this vehicle id
    myNet.addVehicleID(id); // !!! what for?
    return id;
}


ROEdge *
RORDLoader_TripDefs::getEdge(const Attributes &attrs,
                             const std::string &purpose,
                             SumoXMLAttr which, const string &vid,
                             bool emptyAllowed)
{
    ROEdge *e = 0;
    string id;
    try {
        id = getString(attrs, which);
        e = myNet.getEdge(id);
        if (e!=0) {
            return e;
        }
    } catch (EmptyData &) {
        if (!emptyAllowed) {
            MsgHandler::getErrorInstance()->inform("Missing " + purpose + " edge in description of a route.");
        }
    }
    if (e==0) {
        if (!emptyAllowed) {
            MsgHandler::getErrorInstance()->inform("The edge '" + id + "' is not known.\n Vehicle id='" + vid + "'.");
        }
    }
    return 0;
}


SUMOReal
RORDLoader_TripDefs::getOptionalFloat(const Attributes &attrs,
                                      const std::string &name,
                                      SumoXMLAttr which,
                                      const std::string &place)
{
    if (!hasAttribute(attrs, which)) {
        return -1;
    }
    try {
        return getFloat(attrs, SUMO_ATTR_POS);
    } catch (EmptyData &) {} catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("The value of '" + name + "' should be numeric but is not.");
        if (place.length()!=0)
            MsgHandler::getErrorInstance()->inform(" Route id='" + place + "')");
    }
    return -1;
}


SUMOTime
RORDLoader_TripDefs::getTime(const Attributes &attrs, SumoXMLAttr which,
                             const std::string &id)
{
    // get the departure time
    try {
        return getInt(attrs, which);
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("Missing time in description of a route.");
        if (id.length()!=0)
            MsgHandler::getErrorInstance()->inform(" Vehicle id='" + id + "'.");
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("The value of the departure time should be numeric but is not.");
        if (id.length()!=0)
            MsgHandler::getErrorInstance()->inform(" Route id='" + id + "'");
    }
    return -1;
}


SUMOTime
RORDLoader_TripDefs::getPeriod(const Attributes &attrs,
                               const std::string &id)
{
    if (!hasAttribute(attrs, SUMO_ATTR_PERIOD)) {
        return -1;
    }
    // get the repetition period
    try {
        return getInt(attrs, SUMO_ATTR_PERIOD);
    } catch (EmptyData &) {
        return -1;
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("The value of the period should be numeric but is not.");
        if (id.length()!=0)
            MsgHandler::getErrorInstance()->inform(" Route id='" + id + "'");
    }
    return -1;
}


int
RORDLoader_TripDefs::getRepetitionNumber(const Attributes &attrs,
        const std::string &id)
{
    if (!hasAttribute(attrs, SUMO_ATTR_REPNUMBER)) {
        return -1;
    }
    // get the repetition period
    try {
        return getInt(attrs, SUMO_ATTR_REPNUMBER);
    } catch (EmptyData &) {
        return -1;
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("The number of cars that shall be emitted with the same parameter must be numeric.");
        if (id.length()!=0)
            MsgHandler::getErrorInstance()->inform(" Route id='" + id + "'");
    }
    return -1;
}


string
RORDLoader_TripDefs::getLane(const Attributes &attrs)
{
    try {
        return getString(attrs, SUMO_ATTR_LANE);
    } catch (EmptyData &) {
        return "";
    }
}


void
RORDLoader_TripDefs::myCharacters(SumoXMLTag element,
                                  const std::string &chars) throw(ProcessError)
{
    if (element==SUMO_TAG_TRIPDEF) {
        StringTokenizer st(chars);
        myEdges.clear();
        while (st.hasNext()) {
            string id = st.next();
            ROEdge *edge = myNet.getEdge(id);
            if (edge==0) {
                MsgHandler::getErrorInstance()->inform("Could not find edge '" + id + "' within route '" + myID + "'.");
                return;
            }
            myEdges.add(edge);
        }
    }
}


void
RORDLoader_TripDefs::myEndElement(SumoXMLTag element) throw(ProcessError)
{
    if (element==SUMO_TAG_TRIPDEF &&
            !MsgHandler::getErrorInstance()->wasInformed()) {

        if (myDepartureTime<myBegin||myDepartureTime>=myEnd) {
            return;
        }
        RORouteDef *route = 0;
        if (myEdges.size()==0) {
            route = new RORouteDef_OrigDest(myID, myColor,
                                            myBeginEdge, myEndEdge);
        } else {
            route = new RORouteDef_Complete(myID, myColor,
                                            myEdges);
        }
        ROVehicleType *type = myNet.getVehicleTypeSecure(myType);
        // check whether any errors occured
        if (MsgHandler::getErrorInstance()->wasInformed()) {
            return;
        }
        myNet.addRouteDef(route);
        myNextRouteRead = true;
        // build the vehicle
        ROVehicle *veh = 0;
        if (myPos>=0||mySpeed>=0) {
            veh = myVehicleBuilder.buildRunningVehicle(
                      myID, route, myDepartureTime,
                      type, myLane, (SUMOReal) myPos, (SUMOReal) mySpeed, myColor, myPeriodTime,
                      myNumberOfRepetitions);
        } else {
            veh = myVehicleBuilder.buildVehicle(
                      myID, route, myDepartureTime,
                      type, myColor, myPeriodTime, myNumberOfRepetitions);
        }
        myNet.addVehicle(myID, veh);
    }
}


std::string
RORDLoader_TripDefs::getDataName() const
{
    return "XML-trip definitions";
}


RGBColor
RORDLoader_TripDefs::getRGBColorReporting(const Attributes &attrs,
        const std::string &id)
{
    try {
        return GfxConvHelper::parseColor(getString(attrs, SUMO_ATTR_COLOR));
    } catch (EmptyData &) {} catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("Color in vehicle '" + id + "' is not numeric.");
    }
    return RGBColor(-1, -1, -1);
}



bool
RORDLoader_TripDefs::nextRouteRead()
{
    return myNextRouteRead;
}


void
RORDLoader_TripDefs::beginNextRoute()
{
    myNextRouteRead = false;
}


SUMOTime
RORDLoader_TripDefs::getCurrentTimeStep() const
{
    return myDepartureTime;
}



/****************************************************************************/

