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
#include <utils/common/GfxConvHelper.h>
#include <utils/common/ToString.h>
#include "ROVehicleType_Krauss.h"
#include "ROVehicleBuilder.h"
#include <utils/options/OptionsCont.h>
#include "ROVehicle.h"
#include "RORouteDef_Alternatives.h"
#include "ROEdgeVector.h"
#include "RORoute.h"

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
        const std::string &file)
        : ROTypedXMLRoutesLoader(vb, net, begin, end, file),
        myDataName(dataName), myHaveNextRoute(false),
        myCurrentAlternatives(0),
        myGawronBeta(gawronBeta), myGawronA(gawronA), myMaxRouteNumber(maxRouteNumber),
        myCurrentRoute(0)
{}


RORDLoader_SUMOBase::~RORDLoader_SUMOBase() throw()
{
    // clean up (on failure)
    delete myCurrentAlternatives;
    delete myCurrentRoute;
}


void
RORDLoader_SUMOBase::myStartElement(SumoXMLTag element,
                                    const Attributes &attrs) throw(ProcessError)
{
    switch (element) {
    case SUMO_TAG_ROUTE:
        startRoute(attrs);
        break;
    case SUMO_TAG_VEHICLE:
        // try to parse the vehicle definition
        if (!SUMOBaseRouteHandler::openVehicle(*this, attrs, true)) {
            mySkipCurrent = true;
        }
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
RORDLoader_SUMOBase::startRoute(const Attributes &attrs)
{
    mySkipCurrent = false;
    if (myCurrentAlternatives==0) {
        // parse plain route...
        try {
            mySkipCurrent = false;
            if (myAmInEmbeddedMode) {
                myCurrentRouteName = getStringSecure(attrs, SUMO_ATTR_ID, "!" + myActiveVehicleID);
            } else {
                myCurrentRouteName = getString(attrs, SUMO_ATTR_ID);
            }
            myCurrentColor = parseColor(*this, attrs, "route", myCurrentRouteName);
        } catch (EmptyData &) {
            myCurrentRouteName = "";
            getErrorHandlerMarkInvalid()->inform("Missing id in route.");
        }
        return;
    }
    // parse route alternative...
    // try to get the costs
    try {
        myCost = getFloat(attrs, SUMO_ATTR_COST);
    } catch (NumberFormatException &) {
        getErrorHandlerMarkInvalid()->inform(
            "Invalid cost in alternative for route '" + myCurrentAlternatives->getID() + "' (" + getString(attrs, SUMO_ATTR_COST) + ").");
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
        myProbability = getFloatSecure(attrs, SUMO_ATTR_PROB, -10000);
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
RORDLoader_SUMOBase::startAlternative(const Attributes &attrs)
{
    // try to get the id
    string id;
    try {
        mySkipCurrent = false;
        if (myAmInEmbeddedMode) {
            id = getStringSecure(attrs, SUMO_ATTR_ID, "!" + myActiveVehicleID);
        } else {
            id = getString(attrs, SUMO_ATTR_ID);
        }
    } catch (EmptyData &) {
        getErrorHandlerMarkInvalid()->inform("Missing route alternative name.");
        return;
    }
    // try to get the index of the last element
    int index = getIntSecure(attrs, SUMO_ATTR_LAST, -1);
    if (index<0) {
        getErrorHandlerMarkInvalid()->inform("Missing or non-numeric index of a route alternative (id='" + id + "'.");
        return;
    }
    // try to get the color
    myCurrentColor = parseColor(*this, attrs, "route", myCurrentRouteName);
    // build the alternative cont
    myCurrentAlternatives = new RORouteDef_Alternatives(id, myCurrentColor,
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
    ROEdgeVector *list = new ROEdgeVector();
    StringTokenizer st(chars);
    bool ok = true;
    while (ok&&st.hasNext()) { // !!! too slow !!!
        string id = st.next();
        ROEdge *edge = myNet.getEdge(id);
        if (edge!=0) {
            list->add(edge);
        } else {
            getErrorHandlerMarkInvalid()->inform("The route '" + myCurrentAlternatives->getID() + "' contains the unknown edge '" + id + "'.");
            ok = false;
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
            myCurrentAlternatives = new RORouteDef_Alternatives(myCurrentRouteName, myCurrentColor,
                    0, myGawronBeta, myGawronA, myMaxRouteNumber);
            myCurrentAlternatives->addLoadedAlternative(myCurrentRoute);
            if (!myAmInEmbeddedMode) {
                myHaveNextRoute = true;
            }
            myNet.addRouteDef(myCurrentAlternatives);
            myCurrentRoute = 0;
            myCurrentAlternatives = 0;
        }
        break;
    case SUMO_TAG_ROUTEALT:
        if (mySkipCurrent) {
            return;
        }
        if (!myAmInEmbeddedMode) {
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


void
RORDLoader_SUMOBase::closeVehicle() throw()
{
    SUMOBaseRouteHandler::closeVehicle();
    // get the vehicle id
    if (myCurrentDepart<myBegin||myCurrentDepart>=myEnd) {
        mySkipCurrent = true;
        return;
    }
    // get vehicle type
    ROVehicleType *type = myNet.getVehicleTypeSecure(myCurrentVType);
    // get the route
    RORouteDef *route = myNet.getRouteDef(myCurrentRouteName);
    if (route==0) {
        route = myNet.getRouteDef("!" + myActiveVehicleID);
    }
    if (route==0) {
        getErrorHandlerMarkInvalid()->inform("The route of the vehicle '" + myActiveVehicleID + "' is not known.");
        return;
    }
    // get the vehicle color
    // build the vehicle
    // get further optional information
    if (!MsgHandler::getErrorInstance()->wasInformed()) {
        if (myCurrentDepart<myBegin||myCurrentDepart>=myEnd) {
            myNet.removeRouteSecure(route);
            // !!! was ist mit type?
            return;
        }
        ROVehicle *veh = myVehicleBuilder.buildVehicle(
                             myActiveVehicleID, route, myCurrentDepart, type, myCurrentVehicleColor,
                             myRepOffset, myRepNumber);
        myNet.addVehicle(myActiveVehicleID, veh);
    }
}



std::string
RORDLoader_SUMOBase::getDataName() const
{
    return myDataName;
}



void
RORDLoader_SUMOBase::startVehType(const Attributes &attrs)
{
    // get the vehicle type id
    string id;
    try {
        id = getString(attrs, SUMO_ATTR_ID);
    } catch (EmptyData &) {
        getErrorHandlerMarkInvalid()->inform("Missing id in vtype.");
        return;
    }
    // get the other values
    try {
        SUMOReal maxspeed = getFloatSecure(attrs, SUMO_ATTR_MAXSPEED, DEFAULT_VEH_MAXSPEED);
        SUMOReal length = getFloatSecure(attrs, SUMO_ATTR_LENGTH, DEFAULT_VEH_LENGTH);
        SUMOReal accel = getFloatSecure(attrs, SUMO_ATTR_ACCEL, DEFAULT_VEH_A);
        SUMOReal decel = getFloatSecure(attrs, SUMO_ATTR_DECEL, DEFAULT_VEH_B);
        SUMOReal sigma = getFloatSecure(attrs, SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA);
        SUMOReal tau = getFloatSecure(attrs, SUMO_ATTR_TAU, DEFAULT_VEH_TAU);
        RGBColor color = parseColor(*this, attrs, "vehicle type", id);
        SUMOVehicleClass vclass = parseVehicleClass(*this, attrs, "vehicle type", id);
        // build the vehicle type
        //  by now, only vehicles using the krauss model are supported
        myCurrentVehicleType = new ROVehicleType_Krauss(
            id, color, length, vclass, accel, decel, sigma, maxspeed, tau);
        myNet.addVehicleType(myCurrentVehicleType);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("At least one parameter of vehicle type '" + id + "' is not numeric, but should be.");
    }
}


SUMOTime
RORDLoader_SUMOBase::getCurrentTimeStep() const
{
    return myCurrentDepart;
}


MsgHandler *
RORDLoader_SUMOBase::getErrorHandlerMarkInvalid()
{
    mySkipCurrent = true;
    return
        OptionsCont::getOptions().getBool("continue-on-unbuild")
        ? MsgHandler::getWarningInstance()
        : MsgHandler::getErrorInstance();
}


bool
RORDLoader_SUMOBase::nextRouteRead()
{
    return myHaveNextRoute;
}


void
RORDLoader_SUMOBase::beginNextRoute()
{
    myHaveNextRoute = false;
}



/****************************************************************************/

