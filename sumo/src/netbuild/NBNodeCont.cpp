/****************************************************************************/
/// @file    NBNodeCont.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Container for nodes during the netbuilding process
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
#include <algorithm>
#include <utils/options/OptionsCont.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ToString.h>
#include "NBDistrict.h"
#include "NBEdgeCont.h"
#include "NBJunctionLogicCont.h"
#include "NBTrafficLightLogicCont.h"
#include "NBJoinedEdgesMap.h"
#include "NBOwnTLDef.h"
#include <cmath>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>

#include "NBNodeCont.h"

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
NBNodeCont::NBNodeCont()
        : myInternalID(1)
{}


NBNodeCont::~NBNodeCont()
{
    clear();
}


bool
NBNodeCont::insert(const std::string &id, const Position2D &position,
                   NBDistrict *district)
{
    NodeCont::iterator i = myNodes.find(id);
    if (i!=myNodes.end()) {
        if (fabs((*i).second->getPosition().x()-position.x())<POSITION_EPS &&
                fabs((*i).second->getPosition().y()-position.y())<POSITION_EPS) {
            return true;
        }
        return false;
    }
    NBNode *node = new NBNode(id, position, district);
    myNodes[id] = node;
    return true;
}


bool
NBNodeCont::insert(const string &id, const Position2D &position)
{
    NodeCont::iterator i = myNodes.find(id);
    if (i!=myNodes.end()) {
        if (fabs((*i).second->getPosition().x()-position.x())<POSITION_EPS &&
                fabs((*i).second->getPosition().y()-position.y())<POSITION_EPS) {
            return true;
        }
        return false;
    }
    NBNode *node = new NBNode(id, position);
    myNodes[id] = node;
    return true;
}


Position2D
NBNodeCont::insert(const string &id) // !!! really needed
{
    pair<SUMOReal, SUMOReal> ret(-1.0, -1.0);
    NodeCont::iterator i = myNodes.find(id);
    if (i!=myNodes.end()) {
        return (*i).second->getPosition();
    } else {
        NBNode *node = new NBNode(id, Position2D(-1.0, -1.0));
        myNodes[id] = node;
    }
    return Position2D(-1, -1);
}


bool
NBNodeCont::insert(NBNode *node)
{
    string id = node->getID();
    NodeCont::iterator i = myNodes.find(id);
    if (i!=myNodes.end()) {
        if (fabs((*i).second->getPosition().x()-node->getPosition().x())<POSITION_EPS &&
                fabs((*i).second->getPosition().y()-node->getPosition().y())<POSITION_EPS) {
            return true;
        }
        return false;
    }
    myNodes[id] = node;
    return true;
}


NBNode *
NBNodeCont::retrieve(const string &id)
{
    NodeCont::iterator i = myNodes.find(id);
    if (i==myNodes.end()) {
        return 0;
    }
    return (*i).second;
}


NBNode *
NBNodeCont::retrieve(const Position2D &position)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        NBNode *node = (*i).second;
        if (fabs(node->getPosition().x()-position.x())<POSITION_EPS
                &&
                fabs(node->getPosition().y()-position.y())<POSITION_EPS) {

            return node;
        }
    }
    return 0;
}


bool
NBNodeCont::erase(NBNode *node)
{
    NodeCont::iterator i = myNodes.find(node->getID());
    if (i==myNodes.end()) {
        return false;
    }
    myNodes.erase(i);
    delete node;
    return true;
}


void
NBNodeCont::normaliseNodePositions()
{
    NodeCont::iterator i;
    // reformat
    const Boundary &b = GeoConvHelper::getConvBoundary();
    const SUMOReal xmin = -b.xmin();
    const SUMOReal ymin = -b.ymin();
    if (fabs(xmin) > POSITION_EPS || fabs(ymin) > POSITION_EPS) {
        for (i=myNodes.begin(); i!=myNodes.end(); i++) {
            (*i).second->resetby(xmin, ymin);
        }
        GeoConvHelper::moveConvertedBy(xmin, ymin);
    }
}


void
NBNodeCont::reshiftNodePositions(SUMOReal xoff, SUMOReal yoff, SUMOReal rot)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->reshiftPosition(xoff, yoff, rot);
    }
    GeoConvHelper::moveConvertedBy(xoff, yoff); // !!! rotation
}


