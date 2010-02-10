/****************************************************************************/
/// @file    NGNode.cpp
/// @author  Markus Hartinger
/// @date    Mar, 2003
/// @version $Id$
///
// A netgen-representation of a node
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
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

#include <algorithm>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBOwnTLDef.h>
#include <netbuild/NBTypeCont.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include "NGNode.h"

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
NGNode::NGNode() throw()
        : xID(-1), yID(-1), myID(""), myAmCenter(false) {}


NGNode::NGNode(const std::string &id) throw()
        : xID(-1), yID(-1), myID(id), myAmCenter(false) {}


NGNode::NGNode(const std::string &id, int xIDa, int yIDa) throw()
        : xID(xIDa), yID(yIDa), myID(id), myAmCenter(false) {}


NGNode::NGNode(const std::string &id, int xIDa, int yIDa, bool amCenter) throw()
        : xID(xIDa), yID(yIDa), myID(id), myAmCenter(amCenter) {}


NGNode::~NGNode() throw() {
    NGEdgeList::iterator li;
    while (LinkList.size() != 0) {
        li = LinkList.begin();
        delete(*li);
    }
}


NBNode *
NGNode::buildNBNode(NBNetBuilder &nb) const throw(ProcessError) {
    Position2D pos(myPosition);
    GeoConvHelper::x2cartesian(pos);
    // the center will have no logic!
    if (myAmCenter) {
        return new NBNode(myID, pos, NBNode::NODETYPE_NOJUNCTION);
    }
    //
    // check whether it is a traffic light junction
    string nodeType = OptionsCont::getOptions().isSet("default-junction-type")
                      ? OptionsCont::getOptions().getString("default-junction-type")
                      : "";
    NBNode *node = 0;
    if (nodeType=="") {
        node = new NBNode(myID, pos);
    } else if (nodeType=="priority") {
        node = new NBNode(myID, pos, NBNode::NODETYPE_PRIORITY_JUNCTION);
    } else if (nodeType=="right_before_left") {
        node = new NBNode(myID, pos, NBNode::NODETYPE_RIGHT_BEFORE_LEFT);
    } else if (nodeType=="traffic_light") {
        node = new NBNode(myID, pos, NBNode::NODETYPE_PRIORITY_JUNCTION);
        NBTrafficLightDefinition *tlDef = new NBOwnTLDef(myID, node);
        if (!nb.getTLLogicCont().insert(tlDef)) {
            // actually, nothing should fail here
            delete tlDef;
            throw ProcessError();
        }
    }
    return node;
}


void
NGNode::addLink(NGEdge *link) throw() {
    LinkList.push_back(link);
}


void
NGNode::removeLink(NGEdge *link) throw() {
    LinkList.remove(link);
}


bool
NGNode::connected(NGNode *node) const throw() {
    for (NGEdgeList::const_iterator i=LinkList.begin(); i!=LinkList.end(); i++) {
        if (find(node->LinkList.begin(), node->LinkList.end(), *i)!=node->LinkList.end()) {
            return true;
        }
    }
    return false;
}


/****************************************************************************/

