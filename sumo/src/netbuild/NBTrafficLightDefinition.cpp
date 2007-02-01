/****************************************************************************/
/// @file    NBTrafficLightDefinition.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: $
///
// The definition of a traffic light logic
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
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "NBTrafficLightDefinition.h"
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsSubSys.h>
#include "NBTrafficLightLogicVector.h"
#include "NBTrafficLightDefinition.h"
#include "NBTrafficLightPhases.h"
#include "NBLinkPossibilityMatrix.h"
#include "NBTrafficLightLogic.h"
#include "NBContHelper.h"
#include "NBRequestEdgeLinkIterator.h"
#include "NBLinkCliqueContainer.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// some definitions (debugging only)
// ===========================================================================
#define DEBUG_OUT cout


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * NBTrafficLightDefinition
 * ----------------------------------------------------------------------- */
NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string &id,
        const std::set<NBNode*> &junctions)
            : Named(id), myType("static"), _nodes(junctions)
{
    for (NodeCont::const_iterator i=junctions.begin(); i!=junctions.end(); i++) {
        (*i)->addTrafficLight(this);
    }
}


NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string &id,
        NBNode *junction)
        : Named(id), myType("static")
{
    addNode(junction);
    junction->addTrafficLight(this);
}


NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string &id,
        std::string type,
        NBNode *junction)
        : Named(id), myType(type)
{
    addNode(junction);
    junction->addTrafficLight(this);
}


NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string &id)
        : Named(id), myType("static")
{}


NBTrafficLightDefinition::~NBTrafficLightDefinition()
{}


NBTrafficLightLogicVector *
NBTrafficLightDefinition::compute(const NBEdgeCont &ec, OptionsCont &oc)
{
    // it is not really a traffic light if no incoming edge exists
    if (_incoming.size()==0) {
        WRITE_WARNING("The traffic light '" + getID() + "' has no incoming edges; it will not be build.");
        return 0;
    }
    // compute the time needed to brake
    size_t breakingTime = computeBrakingTime(oc.getFloat("min-decel"));
    // perform the computation depending on whether the traffic light
    //  definition was loaded or shall be computed new completely
    if (OptionsSubSys::getOptions().isSet("traffic-light-yellow")) {
        breakingTime = OptionsSubSys::getOptions().getInt("traffic-light-yellow");
    }
    return myCompute(ec, breakingTime, myType, oc.getBool("all-logics"));
}


size_t
NBTrafficLightDefinition::computeBrakingTime(SUMOReal minDecel) const
{
    SUMOReal vmax = NBContHelper::maxSpeed(_incoming);
    return (size_t)(vmax / minDecel);
}


void
NBTrafficLightDefinition::setParticipantsInformation()
{
    // collect the infomration about participating edges and links
    collectEdges();
    collectLinks();
}


void
NBTrafficLightDefinition::collectEdges()
{
    EdgeVector myOutgoing;
    // collect the edges from the participating nodes
    for (NodeCont::iterator i=_nodes.begin(); i!=_nodes.end(); i++) {
        const EdgeVector &incoming = (*i)->getIncomingEdges();
        copy(incoming.begin(), incoming.end(), back_inserter(_incoming));
        const EdgeVector &outgoing = (*i)->getOutgoingEdges();
        copy(outgoing.begin(), outgoing.end(), back_inserter(myOutgoing));
    }
    // check which of the edges are completely within the junction
    //  remove these edges from the list of incoming edges
    //  add them to the list of edges lying within the node
    size_t pos = 0;
    while (pos<_incoming.size()) {
        NBEdge *edge = *(_incoming.begin() + pos);
        // an edge lies within the logic if it outgoing as well as incoming
        EdgeVector::iterator j = find(myOutgoing.begin(), myOutgoing.end(), edge);
        if (j!=myOutgoing.end()) {
            _within.push_back(edge);
            _incoming.erase(_incoming.begin() + pos);
        } else {
            pos++;
        }
    }
}