void
NBNodeCont::computeLanes2Lanes()
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->computeLanes2Lanes();
    }
}


// computes the "wheel" of incoming and outgoing edges for every node
void
NBNodeCont::computeLogics(const NBEdgeCont &ec, NBJunctionLogicCont &jc,
                          OptionsCont &oc)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->computeLogic(ec, jc, oc);
    }
}


void
NBNodeCont::sortNodesEdges(const NBTypeCont &tc)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->sortNodesEdges(tc);
    }
}


void
NBNodeCont::writeXMLNumber(OutputDevice &into)
{
    into << "   <node_count>" << myNodes.size() << "</node_count>\n";
}


std::vector<std::string>
NBNodeCont::getInternalNamesList()
{
    std::vector<std::string> ret;
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        std::vector<std::string> nodes = (*i).second->getInternalNamesList();
        copy(nodes.begin(), nodes.end(), back_inserter(ret));
    }
    return ret;
}



void
NBNodeCont::writeXMLInternalLinks(OutputDevice &into)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->writeXMLInternalLinks(into);
    }
    into << "\n";
}


void
NBNodeCont::writeXMLInternalSuccInfos(OutputDevice &into)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->writeXMLInternalSuccInfos(into);
    }
    into << "\n";
}


void
NBNodeCont::writeXMLInternalNodes(OutputDevice &into)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->writeXMLInternalNodes(into);
    }
    into << "\n";
}


void
NBNodeCont::writeXML(OutputDevice &into)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->writeXML(into);
    }
    into << "\n";
}


int
NBNodeCont::size()
{
    return myNodes.size();
}


void
NBNodeCont::clear()
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        delete((*i).second);
    }
    myNodes.clear();
}


void
NBNodeCont::recheckEdges(NBDistrictCont &dc, NBTrafficLightLogicCont &tlc,
                         NBEdgeCont &ec)
{
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        // count the edges to other nodes outgoing from the current
        //  node
        std::map<NBNode*, EdgeVector> connectionCount;
        const EdgeVector &outgoing = (*i).second->getOutgoingEdges();
        for (EdgeVector::const_iterator j=outgoing.begin(); j!=outgoing.end(); j++) {
            NBEdge *e = (*j);
            NBNode *connected = e->getToNode();
            if (connectionCount.find(connected)==connectionCount.end()) {
                connectionCount[connected] = EdgeVector();
                connectionCount[connected].push_back(e);
            } else {
                connectionCount[connected].push_back(e);
            }
        }
        // check whether more than a single edge connect another node
        //  and join them
        std::map<NBNode*, EdgeVector>::iterator k;
        for (k=connectionCount.begin(); k!=connectionCount.end(); k++) {
            // possibly we do not have anything to join...
            if ((*k).second.size()<2) {
                continue;
            }
            // for the edges that seem to be a single street,
            //  check whether the geometry is similar
            const EdgeVector &ev = (*k).second;
            typedef std::vector<EdgeVector> EdgeVV;
            EdgeVV geometryCombinations;
            for (EdgeVector::const_iterator l=ev.begin(); l!=ev.end(); ++l) {
                // append the first one simply to the list of really joinable edges
                if (geometryCombinations.size()==0) {
                    EdgeVector tmp;
                    tmp.push_back(*l);
                    geometryCombinations.push_back(tmp);
                    continue;
                }
                // check the lists of really joinable edges
                bool wasPushed = false;
                for (EdgeVV::iterator m=geometryCombinations.begin(); !wasPushed&&m!=geometryCombinations.end();) {
                    for (EdgeVector::iterator n=(*m).begin(); !wasPushed&&n!=(*m).end();) {
                        if ((*n)->isNearEnough2BeJoined2(*l)) {
                            (*m).push_back(*l);
                            wasPushed = true;
                        }
                        if (!wasPushed) {
                            ++n;
                        }
                    }
                    if (!wasPushed) {
                        ++m;
                    }
                }
                if (!wasPushed) {
                    EdgeVector tmp;
                    tmp.push_back(*l);
                    geometryCombinations.push_back(tmp);
                }
            }
        }
        for (k=connectionCount.begin(); k!=connectionCount.end(); k++) {
            // join edges
            if ((*k).second.size()>1) {
                ec.joinSameNodeConnectingEdges(dc, tlc, (*k).second);
            }
        }
    }
}



