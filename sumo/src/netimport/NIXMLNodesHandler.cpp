/****************************************************************************/
/// @file    NIXMLNodesHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id:NIXMLNodesHandler.cpp 4701 2007-11-09 14:29:29Z dkrajzew $
///
// Importer for network nodes stored in XML
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
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
#include "NIXMLNodesHandler.h"
#include <utils/xml/SUMOSAXHandler.h>
#include <netbuild/NBNodeCont.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/StringTokenizer.h>
#include <utils/options/OptionsCont.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <netbuild/NBOwnTLDef.h>
#include <utils/geom/GeoConvHelper.h>

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
NIXMLNodesHandler::NIXMLNodesHandler(NBNodeCont &nc,
                                     NBTrafficLightLogicCont &tlc,
                                     OptionsCont &options)
        : SUMOSAXHandler("xml-nodes - file"),
        myOptions(options),
        myNodeCont(nc), myTLLogicCont(tlc)
{}


NIXMLNodesHandler::~NIXMLNodesHandler() throw()
{}


void
NIXMLNodesHandler::myStartElement(SumoXMLTag element,
                                  const SUMOSAXAttributes &attrs) throw(ProcessError)
{
    if (element!=SUMO_TAG_NODE) {
        return;
    }
    // get the id, report a warning if not given or empty...
    if (!attrs.setIDFromAttributes("node", myID), false) {
        WRITE_WARNING("No node id given... Skipping.");
        return;
    }
    // retrieve the name of the node
    string name = attrs.getStringSecure(SUMO_ATTR_NAME, myID);
    // retrieve the position of the node
    if (!setPosition(attrs)) {
        return;
    }
    // get the type
    myType = attrs.getStringSecure(SUMO_ATTR_TYPE, "");
    NBNode::BasicNodeType type = NBNode::NODETYPE_UNKNOWN;
    if (myType=="priority") {
        type = NBNode::NODETYPE_PRIORITY_JUNCTION;
    } else if (myType=="right_before_left"||myType=="right_for_left") {
        type = NBNode::NODETYPE_RIGHT_BEFORE_LEFT;
    } else if (myType=="traffic_light") {
        type = NBNode::NODETYPE_TRAFFIC_LIGHT;
    }
    // check whether there is a traffic light to assign this node to
    // build the node
    NBNode *node = new NBNode(myID, myPosition, type);
    // insert the node
    if (!myNodeCont.insert(node)) {
        if (myNodeCont.retrieve(myPosition)!=0) {
            MsgHandler::getErrorInstance()->inform("Duplicate node occured. ID='" + myID + "'");
        }
    }
    // process traffic light definition
    if (type==NBNode::NODETYPE_TRAFFIC_LIGHT) {
        processTrafficLightDefinitions(attrs, node);
    }
}



bool
NIXMLNodesHandler::setPosition(const SUMOSAXAttributes &attrs)
{
    // retrieve the positions
    try {
        SUMOReal x = attrs.getFloat(SUMO_ATTR_X);
        SUMOReal y = attrs.getFloat(SUMO_ATTR_Y);
        myPosition.set(x, y);
        GeoConvHelper::x2cartesian(myPosition);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform("Not numeric value for position (at node ID='" + myID + "').");
        return false;
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("Node position (at node ID='" + myID + "') is not given.");
        return false;
    }
    // check whether the y-axis shall be flipped
    if (myOptions.getBool("flip-y")) {
        myPosition.mul(1.0, -1.0);
    }
    return true;
}


void
NIXMLNodesHandler::processTrafficLightDefinitions(const SUMOSAXAttributes &attrs,
        NBNode *currentNode)
{
    // try to get the tl-id
    // if a tl-id is given, we will look whether this tl already exists
    //  if so, we will add the node to it, otherwise allocate a new one with this id
    // if no tl-id exists, we will build a tl with the node's id
    NBTrafficLightDefinition *tlDef = 0;
    string tlID = attrs.getStringSecure(SUMO_ATTR_TLID, "");
    if (tlID!="") {
        // ok, the traffic light has a name
        tlDef = myTLLogicCont.getDefinition(tlID);
        if (tlDef==0) {
            // this traffic light is visited the first time
            tlDef = new NBOwnTLDef(tlID, currentNode);
            if (!myTLLogicCont.insert(tlDef)) {
                // actually, nothing should fail here
                delete tlDef;
                throw ProcessError("Could not allocate tls '" + tlID + "'.");

            }
        } else {
            tlDef->addNode(currentNode);
        }
    } else {
        // ok, this node is a traffic light node where no other nodes
        //  participate
        tlDef = new NBOwnTLDef(myID, currentNode);
        if (!myTLLogicCont.insert(tlDef)) {
            // actually, nothing should fail here
            delete tlDef;
            throw ProcessError("Could not allocate tls '" + myID + "'.");

        }
    }

    // process inner edges which shall be controlled
    string controlledInner = attrs.getStringSecure(SUMO_ATTR_CONTROLLED_INNER, "");
    if (controlledInner!="") {
        StringTokenizer st(controlledInner, ";");
        tlDef->addControlledInnerEdges(st.getVector());
    }
}



/****************************************************************************/

