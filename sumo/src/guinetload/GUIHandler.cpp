/****************************************************************************/
/// @file    GUIHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The XML-Handler for building networks within the gui-version
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
#include <iostream>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <netload/NLHandler.h>
#include <utils/common/TplConvert.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/common/RGBColor.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <microsim/traffic_lights/MSAgentbasedTrafficLightLogic.h>
#include <guisim/GUIInductLoop.h>
#include <guisim/GUI_E2_ZS_Collector.h>
#include <guisim/GUI_E2_ZS_CollectorOverLanes.h>
#include "GUIEdgeControlBuilder.h"
#include "GUIJunctionControlBuilder.h"
#include "GUIDetectorBuilder.h"
#include "GUIHandler.h"
#include <guisim/GUIVehicle.h>
#include <guisim/GUIVehicleType.h>
#include <guisim/GUIRoute.h>
#include <microsim/MSGlobals.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// member method definitions
// ===========================================================================
GUIHandler::GUIHandler(const std::string &file,
                       MSNet &net,
                       NLDetectorBuilder &detBuilder,
                       NLTriggerBuilder &triggerBuilder,
                       NLEdgeControlBuilder &edgeBuilder,
                       NLJunctionControlBuilder &junctionBuilder,
                       NLGeomShapeBuilder &shapeBuilder,
                       int incDUABase,
                       int incDUAStage)
        : NLHandler(file, net, detBuilder, triggerBuilder,
                    edgeBuilder, junctionBuilder, shapeBuilder, incDUABase, incDUAStage)
{}


GUIHandler::~GUIHandler() throw()
{}


void
GUIHandler::myStartElement(SumoXMLTag element,
                           const Attributes &attrs) throw(ProcessError)
{
    NLHandler::myStartElement(element, attrs);
}


void
GUIHandler::myCharacters(SumoXMLTag element,
                         const std::string &chars) throw(ProcessError)
{
    NLHandler::myCharacters(element, chars);
    if (wanted(LOADFILTER_NET)) {
        switch (element) {
        case SUMO_TAG_SHAPE:
            addJunctionShape(chars);
            break;
        default:
            break;
        }
    }
}


void
GUIHandler::addJunctionShape(const std::string &chars)
{
    try {
        Position2DVector shape = GeomConvHelper::parseShape(chars);
        static_cast<GUIJunctionControlBuilder&>(myJunctionControlBuilder).addJunctionShape(shape);
        return;
    } catch (OutOfBoundsException &) {
    } catch (NumberFormatException &) {
    } catch (EmptyData &) {
        return;
    }
    MsgHandler::getErrorInstance()->inform("Could not parse shape of junction '" + myJunctionControlBuilder.getActiveID() + "'.");
}


void
GUIHandler::addVehicleType(const Attributes &attrs)
{
    RGBColor col =
        RGBColor::parseColor(
            getStringSecure(attrs, SUMO_ATTR_COLOR, "1,1,0"));
    // !!! unsecure
    try {
        string id = getString(attrs, SUMO_ATTR_ID);
        try {
            addParsedVehicleType(id,
                                 getFloatSecure(attrs, SUMO_ATTR_LENGTH, DEFAULT_VEH_LENGTH),
                                 getFloatSecure(attrs, SUMO_ATTR_MAXSPEED, DEFAULT_VEH_MAXSPEED),
                                 getFloatSecure(attrs, SUMO_ATTR_ACCEL, DEFAULT_VEH_A),
                                 getFloatSecure(attrs, SUMO_ATTR_DECEL, DEFAULT_VEH_B),
                                 getFloatSecure(attrs, SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                 getFloatSecure(attrs, SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                 parseVehicleClass(*this, attrs, "vehicle", id),
                                 col,
                                 getFloatSecure(attrs, SUMO_ATTR_PROB, 1.));
        } catch (InvalidArgument &e) {
            MsgHandler::getErrorInstance()->inform(e.what());
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("Missing attribute in a vehicletype-object.");
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("One of an vehtype's attributes must be numeric but is not.");
        }
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("Missing id of a vehicle-object.");
    }
}


void
GUIHandler::addParsedVehicleType(const string &id, const SUMOReal length,
                                 const SUMOReal maxspeed, const SUMOReal bmax,
                                 const SUMOReal dmax, const SUMOReal sigma,
                                 SUMOReal tau,
                                 SUMOVehicleClass vclass,
                                 const RGBColor &c, SUMOReal prob)
{
    myCurrentVehicleType =
        new GUIVehicleType(c, id, length, maxspeed, bmax, dmax, sigma, tau, vclass);
    if (!MSNet::getInstance()->getVehicleControl().addVType(myCurrentVehicleType, prob)) {
        delete myCurrentVehicleType;
        myCurrentVehicleType = 0;
        if (!MSGlobals::gStateLoaded) {
            MsgHandler::getErrorInstance()->inform("Another vehicle type with the id '" + id + "' exists.");
        }
    }
}


void
GUIHandler::closeRoute() throw(ProcessError)
{
    int size = myActiveRoute.size();
    if (size==0) {
        if (myActiveRouteID[0]!='!') {
            MsgHandler::getErrorInstance()->inform("Route '" + myActiveRouteID + "' has no edges.");
        } else {
            MsgHandler::getErrorInstance()->inform("Vehicle's '" + myActiveRouteID.substr(1) + "' route has no edges.");
        }
        return;
    }
    GUIRoute *route =
        new GUIRoute(myColor, myActiveRouteID, myActiveRoute, !myAmInEmbeddedMode||myRepNumber>=1);
    myActiveRoute.clear();
    if (!MSRoute::dictionary(myActiveRouteID, route)) {
        delete route;
        if (!MSGlobals::gStateLoaded) {
            if (myActiveRouteID[0]!='!') {
                MsgHandler::getErrorInstance()->inform("Another route with the id '" + myActiveRouteID + "' exists.");
            } else {
                if (myVehicleControl.getVehicle(myActiveVehicleID)==0) {
                    MsgHandler::getErrorInstance()->inform("Another route for vehicle '" + myActiveRouteID.substr(1) + "' exists.");
                } else {
                    MsgHandler::getErrorInstance()->inform("A vehicle with id '" + myActiveRouteID.substr(1) + "' already exists.");
                }
            }
            return;
        }
    }
}


void
GUIHandler::openRoute(const Attributes &attrs)
{
    myColor =
        RGBColor::parseColor(
            getStringSecure(attrs, SUMO_ATTR_COLOR, "1,1,0"));
    MSRouteHandler::openRoute(attrs);
}


bool
GUIHandler::parseVehicleColor(SUMOSAXHandler &helper,
                              const Attributes &attrs) throw()
{
    try {
        myCurrentVehicleColor = RGBColor::parseColor(helper.getStringSecure(attrs, SUMO_ATTR_COLOR, "-1,-1,-1"));
    } catch (EmptyData &) {
        return false;
    } catch (NumberFormatException &) {
        return false;
    }
    return true;
}


bool
GUIHandler::closeVehicle() throw(ProcessError)
{
    if (MSRouteHandler::closeVehicle()) {
        GUIVehicle *vehicle = (GUIVehicle*)myVehicleControl.getVehicle(myActiveVehicleID);
        if (vehicle!=0&&myCurrentVehicleColor!=RGBColor(-1,-1,-1)) {
            vehicle->setCORNColor(myCurrentVehicleColor.red(), myCurrentVehicleColor.green(), myCurrentVehicleColor.blue());
        }
        return true;
    }
    return false;
}



/****************************************************************************/