void
NBNodeCont::removeDummyEdges(NBDistrictCont &dc, NBEdgeCont &ec,
                             NBTrafficLightLogicCont &tc)
{
    size_t no = 0;
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        no += (*i).second->eraseDummies(dc, ec, tc);
    }
    if (no!=0) {
        WRITE_WARNING(toString<int>(no) + " dummy edge(s) removed.");
    }
}


std::string
NBNodeCont::getFreeID()
{
    return "SUMOGenerated" + toString<int>(size());
}


void
NBNodeCont::computeNodeShapes()
{
    OutputDevice::createDeviceByOption("node-geometry-dump", "pois");
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        (*i).second->computeNodeShape();
    }
}


void
NBNodeCont::removeUnwishedNodes(NBDistrictCont &dc, NBEdgeCont &ec,
                                NBTrafficLightLogicCont &tlc)
{
    size_t no = 0;
    std::vector<NBNode*> toRemove;
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        NBNode *current = (*i).second;
        bool remove = false;
        std::vector<std::pair<NBEdge*, NBEdge*> > toJoin;
        // check for completely empty nodes
        if (current->getOutgoingEdges().size()==0
                &&
                current->getIncomingEdges().size()==0) {

            // remove if empty
            remove = true;
        }
        // check for nodes which are only geometry nodes
        if ((current->getOutgoingEdges().size()==1
                &&
                current->getIncomingEdges().size()==1)
                ||
                (current->getOutgoingEdges().size()==2
                 &&
                 current->getIncomingEdges().size()==2)) {

            // ok, one in, one out or two in, two out
            //  -> ask the node whether to join
            remove = current->checkIsRemovable();
            if (remove) {
                toJoin = current->getEdgesToJoin();
            }
        }
        // remove the node and join the geometries when wished
        if (!remove) {
            continue;
        }
        for (std::vector<std::pair<NBEdge*, NBEdge*> >::iterator j=toJoin.begin(); j!=toJoin.end(); j++) {
            NBEdge *begin = (*j).first;
            NBEdge *continuation = (*j).second;
            begin->append(continuation);
            continuation->getToNode()->replaceIncoming(continuation, begin, 0);
            tlc.replaceRemoved(continuation, -1, begin, -1);
            gJoinedEdges.appended(begin->getID(), continuation->getID());
            ec.erase(dc, continuation);
        }
        toRemove.push_back(current);
        no++;
    }
    // erase
    for (std::vector<NBNode*>::iterator j=toRemove.begin(); j!=toRemove.end(); j++) {
        const set<NBTrafficLightDefinition*> &tls = (*j)->getControllingTLS();
        for(set<NBTrafficLightDefinition*>::const_iterator i=tls.begin(); i!=tls.end(); ++i) {
            (*i)->removeNode(*j);
        }
        erase(*j);
    }
    WRITE_MESSAGE("   " + toString<int>(no) + " nodes removed.");
}



bool
NBNodeCont::mayNeedOnRamp(OptionsCont &oc, NBNode *cur) const
{
    if (cur->getIncomingEdges().size()==2&&cur->getOutgoingEdges().size()==1) {
        // may be an on-ramp
        NBEdge *pot_highway = cur->getIncomingEdges()[0];
        NBEdge *pot_ramp = cur->getIncomingEdges()[1];
        NBEdge *cont = cur->getOutgoingEdges()[0];

        // check whether a lane is missing
        if (pot_highway->getNoLanes()+pot_ramp->getNoLanes()<=cont->getNoLanes()) {
            return false;
        }

        // assign highway/ramp properly
        if (pot_highway->getSpeed()<pot_ramp->getSpeed()) {
            swap(pot_highway, pot_ramp);
        } else if (pot_highway->getSpeed()==pot_ramp->getSpeed()
                   &&
                   pot_highway->getNoLanes()<pot_ramp->getNoLanes()) {

            swap(pot_highway, pot_ramp);
        }

        // check conditions
        // is it really a highway?
        if (pot_highway->getSpeed()<oc.getFloat("ramp-guess.min-highway-speed")
                ||
                cont->getSpeed()<oc.getFloat("ramp-guess.min-highway-speed")) {
            return false;
        }
        // is it really a ramp?
        if (oc.getFloat("ramp-guess.max-ramp-speed")>0
                &&
                oc.getFloat("ramp-guess.max-ramp-speed")<pot_ramp->getSpeed()) {
            return false;
        }

        // ok, may be
        return true;
    }
    return false;
}


