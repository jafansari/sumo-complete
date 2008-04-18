/****************************************************************************/
/// @file    RORDLoader_SUMOBase.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The base class for SUMO-native route handlers
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

#include "RORDLoader_SUMOBase.h"
#include "ROVehicleType.h"
#include "RORouteDef.h"
#include "RONet.h"
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/ToString.h>
#include "ROVehicleType_Krauss.h"
#include "ROVehicleBuilder.h"
#include <utils/options/OptionsCont.h>
#include "ROVehicle.h"
#include "RORouteDef_Alternatives.h"
#include "RORouteDef_Complete.h"
#include "RORoute.h"
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
RORDLoader_SUMOBase::RORDLoader_SUMOBase(ROVehicleBuilder &vb, RONet &net,
        SUMOTime begin, SUMOTime end,
        SUMOReal gawronBeta, SUMOReal gawronA,
        int maxRouteNumber,
        const std::string &dataName,
        const std::string &file) throw(ProcessError)
        : ROTypedXMLRoutesLoader(vb, net, begin, end, file),
        myVehicleParameter(0), myDataName(dataName), myHaveNextRoute(false),
        myCurrentAlternatives(0),
        myGawronBeta(gawronBeta), myGawronA(gawronA), myMaxRouteNumber(maxRouteNumber),
        myCurrentRoute(0), myCurrentDepart(-1)
{}


RORDLoader_SUMOBase::~RORDLoader_SUMOBase() throw()
{
    // clean up (on failure)
    delete myCurrentAlternatives;
    delete myCurrentRoute;
}


void
RORDLoader_SUMOBase::myStartElement(SumoXMLTag element,
                                    const SUMOSAXAttributes &attrs) throw(ProcessError)
{
    switch (element) {
    case SUMO_TAG_ROUTE:
        startRoute(attrs);
        break;
    case SUMO_TAG_VEHICLE:
        // try to parse the vehicle definition
        myVehicleParameter = SUMOVehicleParserHelper::parseVehicleAttributes(attrs);
        if(myVehicleParameter!=0) {
            myCurrentDepart = myVehicleParameter->depart;
        }
        mySkipCurrent = myVehicleParameter==0;
        break;
    case SUMO_TAG_VTYPE:
        myCurrentVehicleType = 0;
        startVehType(attrs);
        break;
    case SUMO_TAG_ROUTEALT:
        startAlternative(attrs);
        break;
    default:
        break;
    }
}


void
RORDLoader_SUMOBase::startRoute(const SUMOSAXAttributes &attrs)
{
    mySkipCurrent = false;
    if (myCurrentAlternatives==0) {
        // parse plain route...
        try {
            if (myVehicleParameter!=0) {
                myCurrentRouteName = attrs.getStringSecure(SUMO_ATTR_ID, "!" + myVehicleParameter->id);
            } else {
                myCurrentRouteName = attrs.getString(SUMO_ATTR_ID);
            }
            myColor = RGBColor::parseColor(attrs.getStringSecure(SUMO_ATTR_COLOR, "-1,-1,-1"));
        } catch (EmptyData &) {
            myCurrentRouteName = "";
            getErrorHandlerMarkInvalid()->inform("Missing id in route.");
        }
        return;
    }
    // parse route alternative...
    // try to get the costs
    try {
        myCost = attrs.getFloat(SUMO_ATTR_COST);
    } catch (NumberFormatException &) {
        getErrorHandlerMarkInvalid()->inform(
            "Invalid cost in alternative for route '" + myCurrentAlternatives->getID() + "' (" + attrs.getString(SUMO_ATTR_COST) + ").");
        mySkipCurrent = true;
        return;
    } catch (EmptyData &) {
        getErrorHandlerMarkInvalid()->inform("Missing cost in alternative for route '" + myCurrentAlternatives->getID() + "'.");
        mySkipCurrent = true;
        return;
    }
    if (myCost<0) {
        getErrorHandlerMarkInvalid()->inform("Invalid cost in alternative for route '" + myCurrentAlternatives->getID() + "' (" + toString<SUMOReal>(myCost) + ").");
        mySkipCurrent = true;
        return;
    }
    // try to get the probability
    try {
        myProbability = attrs.getFloatSecure(SUMO_ATTR_PROB, -10000);
    } catch (NumberFormatException &) {
        getErrorHandlerMarkInvalid()->inform("Invalid probability in alternative for route '" + myCurrentAlternatives->getID() + "' (" + toString<SUMOReal>(myProbability) + ").");
        mySkipCurrent = true;
        return;
    } catch (EmptyData &) {
        getErrorHandlerMarkInvalid()->inform("Missing probability in alternative for route '" + myCurrentAlternatives->getID() + "'.");
        mySkipCurrent = true;
        return;
    }
    if (myProbability<0) {
        getErrorHandlerMarkInvalid()->inform("Invalid probability in alternative for route '" + myCurrentAlternatives->getID() + "' (" + toString<SUMOReal>(myProbability) + ").");
        mySkipCurrent = true;
        return;
    }
}