void
NBTrafficLightDefinition::collectLinks()
{
    // build the list of links which are controled by the traffic light
    for (EdgeVector::iterator i=_incoming.begin(); i!=_incoming.end(); i++) {
        NBEdge *incoming = *i;
        size_t noLanes = incoming->getNoLanes();
        for (size_t j=0; j<noLanes; j++) {
            const EdgeLaneVector &connected = incoming->getEdgeLanesFromLane(j);
            for (EdgeLaneVector::const_iterator k=connected.begin(); k!=connected.end(); k++) {
                const EdgeLane &el = *k;
                if (el.edge!=0) {
                    _links.push_back(
                        NBConnection(incoming, j, el.edge, el.lane));
                }
            }
        }
    }
    // set the information about the link's positions within the tl into the
    //  edges the links are starting at, respectively
    size_t pos = 0;
    for (NBConnectionVector::iterator j=_links.begin(); j!=_links.end(); j++) {
        const NBConnection &conn = *j;
        NBEdge *edge = conn.getFrom();
        edge->setControllingTLInformation(
            conn.getFromLane(), conn.getTo(), conn.getToLane(),
            getID(), pos++);
    }
}


pair<size_t, size_t>
NBTrafficLightDefinition::getSizes() const
{
    size_t noLanes = 0;
    size_t noLinks = 0;
    for (EdgeVector::const_iterator i=_incoming.begin(); i!=_incoming.end(); i++) {
        size_t noLanesEdge = (*i)->getNoLanes();
        for (size_t j=0; j<noLanesEdge; j++) {
            assert((*i)->getEdgeLanesFromLane(j).size()!=0);
            noLinks += (*i)->getEdgeLanesFromLane(j).size();
        }
        noLanes += noLanesEdge;
    }
    return pair<size_t, size_t>(noLanes, noLinks);
}


bool
NBTrafficLightDefinition::isLeftMover(NBEdge *from, NBEdge *to) const
{
    // the destination edge may be unused
    if (to==0) {
        return false;
    }
    // get the node which is holding this connection
    NodeCont::const_iterator i =
        find_if(_nodes.begin(), _nodes.end(),
                NBContHelper::node_with_incoming_finder(from));
    assert(i!=_nodes.end());
    NBNode *node = *i;
    return node->isLeftMover(from, to);
}


bool
NBTrafficLightDefinition::mustBrake(NBEdge *from, NBEdge *to) const
{
    NodeCont::const_iterator i =
        find_if(_nodes.begin(), _nodes.end(),
                NBContHelper::node_with_incoming_finder(from));
    assert(i!=_nodes.end());
    NBNode *node = *i;
    if (!node->hasOutgoing(to)) {
        return true; // !!!
    }
    return node->mustBrake(from, to, -1);
}


bool
NBTrafficLightDefinition::mustBrake(NBEdge *possProhibitedFrom,
                                    NBEdge *possProhibitedTo,
                                    NBEdge *possProhibitorFrom,
                                    NBEdge *possProhibitorTo,
                                    bool regardNonSignalisedLowerPriority) const
{
    return forbids(possProhibitorFrom, possProhibitorTo,
                   possProhibitedFrom, possProhibitedTo,
                   regardNonSignalisedLowerPriority);
}


bool
NBTrafficLightDefinition::mustBrake(const NBConnection &possProhibited,
                                    const NBConnection &possProhibitor,
                                    bool regardNonSignalisedLowerPriority) const
{
    return forbids(possProhibitor.getFrom(), possProhibitor.getTo(),
                   possProhibited.getFrom(), possProhibited.getTo(),
                   regardNonSignalisedLowerPriority);
}