void
NBNodeCont::buildOnRamp(OptionsCont &oc, NBNode *cur,
                        NBEdgeCont &ec, NBDistrictCont &dc,
                        std::vector<NBEdge*> &incremented)
{
    NBEdge *pot_highway = cur->getIncomingEdges()[0];
    NBEdge *pot_ramp = cur->getIncomingEdges()[1];
    NBEdge *cont = cur->getOutgoingEdges()[0];
    // assign highway/ramp properly
    if (pot_highway->getSpeed()<pot_ramp->getSpeed()) {
        swap(pot_highway, pot_ramp);
    } else if (pot_highway->getSpeed()==pot_ramp->getSpeed()
               &&
               pot_highway->getNoLanes()<pot_ramp->getNoLanes()) {

        swap(pot_highway, pot_ramp);
    }

    // compute the number of lanes to append
    int toAdd = (pot_ramp->getNoLanes() + pot_highway->getNoLanes()) - cont->getNoLanes();
    if (toAdd<=0) {
        return;
    }

    //
    if (cont->getGeometry().length()-POSITION_EPS<=oc.getFloat("ramp-guess.ramp-length")) {
        // the edge is shorter than the wished ramp
        //  append a lane only
        if (find(incremented.begin(), incremented.end(), cont)==incremented.end()) {
            cont->incLaneNo(toAdd);
            incremented.push_back(cont);
            if (!pot_highway->addLane2LaneConnections(0, cont, pot_ramp->getNoLanes(),
                    MIN2(cont->getNoLanes()-pot_ramp->getNoLanes(), pot_highway->getNoLanes()), false, true)) {

                throw ProcessError("Could not set connection!");

            }
            if (!pot_ramp->addLane2LaneConnections(0, cont, 0, pot_ramp->getNoLanes(), false, true)) {
                throw ProcessError("Could not set connection!");

            }
            //
            cont->invalidateConnections(true);
            const EdgeVector &o1 = cont->getToNode()->getOutgoingEdges();
            if (o1.size()==1&&o1[0]->getNoLanes()<cont->getNoLanes()) {
                cont->addLane2LaneConnections(cont->getNoLanes()-o1[0]->getNoLanes(),
                                              o1[0], 0, o1[0]->getNoLanes(), false);
            }
            //
            if (cont->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER) {
                Position2DVector g = cont->getGeometry();
                g.move2side(SUMO_const_laneWidthAndOffset);
                cont->setGeometry(g);
            }
        }
        Position2DVector p = pot_ramp->getGeometry();
        p.pop_back();
        p.push_back(cont->getFromNode()->getPosition());//added_ramp->getLaneShape(0).at(0));
        pot_ramp->setGeometry(p);
    } else {
        // there is enough place to build a ramp; do it
        NBNode *rn =
            new NBNode(cont->getID() + "-AddedOnRampNode",
                       cont->getGeometry().positionAtLengthPosition(
                           oc.getFloat("ramp-guess.ramp-length")));
        if (!insert(rn)) {
            throw ProcessError("Ups - could not build on-ramp for edge '" + pot_highway->getID() + "' (node could not be build)!");

        }
        string name = cont->getID();
        bool ok = ec.splitAt(dc, cont, rn,
                             cont->getID()+"-AddedOnRampEdge", cont->getID(),
                             cont->getNoLanes()+toAdd, cont->getNoLanes());
        if (!ok) {
            MsgHandler::getErrorInstance()->inform("Ups - could not build on-ramp for edge '" + pot_highway->getID() + "'!");
            return;
        } else {
            NBEdge *added_ramp = ec.retrieve(name+"-AddedOnRampEdge");
            NBEdge *added = ec.retrieve(name);
            incremented.push_back(added_ramp);
            if (added_ramp->getNoLanes()!=added->getNoLanes()) {
                int off = added_ramp->getNoLanes()-added->getNoLanes();
                if (!added_ramp->addLane2LaneConnections(off, added, 0, added->getNoLanes(), false, true)) {
                    throw ProcessError("Could not set connection!");

                }
                if (added_ramp->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER) {
                    Position2DVector g = added_ramp->getGeometry();
                    SUMOReal factor =
                        SUMO_const_laneWidthAndOffset * (SUMOReal)(toAdd-1)
                        + SUMO_const_halfLaneAndOffset * (SUMOReal)(toAdd%2);
                    g.move2side(factor);
                    added_ramp->setGeometry(g);
                }
            } else {
                if (!added_ramp->addLane2LaneConnections(0, added, 0, added_ramp->getNoLanes(), false, true)) {
                    throw ProcessError("Could not set connection!");

                }
            }
            if (!pot_highway->addLane2LaneConnections(0, added_ramp, pot_ramp->getNoLanes(),
                    MIN2(added_ramp->getNoLanes()-pot_ramp->getNoLanes(), pot_highway->getNoLanes()), false, false)) {

                throw ProcessError("Could not set connection!");

            }
            if (!pot_ramp->addLane2LaneConnections(0, added_ramp, 0, pot_ramp->getNoLanes(), false, true)) {
                throw ProcessError("Could not set connection!");

            }
            Position2DVector p = pot_ramp->getGeometry();
            p.pop_back();
            p.push_back(added_ramp->getFromNode()->getPosition());//added_ramp->getLaneShape(0).at(0));
            pot_ramp->setGeometry(p);
        }
    }
}


