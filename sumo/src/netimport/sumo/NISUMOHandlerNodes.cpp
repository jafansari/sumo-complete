/****************************************************************************/
/// @file    NISUMOHandlerNodes.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A handler for SUMO nodes
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
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <netbuild/nodes/NBNode.h>
#include <netbuild/nodes/NBNodeCont.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "NISUMOHandlerNodes.h"
#include <utils/common/MsgHandler.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS
// ===========================================================================
// used namespaces
// ===========================================================================

using namespace std;

NISUMOHandlerNodes::NISUMOHandlerNodes(NBNodeCont &nc, LoadFilter what)
        : SUMOSAXHandler("sumo-network"),
        _loading(what),
        myNodeCont(nc)
{}


NISUMOHandlerNodes::~NISUMOHandlerNodes()
{}


void
NISUMOHandlerNodes::myStartElement(SumoXMLTag element, const std::string &/*name*/,
                                   const Attributes &attrs)
{
    switch (element) {
    case SUMO_TAG_JUNCTION:
        if (_loading==LOADFILTER_ALL) {
            addNode(attrs);
        }
        break;
    default:
        break;
    }
}


void
NISUMOHandlerNodes::addNode(const Attributes &attrs)
{
    string id;
    try {
        // retrieve the id of the node
        id = getString(attrs, SUMO_ATTR_ID);
        /*        string name = id;
                // retrieve the name of the node
                try {
                    name = getString(attrs, SUMO_ATTR_NAME);
                } catch (EmptyData &) {
                }*/
        string typestr;
        // get the type of the node
        try {
            typestr = getString(attrs, SUMO_ATTR_TYPE);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("The type of the junction '" + id + "' is not empty.");
            return;
        }
        // check whether the type string is valid by converting it to the known
        //  junction types
        NBNode::BasicNodeType type = NBNode::NODETYPE_UNKNOWN;
        if (typestr=="none") {
            type = NBNode::NODETYPE_NOJUNCTION;
        } else if (typestr=="priority") {
            type = NBNode::NODETYPE_PRIORITY_JUNCTION;
        } else if (typestr=="right_before_left") {
            type = NBNode::NODETYPE_RIGHT_BEFORE_LEFT;
        } else if (typestr=="DEAD_END") {
            type = NBNode::NODETYPE_DEAD_END;
        }
        if (type<0) {
            MsgHandler::getErrorInstance()->inform("The type '" + typestr + "' of junction '" + id + "is not known.");
            return;
        }
        // get the position of the node
        SUMOReal x, y;
        x = getFloatSecure(attrs, SUMO_ATTR_X, -1);
        y = getFloatSecure(attrs, SUMO_ATTR_Y, -1);
        if (x<0||y<0) {
            if (x<0) {
                MsgHandler::getErrorInstance()->inform("The x-position of the junction '" + id + "' is not valid.");
            }
            if (y<0) {
                MsgHandler::getErrorInstance()->inform("The y-position of the junction '" + id + "' is not valid.");
            }
            return;
        }
        // get the key
        string key;
        try {
            key = getString(attrs, SUMO_ATTR_KEY);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("The key is missing for junction '" + id + "'.");
        }
        // build the node
        throw 1; // !!! deprecated
        myNodeCont.insert(new NBNode(id, Position2D(x, y), type));
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("A junction without an id occured.");
    }
}


void
NISUMOHandlerNodes::myCharacters(SumoXMLTag , const std::string &,
                                 const std::string &)
{}

void
NISUMOHandlerNodes::myEndElement(SumoXMLTag , const std::string &)
{}



/****************************************************************************/