void
RORDLoader_SUMOBase::startAlternative(const SUMOSAXAttributes &attrs)
{
    // try to get the id
    string id;
    try {
        mySkipCurrent = false;
        if (myVehicleParameter!=0) {
            id = attrs.getStringSecure(SUMO_ATTR_ID, "!" + myVehicleParameter->id);
        } else {
            id = attrs.getString(SUMO_ATTR_ID);
        }
    } catch (EmptyData &) {
        getErrorHandlerMarkInvalid()->inform("Missing route alternative name.");
        return;
    }
    // try to get the index of the last element
    int index = attrs.getIntSecure(SUMO_ATTR_LAST, -1);
    if (index<0) {
        getErrorHandlerMarkInvalid()->inform("Missing or non-numeric index of a route alternative (id='" + id + "'.");
        return;
    }
    // try to get the color
    myColor = RGBColor::parseColor(attrs.getStringSecure(SUMO_ATTR_COLOR, "-1,-1,-1"));
    // build the alternative cont
    myCurrentAlternatives = new RORouteDef_Alternatives(id, myColor,
            index, myGawronBeta, myGawronA, myMaxRouteNumber);
}

void
RORDLoader_SUMOBase::myCharacters(SumoXMLTag element,
                                  const std::string &chars) throw(ProcessError)
{
    // process routes only, all other elements do
    //  not have embedded characters
    if (element!=SUMO_TAG_ROUTE) {
        return;
    }
    // check whether the costs and the probability are valid
    if (myCurrentAlternatives!=0) {
        if (myCost<0||myProbability<0||mySkipCurrent) {
            return;
        }
    }
    // build the list of edges
    std::vector<const ROEdge*> *list = new std::vector<const ROEdge*>();
    StringTokenizer st(chars);
    bool ok = true;
    while (ok&&st.hasNext()) { // !!! too slow !!!
        string id = st.next();
        ROEdge *edge = myNet.getEdge(id);
        if (edge!=0) {
            list->push_back(edge);
        } else {
            if(false) {
                getErrorHandlerMarkInvalid()->inform("The route '" + myCurrentAlternatives->getID() + "' contains the unknown edge '" + id + "'.");
                ok = false;
            }
        }
    }
    if (ok) {
        if (myCurrentAlternatives!=0) {
            myCurrentAlternatives->addLoadedAlternative(
                new RORoute(myCurrentAlternatives->getID(), myCost, myProbability, *list));
        } else {
            myCurrentRoute = new RORoute(myCurrentRouteName, 0, 1, *list);
        }
    }
    delete list;
}


void
RORDLoader_SUMOBase::myEndElement(SumoXMLTag element) throw(ProcessError)
{
    switch (element) {
    case SUMO_TAG_ROUTE:
        if (myCurrentRoute!=0&&!mySkipCurrent) {
            if(myCurrentAlternatives==0) {
                myCurrentAlternatives = new RORouteDef_Alternatives(myCurrentRouteName, myColor,
                        0, myGawronBeta, myGawronA, myMaxRouteNumber);
                myNet.addRouteDef(myCurrentAlternatives);
                myCurrentAlternatives->addLoadedAlternative(myCurrentRoute);
                myCurrentAlternatives = 0;
            } else {
                myCurrentAlternatives->addLoadedAlternative(myCurrentRoute);
            }
            if (myVehicleParameter==0) {
                myHaveNextRoute = true;
            }
            myCurrentRoute = 0;
        }
        break;
    case SUMO_TAG_ROUTEALT:
        if (mySkipCurrent) {
            return;
        }
        if (myVehicleParameter==0) {
            myHaveNextRoute = true;
        }
        myNet.addRouteDef(myCurrentAlternatives);
        myCurrentRoute = 0;
        myCurrentAlternatives = 0;
        break;
    case SUMO_TAG_VEHICLE:
        closeVehicle();
        myHaveNextRoute = true;
        break;
    default:
        break;
    }
}