void
NBNodeCont::buildOffRamp(OptionsCont &oc, NBNode *cur,
                         NBEdgeCont &ec, NBDistrictCont &dc,
                         std::vector<NBEdge*> &incremented)
{
    NBEdge *pot_highway = cur->getOutgoingEdges()[0];
    NBEdge *pot_ramp = cur->getOutgoingEdges()[1];
    NBEdge *prev = cur->getIncomingEdges()[0];
    // assign highway/ramp properly
    if (pot_highway->getSpeed()<pot_ramp->getSpeed()) {
        swap(pot_highway, pot_ramp);
    } else if (pot_highway->getSpeed()==pot_ramp->getSpeed()
               &&
               pot_highway->getNoLanes()<pot_ramp->getNoLanes()) {

        swap(pot_highway, pot_ramp);
    }
    // compute the number of lanes to append
    int toAdd = (pot_ramp->getNoLanes() + pot_highway->getNoLanes()) - prev->getNoLanes();
    if (toAdd<=0) {
        return;
    }
    // append on-ramp
    if (prev->getGeometry().length()-POSITION_EPS<=oc.getFloat("ramp-guess.ramp-length")) {
        // the edge is shorter than the wished ramp
        //  append a lane only
        if (find(incremented.begin(), incremented.end(), prev)==incremented.end()) {
            incremented.push_back(prev);
            prev->incLaneNo(toAdd);
            prev->invalidateConnections(true);
            if (!prev->addLane2LaneConnections(pot_ramp->getNoLanes(), pot_highway, 0,
                                               MIN2(prev->getNoLanes()-1, pot_highway->getNoLanes()), false, true)) {

                throw ProcessError("Could not set connection!");

            }
            if (!prev->addLane2LaneConnections(0, pot_ramp, 0, pot_ramp->getNoLanes(), false, false)) {
                throw ProcessError("Could not set connection!");

            }
            if (prev->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER) {
                Position2DVector g = prev->getGeometry();
                g.move2side(SUMO_const_laneWidthAndOffset);//SUMO_const_laneWidthAndOffset*(SUMOReal)(toAdd-1)+SUMO_const_halfLaneAndOffset);
                prev->setGeometry(g);
            }
        }
        Position2DVector p = pot_ramp->getGeometry();
        p.pop_front();
        p.push_front(prev->getToNode()->getPosition());//added_ramp->getLaneShape(0).at(-1));
        pot_ramp->setGeometry(p);
    } else {
        Position2D pos =
            prev->getGeometry().positionAtLengthPosition(
                prev->getGeometry().length()-oc.getFloat("ramp-guess.ramp-length"));
        NBNode *rn = new NBNode(prev->getID() + "-AddedOffRampNode", pos);
        if (!insert(rn)) {
            throw ProcessError("Ups - could not build off-ramp for edge '" + pot_highway->getID() + "' (node could not be build)!");

        }
        string name = prev->getID();
        bool ok = ec.splitAt(dc, prev, rn,
                             prev->getID(), prev->getID()+"-AddedOffRampEdge",
                             prev->getNoLanes(), prev->getNoLanes()+toAdd);
        if (!ok) {
            MsgHandler::getErrorInstance()->inform("Ups - could not build on-ramp for edge '" + pot_highway->getID() + "'!");
            return;
        } else {
            NBEdge *added_ramp = ec.retrieve(name+"-AddedOffRampEdge");
            NBEdge *added = ec.retrieve(name);
            if (added_ramp->getNoLanes()!=added->getNoLanes()) {
                incremented.push_back(added_ramp);
                int off = added_ramp->getNoLanes()-added->getNoLanes();
                if (!added->addLane2LaneConnections(0, added_ramp, off, added->getNoLanes(), false, true)) {
                    throw ProcessError("Could not set connection!");

                }
                if (added_ramp->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER) {
                    Position2DVector g = added_ramp->getGeometry();
                    SUMOReal factor =
                        SUMO_const_laneWidthAndOffset * (SUMOReal)(toAdd-1)
                        + SUMO_const_halfLaneAndOffset * (SUMOReal)(toAdd%2);
                    g.move2side(factor);
                    added_ramp->setGeometry(g);
                }
            } else {
                if (!added->addLane2LaneConnections(0, added_ramp, 0, added_ramp->getNoLanes(), false, true)) {
                    throw ProcessError("Could not set connection!");

                }
            }
            if (!added_ramp->addLane2LaneConnections(pot_ramp->getNoLanes(), pot_highway, 0,
                    MIN2(added_ramp->getNoLanes()-pot_ramp->getNoLanes(), pot_highway->getNoLanes()), false, true)) {

                throw ProcessError("Could not set connection!");

            }
            if (!added_ramp->addLane2LaneConnections(0, pot_ramp, 0, pot_ramp->getNoLanes(), false, false)) {
                throw ProcessError("Could not set connection!");

            }
            Position2DVector p = pot_ramp->getGeometry();
            p.pop_front();
            p.push_front(added_ramp->getToNode()->getPosition());//added_ramp->getLaneShape(0).at(-1));
            pot_ramp->setGeometry(p);
        }
    }
}