bool
NBTrafficLightDefinition::forbids(NBEdge *possProhibitorFrom,
                                  NBEdge *possProhibitorTo,
                                  NBEdge *possProhibitedFrom,
                                  NBEdge *possProhibitedTo,
                                  bool regardNonSignalisedLowerPriority) const
{
    if (possProhibitorFrom==0||possProhibitorTo==0||possProhibitedFrom==0||possProhibitedTo==0) {
        return false;
    }
    /*
    if(getID()=="test1") {
        cout << possProhibitorFrom->getID() << "->" << possProhibitorTo->getID()
            << " vs "
            << possProhibitedFrom->getID() << "->" << possProhibitedTo->getID();
        int bla;
    }
    if(possProhibitorFrom->getID()=="-53488232" && possProhibitorTo->getID()=="-53488245") {
        if(possProhibitedFrom->getID()=="53488385" && possProhibitedTo->getID()=="53488244") {
            int bla = 0;
        }
    }
    */

    // retrieve both nodes
    NodeCont::const_iterator incoming =
        find_if(_nodes.begin(), _nodes.end(), NBContHelper::node_with_incoming_finder(possProhibitorFrom));
    NodeCont::const_iterator outgoing =
        find_if(_nodes.begin(), _nodes.end(), NBContHelper::node_with_outgoing_finder(possProhibitedTo));
    assert(incoming!=_nodes.end());
    NBNode *incnode = *incoming;
    NBNode *outnode = *outgoing;
    EdgeVector::const_iterator i;
    if (incnode!=outnode) {
        // the links are located at different nodes
        const EdgeVector &ev1 = possProhibitedTo->getConnected();
        // go through the following edge,
        //  check whether one of these connections is prohibited
        for (i=ev1.begin(); i!=ev1.end(); ++i) {
            NodeCont::const_iterator outgoing2 =
                find_if(_nodes.begin(), _nodes.end(), NBContHelper::node_with_outgoing_finder(*i));
            if (outgoing2==_nodes.end()) {
                continue;
            }
            NBNode *outnode2 = *outgoing2;
            if (incnode!=outnode2) {
                continue;
            }
            bool ret1 = incnode->foes(possProhibitorTo, *i,
                                      possProhibitedFrom, possProhibitedTo);
            bool ret2 = incnode->forbids(possProhibitorFrom, possProhibitorTo,
                                         possProhibitedTo, *i,
                                         regardNonSignalisedLowerPriority);
            bool ret = ret1||ret2;
            if (ret) {
                return true;
            }
        }

        const EdgeVector &ev2 = possProhibitorTo->getConnected();
        // go through the following edge,
        //  check whether one of these connections is prohibited
        for (i=ev2.begin(); i!=ev2.end(); ++i) {
            NodeCont::const_iterator incoming2 =
                find_if(_nodes.begin(), _nodes.end(), NBContHelper::node_with_incoming_finder(possProhibitorTo));
            if (incoming2==_nodes.end()) {
                continue;
            }
            NBNode *incnode2 = *incoming2;
            if (incnode2!=outnode) {
                continue;
            }
            /*
            if(possProhibitorTo->getID()=="-53488245" && (*i)->getID()=="-53488290") {
                if(possProhibitedFrom->getID()=="53488385" && possProhibitedTo->getID()=="53488244") {
                    int bla = 0;
                }
            }
            */
            bool ret1 = incnode2->foes(possProhibitorTo, *i,
                                       possProhibitedFrom, possProhibitedTo);
            bool ret2 = incnode2->forbids(possProhibitorTo, *i,
                                          possProhibitedFrom, possProhibitedTo,
                                          regardNonSignalisedLowerPriority);
            bool ret = ret1||ret2;
            if (ret) {
                return true;
            }
        }
        return false;
    }
    // both links are located at the same node
    //  check using this node's information
    return incnode->forbids(possProhibitorFrom, possProhibitorTo,
                            possProhibitedFrom, possProhibitedTo,
                            regardNonSignalisedLowerPriority);
    /*
    if(!ret) {
        cout << ": no2" << endl;
    } else {
        cout << ": yes" << endl;
    }
    return ret;
    */
}


bool
NBTrafficLightDefinition::foes(NBEdge *from1, NBEdge *to1,
                               NBEdge *from2, NBEdge *to2) const
{
    // retrieve both nodes (it is possible that a connection
    NodeCont::const_iterator incoming =
        find_if(_nodes.begin(), _nodes.end(),
                NBContHelper::node_with_incoming_finder(from1));
    NodeCont::const_iterator outgoing =
        find_if(_nodes.begin(), _nodes.end(),
                NBContHelper::node_with_outgoing_finder(to1));
    assert(incoming!=_nodes.end());
    NBNode *incnode = *incoming;
    NBNode *outnode = *outgoing;
    if (incnode!=outnode) {
        return false;
    }
    return incnode->foes(from1, to1, from2, to2);
}


void
NBTrafficLightDefinition::addNode(NBNode *node)
{
    _nodes.insert(node);
    node->addTrafficLight(this);
}



/****************************************************************************/