bool
RORDLoader_SUMOBase::closeVehicle() throw()
{
    // get the vehicle id
    if (myVehicleParameter->depart<myBegin||myVehicleParameter->depart>=myEnd) {
        mySkipCurrent = true;
        return false;
    }
    // get vehicle type
    ROVehicleType *type = myNet.getVehicleTypeSecure(myVehicleParameter->vtypeid);
    // get the route
    RORouteDef *route = myNet.getRouteDef(myVehicleParameter->routeid);
    if (route==0) {
        route = myNet.getRouteDef("!" + myVehicleParameter->id);
    }
    if (route==0) {
        getErrorHandlerMarkInvalid()->inform("The route of the vehicle '" + myVehicleParameter->id + "' is not known.");
        return false;
    }
    // get the vehicle color
    // build the vehicle
    // get further optional information
    if (!MsgHandler::getErrorInstance()->wasInformed()) {
        if (myVehicleParameter->depart<myBegin||myVehicleParameter->depart>=myEnd) {
            myNet.removeRouteSecure(route);
            // !!! was ist mit type?
            return false;
        }
        ROVehicle *veh = myVehicleBuilder.buildVehicle(*myVehicleParameter, route, type);
        myNet.addVehicle(myVehicleParameter->id, veh);
        return true;
    }
    return false;
}


void
RORDLoader_SUMOBase::startVehType(const SUMOSAXAttributes &attrs)
{
    // get the id, report an error if not given or empty...
    string id;
    if(!attrs.setIDFromAttribues("vtype", id, false)) {
        getErrorHandlerMarkInvalid()->inform("Missing id in vtype.");
        return;
    }
    // get the other values
    try {
        SUMOReal maxspeed = attrs.getFloatSecure(SUMO_ATTR_MAXSPEED, DEFAULT_VEH_MAXSPEED);
        SUMOReal length = attrs.getFloatSecure(SUMO_ATTR_LENGTH, DEFAULT_VEH_LENGTH);
        SUMOReal accel = attrs.getFloatSecure(SUMO_ATTR_ACCEL, DEFAULT_VEH_A);
        SUMOReal decel = attrs.getFloatSecure(SUMO_ATTR_DECEL, DEFAULT_VEH_B);
        SUMOReal sigma = attrs.getFloatSecure(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA);
        SUMOReal tau = attrs.getFloatSecure(SUMO_ATTR_TAU, DEFAULT_VEH_TAU);
        std::string color = attrs.getStringSecure(SUMO_ATTR_COLOR, "");
        SUMOVehicleClass vclass = SUMOVehicleParserHelper::parseVehicleClass(attrs, "vtype", id);
        // build the vehicle type
        //  by now, only vehicles using the krauss model are supported
        myCurrentVehicleType = new ROVehicleType_Krauss(
            id, color, length, vclass, accel, decel, sigma, maxspeed, tau);
        myNet.addVehicleType(myCurrentVehicleType);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("At least one parameter of vehicle type '" + id + "' is not numeric, but should be.");
    }
}


MsgHandler *
RORDLoader_SUMOBase::getErrorHandlerMarkInvalid() throw()
{
    mySkipCurrent = true;
    return
        OptionsCont::getOptions().getBool("continue-on-unbuild")
        ? MsgHandler::getWarningInstance()
        : MsgHandler::getErrorInstance();
}


void
RORDLoader_SUMOBase::beginNextRoute() throw()
{
    myHaveNextRoute = false;
}



/****************************************************************************/