bool
NBNodeCont::mayNeedOffRamp(OptionsCont &oc, NBNode *cur) const
{
    if (cur->getIncomingEdges().size()==1&&cur->getOutgoingEdges().size()==2) {
        // may be an off-ramp
        NBEdge *pot_highway = cur->getOutgoingEdges()[0];
        NBEdge *pot_ramp = cur->getOutgoingEdges()[1];
        NBEdge *prev = cur->getIncomingEdges()[0];

        // check whether a lane is missing
        if (pot_highway->getNoLanes()+pot_ramp->getNoLanes()<=prev->getNoLanes()) {
            return false;
        }

        // assign highway/ramp properly
        if (pot_highway->getSpeed()<pot_ramp->getSpeed()) {
            swap(pot_highway, pot_ramp);
        } else if (pot_highway->getSpeed()==pot_ramp->getSpeed()
                   &&
                   pot_highway->getNoLanes()<pot_ramp->getNoLanes()) {

            swap(pot_highway, pot_ramp);
        }

        // check conditions
        // is it really a highway?
        if (pot_highway->getSpeed()<oc.getFloat("ramp-guess.min-highway-speed")
                ||
                prev->getSpeed()<oc.getFloat("ramp-guess.min-highway-speed")) {
            return false;
        }
        // is it really a ramp?
        if (oc.getFloat("ramp-guess.max-ramp-speed")>0
                &&
                oc.getFloat("ramp-guess.max-ramp-speed")<pot_ramp->getSpeed()) {
            return false;
        }

        return true;
    }
    return false;
}


void
NBNodeCont::checkHighwayRampOrder(NBEdge *&pot_highway, NBEdge *&pot_ramp)
{
    if (pot_highway->getSpeed()<pot_ramp->getSpeed()) {
        swap(pot_highway, pot_ramp);
    } else if (pot_highway->getSpeed()==pot_ramp->getSpeed()
               &&
               pot_highway->getNoLanes()<pot_ramp->getNoLanes()) {

        swap(pot_highway, pot_ramp);
    }
}


void
NBNodeCont::guessRamps(OptionsCont &oc, NBEdgeCont &ec,
                       NBDistrictCont &dc)
{
    std::vector<NBEdge*> incremented;
    // check whether obsure highway connections shall be checked
    if (oc.getBool("guess-obscure-ramps")) {
        for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
            NBNode *cur = (*i).second;
            const EdgeVector &inc = cur->getIncomingEdges();
            const EdgeVector &out = cur->getOutgoingEdges();
            if (inc.size()!=2||out.size()!=2) {
                continue;
            }
            {
                bool hadInHighway = false;
                for (EdgeVector::const_iterator j=inc.begin(); j!=inc.end(); ++j) {
                    if ((*j)->getSpeed()>oc.getFloat("obscure-ramps.min-highway-speed")) {

                        hadInHighway = true;
                    }
                }
                if (!hadInHighway) {
                    continue;
                }
            }
            {
                bool hadOutHighway = false;
                for (EdgeVector::const_iterator j=out.begin(); j!=out.end(); ++j) {
                    if ((*j)->getSpeed()>oc.getFloat("obscure-ramps.min-highway-speed")) {

                        hadOutHighway = true;
                    }
                }
                if (!hadOutHighway) {
                    continue;
                }
            }
            // ok, something is strange:
            //  we do have a highway, here with an off- and an on-ramp on the same node!?
            // try to place the incoming before...
            //  ... determine a possible position, first
            NBEdge *inc_highway = inc[0];
            NBEdge *inc_ramp = inc[1];
            NBEdge *out_highway = out[0];
            NBEdge *out_ramp = out[1];
            checkHighwayRampOrder(inc_highway, inc_ramp);
            checkHighwayRampOrder(out_highway, out_ramp);

            if (100>GeomHelper::distance(inc_highway->getToNode()->getPosition(), inc_ramp->getGeometry()[-1])) {
                Position2DVector tmp = inc_ramp->getGeometry();
                tmp.eraseAt(-1);
                inc_ramp->setGeometry(tmp);
            }
            SUMOReal pos =
                inc_highway->getGeometry().nearest_position_on_line_to_point(
                    inc_ramp->getGeometry()[-1]);
            if (pos<0) {
                continue;
            }
            Position2D p = inc_highway->getGeometry().positionAtLengthPosition(pos);
            NBNode *rn =
                new NBNode(inc_highway->getID() + "-AddedAntiObscureNode", p);
            if (!insert(rn)) {
                throw ProcessError("Ups - could not build anti-obscure node '" + inc_highway->getID() + "'!");

            }
            string name = inc_highway->getID();
            bool ok = ec.splitAt(dc, inc_highway, rn,
                                 inc_highway->getID(), inc_highway->getID()+"-AddedInBetweenEdge",
                                 inc_highway->getNoLanes(), inc_highway->getNoLanes());
            if (!ok) {
                throw ProcessError("Ups - could not build anti-obscure edge '" + inc_highway->getID() + "'!");

            } else {
                NBEdge *added_cont = ec.retrieve(name+"-AddedInBetweenEdge");
                NBEdge *added = ec.retrieve(name);
                added_cont->getToNode()->removeIncoming(out_ramp);
                added->getToNode()->addIncomingEdge(out_ramp);
            }
        }
    }
    // check whether on-off ramps shall be guessed
    if (oc.getBool("guess-ramps")) {
        for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
            NBNode *cur = (*i).second;
            if (mayNeedOnRamp(oc, cur)) {
                buildOnRamp(oc, cur, ec, dc, incremented);
            }
            if (mayNeedOffRamp(oc, cur)) {
                buildOffRamp(oc, cur, ec, dc, incremented);
            }
        }
    }
}


void
NBNodeCont::guessTLs(OptionsCont &oc, NBTrafficLightLogicCont &tlc)
{
    // loop#1 checking whether the node shall tls controlled,
    //  because it is assigned to a district
    if (oc.getBool("tls-guess.district-nodes")) {
        for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
            NBNode *cur = (*i).second;
            if (cur->isNearDistrict()) {
                setAsTLControlled((*i).first, tlc);
            }
        }
    }
    // maybe no tls shall be guessed
    if (!oc.getBool("guess-tls")) {
        return;
    }
    // build list of definitely not tls-controlled junctions
    std::vector<NBNode*> ncontrolled;
    if (oc.isSet("explicite-no-tls")) {
        StringTokenizer st(oc.getString("explicite-no-tls"), ";");
        while (st.hasNext()) {
            string name = st.next();
            NBNode *n = NBNodeCont::retrieve(name);
            if (n==0) {
                throw ProcessError(" The node '" + name + "' to set as not-controlled is not known.");

            }
            ncontrolled.push_back(n);
        }
    }

    // loop#2: checking whether the node shall be controlled by a tls due
    //  to the number of lane & their speeds
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        NBNode *cur = (*i).second;
        //  do nothing if already is tl-controlled
        if (cur->isTLControlled()) {
            continue;
        }
        // do nothing if in the list of explicite non-controlled junctions
        if (find(ncontrolled.begin(), ncontrolled.end(), cur)!=ncontrolled.end()) {
            continue;
        }

        // check whether the node has the right amount of incoming edges
        //  to be controlled by a tl
        if ((int) cur->getIncomingEdges().size()<oc.getInt("tls-guess.no-incoming-min")
                ||
                (int) cur->getIncomingEdges().size()>oc.getInt("tls-guess.no-incoming-max")) {

            // nope...
            continue;
        }

        // check whether the node has the right amount of outgoing edges
        //  to be controlled by a tl
        if ((int) cur->getOutgoingEdges().size()<oc.getInt("tls-guess.no-outgoing-min")
                ||
                (int) cur->getOutgoingEdges().size()>oc.getInt("tls-guess.no-outgoing-max")) {

            // nope...
            continue;
        }

        // check whether the edges have the correct speed
        //  to be controlled by a tl
        if (NBContHelper::getMinSpeed(cur->getIncomingEdges())<oc.getFloat("tls-guess.min-incoming-speed")
                ||
                NBContHelper::getMaxSpeed(cur->getIncomingEdges())>oc.getFloat("tls-guess.max-incoming-speed")
                ||
                NBContHelper::getMinSpeed(cur->getOutgoingEdges())<oc.getFloat("tls-guess.min-outgoing-speed")
                ||
                NBContHelper::getMaxSpeed(cur->getOutgoingEdges())>oc.getFloat("tls-guess.max-outgoing-speed")) {

            // nope...
            continue;
        }

        // hmmm, should be tls-controlled (probably)
        setAsTLControlled((*i).first, tlc);

    }
}


void
NBNodeCont::setAsTLControlled(const std::string &name,
                              NBTrafficLightLogicCont &tlc)
{
    NBNode *node = retrieve(name);
    if (node==0) {
        WRITE_WARNING("Building a tl-logic for node '" + name + "' is not possible."
                      + "\n The node '" + name + "' is not known.");
        return;
    }
    NBTrafficLightDefinition *tlDef =
        new NBOwnTLDef(name, node);
    if (!tlc.insert(name, tlDef)) {
        // actually, nothing should fail here
        WRITE_WARNING("Building a tl-logic for node '" + name + "' twice is not possible.");
        delete tlDef;
        return;
    }
}


bool
NBNodeCont::savePlain(const std::string &file)
{
    OutputDevice& device = OutputDevice::getDevice(file);
    device.writeXMLHeader("nodes");
    for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
        NBNode *n = (*i).second;
        device << "   <node id=\"" << n->getID() << "\" ";
        device << "x=\"" << n->getPosition().x() << "\" y=\"" << n->getPosition().y() << "\"/>\n";
    }
    device.close();
    return true;
}

bool
NBNodeCont::writeTLSasPOIs(const std::string &file)
{
    try {
        OutputDevice& device = OutputDevice::getDevice(file);
        device.writeXMLHeader("pois");
        for (NodeCont::iterator i=myNodes.begin(); i!=myNodes.end(); i++) {
            NBNode *n = (*i).second;
            if (n->isTLControlled()) {
                device << "   <poi id=\"" << (*i).first
                << "\" type=\"tls controlled node\" color=\"1,1,0\""
                << " x=\"" << n->getPosition().x() << "\" y=\"" << n->getPosition().y() << "\"/>\n";
            }
        }
        device.close();
        return true;
    } catch (IOError e) {
        MsgHandler::getErrorInstance()->inform("POI file '" + file + "' could not be opened.");
        return false;
    }
}



/****************************************************************************/

