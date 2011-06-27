/****************************************************************************/
/// @file    NBNode.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// The representation of a single node
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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
#include <cassert>
#include <algorithm>
#include <vector>
#include <deque>
#include <set>
#include <cmath>
#include <iterator>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/Line.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/bezier.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ToString.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>
#include <iomanip>
#include "NBNode.h"
#include "NBNodeCont.h"
#include "NBNodeShapeComputer.h"
#include "NBEdgeCont.h"
#include "NBTypeCont.h"
#include "NBHelpers.h"
#include "NBDistrict.h"
#include "NBContHelper.h"
#include "NBRequest.h"
#include "NBOwnTLDef.h"
#include "NBTrafficLightLogicCont.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// some constant definitions
// ===========================================================================
/** Definition how many points an internal lane-geometry should be made of */
#define NO_INTERNAL_POINTS 5

// ===========================================================================
// static members
// ===========================================================================

// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * NBNode::ApproachingDivider-methods
 * ----------------------------------------------------------------------- */
NBNode::ApproachingDivider::ApproachingDivider(
    EdgeVector *approaching, NBEdge *currentOutgoing) throw()
        : myApproaching(approaching), myCurrentOutgoing(currentOutgoing) {
    // check whether origin lanes have been given
    assert(myApproaching!=0);
}


NBNode::ApproachingDivider::~ApproachingDivider() throw() {}


void
NBNode::ApproachingDivider::execute(const unsigned int src, const unsigned int dest) throw() {
    assert(myApproaching->size()>src);
    // get the origin edge
    NBEdge *incomingEdge = (*myApproaching)[src];
    if (incomingEdge->getStep()==NBEdge::LANES2LANES_DONE||incomingEdge->getStep()==NBEdge::LANES2LANES_USER) {
        return;
    }
    std::vector<int> approachingLanes =
        incomingEdge->getConnectionLanes(myCurrentOutgoing);
    assert(approachingLanes.size()!=0);
    std::deque<int> *approachedLanes = spread(approachingLanes, dest);
    assert(approachedLanes->size()<=myCurrentOutgoing->getNoLanes());
    // set lanes
    for (unsigned int i=0; i<approachedLanes->size(); i++) {
        unsigned int approached = (*approachedLanes)[i];
        assert(approachedLanes->size()>i);
        assert(approachingLanes.size()>i);
        incomingEdge->setConnection((unsigned int) approachingLanes[i], myCurrentOutgoing,
                                    approached, NBEdge::L2L_COMPUTED);
    }
    delete approachedLanes;
}


std::deque<int> *
NBNode::ApproachingDivider::spread(const std::vector<int> &approachingLanes,
                                   int dest) const {
    std::deque<int> *ret = new std::deque<int>();
    unsigned int noLanes = (unsigned int) approachingLanes.size();
    // when only one lane is approached, we check, whether the SUMOReal-value
    //  is assigned more to the left or right lane
    if (noLanes==1) {
        ret->push_back(dest);
        return ret;
    }

    unsigned int noOutgoingLanes = myCurrentOutgoing->getNoLanes();
    //
    ret->push_back(dest);
    unsigned int noSet = 1;
    int roffset = 1;
    int loffset = 1;
    while (noSet<noLanes) {
        // It may be possible, that there are not enough lanes the source
        //  lanes may be divided on
        //  In this case, they remain unset
        //  !!! this is only a hack. It is possible, that this yields in
        //   uncommon divisions
        if (noOutgoingLanes==noSet)
            return ret;

        // as due to the conversion of SUMOReal->uint the numbers will be lower
        //  than they should be, we try to append to the left side first
        //
        // check whether the left boundary of the approached street has
        //  been overridden; if so, move all lanes to the right
        if (dest+loffset >= static_cast<int>(noOutgoingLanes)) {
            loffset -= 1;
            roffset += 1;
            for (unsigned int i=0; i<ret->size(); i++) {
                (*ret)[i] = (*ret)[i] - 1;
            }
        }
        // append the next lane to the left of all edges
        //  increase the position (destination edge)
        ret->push_back(dest+loffset);
        noSet++;
        loffset += 1;

        // as above
        if (noOutgoingLanes==noSet)
            return ret;

        // now we try to append the next lane to the right side, when needed
        if (noSet<noLanes) {
            // check whether the right boundary of the approached street has
            //  been overridden; if so, move all lanes to the right
            if (dest < roffset) {
                loffset += 1;
                roffset -= 1;
                for (unsigned int i=0; i<ret->size(); i++) {
                    (*ret)[i] = (*ret)[i] + 1;
                }
            }
            ret->push_front(dest-roffset);
            noSet++;
            roffset += 1;
        }
    }
    return ret;
}




/* -------------------------------------------------------------------------
 * NBNode-methods
 * ----------------------------------------------------------------------- */
NBNode::NBNode(const std::string &id, const Position &position) throw() : 
    Named(StringUtils::convertUmlaute(id)), 
    myPosition(position),
    myType(NODETYPE_UNKNOWN), myDistrict(0), myRequest(0) 
{ }


NBNode::NBNode(const std::string &id, const Position &position,
               SumoXMLNodeType type) throw() : 
    Named(StringUtils::convertUmlaute(id)), 
    myPosition(position),
    myType(type), myDistrict(0), myRequest(0) 
{ }


NBNode::NBNode(const std::string &id, const Position &position, NBDistrict *district) throw() : 
    Named(StringUtils::convertUmlaute(id)), 
    myPosition(position),
    myType(NODETYPE_DISTRICT), myDistrict(district), myRequest(0) 
{ }


NBNode::~NBNode() throw() {
    delete myRequest;
}


void
NBNode::reinit(const Position &position, SumoXMLNodeType type) throw() {
    myPosition = position;
    // patch type
    myType = type;
    if (myType!=NODETYPE_TRAFFIC_LIGHT) {
        removeTrafficLights();
    }
}



// -----------  Methods for dealing with assigned traffic lights
void
NBNode::addTrafficLight(NBTrafficLightDefinition *tld) throw() {
    myTrafficLights.insert(tld);
    myType = NODETYPE_TRAFFIC_LIGHT;
}


void
NBNode::removeTrafficLights() throw() {
    for (std::set<NBTrafficLightDefinition*>::const_iterator i=myTrafficLights.begin(); i!=myTrafficLights.end(); ++i) {
        (*i)->removeNode(this);
    }
    myTrafficLights.clear();
}


bool
NBNode::isJoinedTLSControlled() const throw() {
    if (!isTLControlled()) {
        return false;
    }
    for (std::set<NBTrafficLightDefinition*>::const_iterator i=myTrafficLights.begin(); i!=myTrafficLights.end(); ++i) {
        if ((*i)->getID().find("joined")==0) {
            return true;
        }
    }
    return false;
}


// -----------
void
NBNode::addIncomingEdge(NBEdge *edge) {
    assert(edge!=0);
    if (find(myIncomingEdges.begin(), myIncomingEdges.end(), edge)==myIncomingEdges.end()) {
        myIncomingEdges.push_back(edge);
        myAllEdges.push_back(edge);
    }
}


void
NBNode::addOutgoingEdge(NBEdge *edge) {
    assert(edge!=0);
    if (find(myOutgoingEdges.begin(), myOutgoingEdges.end(), edge)==myOutgoingEdges.end()) {
        myOutgoingEdges.push_back(edge);
        myAllEdges.push_back(edge);
    }
}


bool
NBNode::swapWhenReversed(bool leftHand,
                         const EdgeVector::iterator &i1,
                         const EdgeVector::iterator &i2) {
    NBEdge *e1 = *i1;
    NBEdge *e2 = *i2;
    if (leftHand) {
        if (e1->getToNode()==this && e1->isTurningDirectionAt(this, e2)) {
            std::swap(*i1, *i2);
            return true;
        }
    } else {
        if (e2->getToNode()==this && e2->isTurningDirectionAt(this, e1)) {
            std::swap(*i1, *i2);
            return true;
        }
    }
    return false;
}

void
NBNode::setPriorities() {
    // reset all priorities
    EdgeVector::iterator i;
    // check if the junction is not a real junction
    if (myIncomingEdges.size()==1&&myOutgoingEdges.size()==1) {
        for (i=myAllEdges.begin(); i!=myAllEdges.end(); i++) {
            (*i)->setJunctionPriority(this, 1);
        }
        return;
    }
    // preset all junction's edge priorities to zero
    for (i=myAllEdges.begin(); i!=myAllEdges.end(); i++) {
        (*i)->setJunctionPriority(this, 0);
    }
    // compute the priorities on junction when needed
    if (myType!=NODETYPE_RIGHT_BEFORE_LEFT) {
        setPriorityJunctionPriorities();
    }
}


SumoXMLNodeType
NBNode::computeType(const NBTypeCont &tc) const {
    // the type may already be set from the data
    if (myType!=NODETYPE_UNKNOWN) {
        return myType;
    }
    // check whether the junction is not a real junction
    if (myIncomingEdges.size()==1) {
        return NODETYPE_PRIORITY_JUNCTION;
    }
    if (isSimpleContinuation()) {
        return NODETYPE_PRIORITY_JUNCTION;
    }
    // choose the uppermost type as default
    SumoXMLNodeType type = NODETYPE_RIGHT_BEFORE_LEFT;
    // determine the type
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        for (EdgeVector::const_iterator j=i+1; j!=myIncomingEdges.end(); j++) {
            bool isOpposite = false;
            if (getOppositeIncoming(*j)==*i&&myIncomingEdges.size()>2) {
                isOpposite = true;
            }

            // This usage of defaults is not very well, still we do not have any
            //  methods for the conversion of foreign, sometimes not supplied
            //  road types into an own format
            SumoXMLNodeType tmptype = type;
            if (!isOpposite) {
                tmptype = tc.getJunctionType((*i)->getSpeed(), (*j)->getSpeed());
                if (tmptype<type&&tmptype!=NODETYPE_UNKNOWN&&tmptype!=NODETYPE_NOJUNCTION) {
                    type = tmptype;
                }
            }
        }
    }
    return type;
}


bool
NBNode::isSimpleContinuation() const {
    // one in, one out->continuation
    if (myIncomingEdges.size()==1&&myOutgoingEdges.size()==1) {
        // both must have the same number of lanes
        return (*(myIncomingEdges.begin()))->getNoLanes()==(*(myOutgoingEdges.begin()))->getNoLanes();
    }
    // two in and two out and both in reverse direction
    if (myIncomingEdges.size()==2&&myOutgoingEdges.size()==2) {
        for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
            NBEdge *in = *i;
            EdgeVector::const_iterator opposite = find_if(myOutgoingEdges.begin(), myOutgoingEdges.end(), NBContHelper::opposite_finder(in, this));
            // must have an opposite edge
            if (opposite==myOutgoingEdges.end()) {
                return false;
            }
            // both must have the same number of lanes
            NBContHelper::nextCW(myOutgoingEdges, opposite);
            if (in->getNoLanes()!=(*opposite)->getNoLanes()) {
                return false;
            }
        }
        return true;
    }
    // nope
    return false;
}


bool
samePriority(NBEdge *e1, NBEdge *e2) {
    if (e1==e2) {
        return true;
    }
    if (e1->getPriority()!=e2->getPriority()) {
        return false;
    }
    if ((int) e1->getSpeed()!=(int) e2->getSpeed()) {
        return false;
    }
    return (int) e1->getNoLanes()==(int) e2->getNoLanes();
}


void
NBNode::setPriorityJunctionPriorities() {
    if (myIncomingEdges.size()==0||myOutgoingEdges.size()==0) {
        return;
    }
    EdgeVector incoming = myIncomingEdges;
    EdgeVector outgoing = myOutgoingEdges;
    // what we do want to have is to extract the pair of roads that are
    //  the major roads for this junction
    // let's get the list of incoming edges with the highest priority
    std::sort(incoming.begin(), incoming.end(), NBContHelper::edge_by_priority_sorter());
    EdgeVector bestIncoming;
    NBEdge *best = incoming[0];
    while (incoming.size()>0&&samePriority(best, incoming[0])) {
        bestIncoming.push_back(*incoming.begin());
        incoming.erase(incoming.begin());
    }
    // now, let's get the list of best outgoing
    assert(outgoing.size()!=0);
    sort(outgoing.begin(), outgoing.end(), NBContHelper::edge_by_priority_sorter());
    EdgeVector bestOutgoing;
    best = outgoing[0];
    while (outgoing.size()>0&&samePriority(best, outgoing[0])) {//->getPriority()==best->getPriority()) {
        bestOutgoing.push_back(*outgoing.begin());
        outgoing.erase(outgoing.begin());
    }
    // now, let's compute for each of the best incoming edges
    //  the incoming which is most opposite
    //  the outgoing which is most opposite
    EdgeVector::iterator i;
    std::map<NBEdge*, NBEdge*> counterIncomingEdges;
    std::map<NBEdge*, NBEdge*> counterOutgoingEdges;
    incoming = myIncomingEdges;
    outgoing = myOutgoingEdges;
    for (i=bestIncoming.begin(); i!=bestIncoming.end(); ++i) {
        std::sort(incoming.begin(), incoming.end(), NBContHelper::edge_opposite_direction_sorter(*i, this));
        counterIncomingEdges[*i] = *incoming.begin();
        std::sort(outgoing.begin(), outgoing.end(), NBContHelper::edge_opposite_direction_sorter(*i, this));
        counterOutgoingEdges[*i] = *outgoing.begin();
    }
    // ok, let's try
    // 1) there is one best incoming road
    if (bestIncoming.size()==1) {
        // let's mark this road as the best
        NBEdge *best1 = extractAndMarkFirst(bestIncoming);
        if (bestOutgoing.size()!=0) {
            // mark the best outgoing as the continuation
            sort(bestOutgoing.begin(), bestOutgoing.end(), NBContHelper::edge_similar_direction_sorter(best1));
            extractAndMarkFirst(bestOutgoing);
        }
        return;
    }
    // 2b) there are more than one best incoming roads
    //      and the same number of best outgoing roads

    // ok, what we want to do in this case is to determine which incoming
    //  has the best continuation...
    // This means, when several incoming roads have the same priority,
    //  we want a (any) straight connection to be more priorised than a turning
    SUMOReal bestAngle = 0;
    NBEdge *bestFirst = 0;
    NBEdge *bestSecond = 0;
    bool hadBest = false;
    for (i=bestIncoming.begin(); i!=bestIncoming.end(); ++i) {
        EdgeVector::iterator j;
        NBEdge *t1 = *i;
        SUMOReal angle1 = t1->getAngle()+180;
        if (angle1>=360) {
            angle1 -= 360;
        }
        for (j=i+1; j!=bestIncoming.end(); ++j) {
            NBEdge *t2 = *j;
            SUMOReal angle2 = t2->getAngle()+180;
            if (angle2>=360) {
                angle2 -= 360;
            }
            SUMOReal angle = GeomHelper::getMinAngleDiff(angle1, angle2);
            if (!hadBest||angle>bestAngle) {
                bestAngle = angle;
                bestFirst = *i;
                bestSecond = *j;
                hadBest = true;
            }
        }
    }
    bestFirst->setJunctionPriority(this, 1);
    sort(bestOutgoing.begin(), bestOutgoing.end(), NBContHelper::edge_similar_direction_sorter(bestFirst));
    if (bestOutgoing.size()!=0) {
        extractAndMarkFirst(bestOutgoing);
    }
    bestSecond->setJunctionPriority(this, 1);
    sort(bestOutgoing.begin(), bestOutgoing.end(), NBContHelper::edge_similar_direction_sorter(bestSecond));
    if (bestOutgoing.size()!=0) {
        extractAndMarkFirst(bestOutgoing);
    }
}


NBEdge*
NBNode::extractAndMarkFirst(EdgeVector &s) {
    if (s.size()==0) {
        return 0;
    }
    NBEdge *ret = s.front();
    s.erase(s.begin());
    ret->setJunctionPriority(this, 1);
    return ret;
}


unsigned int
NBNode::countInternalLanes(bool includeSplits) const {
    unsigned int lno = 0;
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        unsigned int noLanesEdge = (*i)->getNoLanes();
        for (unsigned int j=0; j<noLanesEdge; j++) {
            std::vector<NBEdge::Connection> elv = (*i)->getConnectionsFromLane(j);
            for (std::vector<NBEdge::Connection>::iterator k=elv.begin(); k!=elv.end(); ++k) {
                if ((*k).toEdge==0) {
                    continue;
                }
                lno++;
                // add internal splits if any
                if (includeSplits) {
                    std::pair<SUMOReal, std::vector<unsigned int> > cross = getCrossingPosition(*i, j, (*k).toEdge, (*k).toLane);
                    if (cross.first>=0) {
                        lno++;
                    }
                }
            }
        }
    }
    return lno;
}


PositionVector
NBNode::computeInternalLaneShape(NBEdge *fromE, int fromL,
                                 NBEdge *toE, int toL) const {
    if (fromL>=(int) fromE->getNoLanes()) {
        throw ProcessError("Connection '" + fromE->getID() + "_" + toString(fromL) + "->" + toE->getID() + "_" + toString(toL) + "' starts at a not existing lane.");
    }
    if (toL>=(int) toE->getNoLanes()) {
        throw ProcessError("Connection '" + fromE->getID() + "_" + toString(fromL) + "->" + toE->getID() + "_" + toString(toL) + "' yields in a not existing lane.");
    }
    bool noSpline = false;
    PositionVector ret;
    PositionVector init;
    Position beg = fromE->getLaneShape(fromL).getEnd();
    Position end = toE->getLaneShape(toL).getBegin();
    Position intersection;
    unsigned int noInitialPoints = 0;
    if (beg.distanceTo(end) <= POSITION_EPS) {
        noSpline = true;
    } else {
        if (fromE->getTurnDestination()==toE) {
            // turnarounds:
            //  - end of incoming lane
            //  - position between incoming/outgoing end/begin shifted by the distace orthogonally
            //  - begin of outgoing lane
            noInitialPoints = 3;
            init.push_back(beg);
            Line straightConn(fromE->getLaneShape(fromL)[-1],toE->getLaneShape(toL)[0]);
            Position straightCenter = straightConn.getPositionAtDistance((SUMOReal) straightConn.length() / (SUMOReal) 2.);
            Position center = straightCenter;//.add(straightCenter);
            Line cross(straightConn);
            cross.sub(cross.p1().x(), cross.p1().y());
            cross.rotateAtP1(PI/2);
            center.sub(cross.p2());
            init.push_back(center);
            init.push_back(end);
        } else {
            const SUMOReal angle = fabs(fromE->getLaneShape(fromL).getEndLine().atan2Angle() - toE->getLaneShape(toL).getBegLine().atan2Angle());
            if (angle < PI/4. || angle > 7./4.*PI) {
                // very low angle: almost straight
                noInitialPoints = 4;
                init.push_back(beg);
                Line begL = fromE->getLaneShape(fromL).getEndLine();
                begL.extrapolateSecondBy(100);
                Line endL = toE->getLaneShape(toL).getBegLine();
                endL.extrapolateFirstBy(100);
                SUMOReal distance = beg.distanceTo(end);
                if (distance>10) {
                    {
                        SUMOReal off1 = fromE->getLaneShape(fromL).getEndLine().length() + (SUMOReal) 5. * (SUMOReal) fromE->getNoLanes();
                        off1 = MIN2(off1, (SUMOReal)(fromE->getLaneShape(fromL).getEndLine().length()+distance/2.));
                        Position tmp = begL.getPositionAtDistance(off1);
                        init.push_back(tmp);
                    }
                    {
                        SUMOReal off1 = (SUMOReal) 100. - (SUMOReal) 5. * (SUMOReal) toE->getNoLanes();
                        off1 = MAX2(off1, (SUMOReal)(100.-distance/2.));
                        Position tmp = endL.getPositionAtDistance(off1);
                        init.push_back(tmp);
                    }
                } else {
                    noSpline = true;
                }
                init.push_back(end);
            } else {
                // turning
                //  - end of incoming lane
                //  - intersection of the extrapolated lanes
                //  - begin of outgoing lane
                // attention: if there is no intersection, use a straight line
                noInitialPoints = 3;
                init.push_back(beg);
                Line begL = fromE->getLaneShape(fromL).getEndLine();
                Line endL = toE->getLaneShape(toL).getBegLine();
                bool check = !begL.p1().almostSame(begL.p2()) && !endL.p1().almostSame(endL.p2());
                if (check) {
                    begL.extrapolateSecondBy(100);
                    endL.extrapolateFirstBy(100);
                } else {
                    MsgHandler::getWarningInstance()->inform("Could not use edge geometry for internal lane, node '" + getID() + "'.");
                }
                if (!check||!begL.intersects(endL)) {
                    noSpline = true;
                } else {
                    init.push_back(begL.intersectsAt(endL));
                }
                init.push_back(end);
            }
        }
    }
    //
    if (noSpline) {
        ret.push_back(fromE->getLaneShape(fromL).getEnd());
        ret.push_back(toE->getLaneShape(toL).getBegin());
    } else {
        SUMOReal *def = new SUMOReal[1+noInitialPoints*3];
        for (int i=0; i<(int) init.size(); ++i) {
            // starts at index 1
            def[i*3+1] = init[i].x();
            def[i*3+2] = 0;
            def[i*3+3] = init[i].y();
        }
        SUMOReal ret_buf[NO_INTERNAL_POINTS*3+1];
        bezier(noInitialPoints, def, NO_INTERNAL_POINTS, ret_buf);
        delete[] def;
        Position prev;
        for (int i=0; i<(int) NO_INTERNAL_POINTS; i++) {
            Position current(ret_buf[i*3+1], ret_buf[i*3+3]);
            if (prev!=current) {
                ret.push_back(current);
            }
            prev = current;
        }
    }
    const NBEdge::Lane &lane = fromE->getLaneStruct(fromL);
    if(lane.offset>0) {
        PositionVector beg = lane.shape.getSubpart(lane.shape.length()-lane.offset, lane.shape.length());;
        beg.appendWithCrossingPoint(ret);
        ret = beg;
    }
    return ret;
}


bool
NBNode::needsCont(NBEdge *fromE, NBEdge *toE, NBEdge *otherFromE, NBEdge *otherToE, NBEdge::Connection &c) const {
    if(myType==NODETYPE_RIGHT_BEFORE_LEFT) {
        return false;
    }
    if (fromE==otherFromE) {
        // ignore same edge links
        return false;
    }
    if (!foes(otherFromE, otherToE, fromE, toE)) {
        // if they do not cross, no waiting place is needed
        return false;
    }
    LinkDirection d1 = getDirection(fromE, toE);
    LinkDirection d2 = getDirection(otherFromE, otherToE);
    bool thisLeft = (d1==LINKDIR_LEFT||d1==LINKDIR_TURN);
    bool otherLeft = (d2==LINKDIR_LEFT||d2==LINKDIR_TURN);
    bool bothLeft = thisLeft&&otherLeft;
    if (c.tlID!=""&&!bothLeft) {
        // tls-controlled links will have space
        return true;
    }
    if (fromE->getJunctionPriority(this)>0&&otherFromE->getJunctionPriority(this)>0) {
        return true;
    }
    return false;
}


std::pair<SUMOReal, std::vector<unsigned int> >
NBNode::getCrossingPosition(NBEdge *fromE, unsigned int fromL, NBEdge *toE, unsigned int toL) const {
    std::pair<SUMOReal, std::vector<unsigned int> > ret(-1, std::vector<unsigned int>());
    LinkDirection dir = getDirection(fromE, toE);
    switch (dir) {
    case LINKDIR_LEFT:
    case LINKDIR_PARTLEFT:
    case LINKDIR_TURN: {
        PositionVector thisShape = computeInternalLaneShape(fromE, fromL, toE, toL);
        unsigned int index = 0;
        for (EdgeVector::const_iterator i2=myIncomingEdges.begin(); i2!=myIncomingEdges.end(); i2++) {
            unsigned int noLanesEdge = (*i2)->getNoLanes();
            for (unsigned int j2=0; j2<noLanesEdge; j2++) {
                std::vector<NBEdge::Connection> elv = (*i2)->getConnectionsFromLane(j2);
                for (std::vector<NBEdge::Connection>::iterator k2=elv.begin(); k2!=elv.end(); k2++) {
                    if ((*k2).toEdge==0) {
                        continue;
                    }
                    if (needsCont(fromE, toE, *i2, (*k2).toEdge, *k2)) {
                        // compute the crossing point
                        ret.second.push_back(index);
                        PositionVector otherShape = computeInternalLaneShape(*i2, j2, (*k2).toEdge, (*k2).toLane);
                        if (thisShape.intersects(otherShape)) {
                            DoubleVector dv = thisShape.intersectsAtLengths(otherShape);
                            SUMOReal minDV = dv[0];
                            if (minDV<thisShape.length()-.1&&minDV>.1) { // !!!?
                                assert(minDV>=0);
                                if (ret.first<0||ret.first>minDV) {
                                    ret.first = minDV;
                                }
                            }
                        }
                    }
                    index++;
                }
            }
        }
        if (dir==LINKDIR_TURN&&ret.first<0&&ret.second.size()!=0) {
            // let turnarounds wait at the begin if no other crossing point was found
            ret.first = (SUMOReal) thisShape.length() / 2.;
        }
    }
    break;
    default:
        break;
    }
    return ret;
}


std::string
NBNode::getCrossingNames_dividedBySpace(NBEdge *fromE, unsigned int fromL,
                                        NBEdge *toE, unsigned int toL) const {
    std::string ret;
    LinkDirection dir = getDirection(fromE, toE);
    switch (dir) {
    case LINKDIR_LEFT:
    case LINKDIR_PARTLEFT:
    case LINKDIR_TURN: {
        PositionVector thisShape = computeInternalLaneShape(fromE, fromL, toE, toL);
        unsigned int index = 0;
        for (EdgeVector::const_iterator i2=myIncomingEdges.begin(); i2!=myIncomingEdges.end(); i2++) {
            unsigned int noLanesEdge = (*i2)->getNoLanes();
            for (unsigned int j2=0; j2<noLanesEdge; j2++) {
                std::vector<NBEdge::Connection> elv = (*i2)->getConnectionsFromLane(j2);
                for (std::vector<NBEdge::Connection>::iterator k2=elv.begin(); k2!=elv.end(); k2++) {
                    if ((*k2).toEdge==0) {
                        continue;
                    }
                    NBEdge *e = fromE->getToNode()->getOppositeIncoming(fromE);
                    if (e!=*i2) {
                        index++;
                        continue;
                    }
//                    LinkDirection dir2 = getDirection(*i2, (*k2).toEdge);
                    if (needsCont(fromE, toE, *i2, (*k2).toEdge, *k2)) {
                        if (ret.length()!=0) {
                            ret += " ";
                        }
                        ret += (":" + myID + "_" + toString(index) + "_0");
                    }
                    index++;
                }
            }
        }
    }
    break;
    default:
        break;
    }
    return ret;
}


std::string
NBNode::getCrossingSourcesNames_dividedBySpace(NBEdge *fromE, unsigned int fromL,
        NBEdge *toE, unsigned int toL) const {
    std::string ret;
    std::vector<std::string> tmp;
    LinkDirection dir = getDirection(fromE, toE);
    switch (dir) {
    case LINKDIR_LEFT:
    case LINKDIR_PARTLEFT:
    case LINKDIR_TURN: {
        PositionVector thisShape = computeInternalLaneShape(fromE, fromL, toE, toL);
        unsigned int index = 0;
        for (EdgeVector::const_iterator i2=myIncomingEdges.begin(); i2!=myIncomingEdges.end(); i2++) {
            unsigned int noLanesEdge = (*i2)->getNoLanes();
            for (unsigned int j2=0; j2<noLanesEdge; j2++) {
                std::vector<NBEdge::Connection> elv = (*i2)->getConnectionsFromLane(j2);
                for (std::vector<NBEdge::Connection>::iterator k2=elv.begin(); k2!=elv.end(); k2++) {
                    if ((*k2).toEdge==0) {
                        continue;
                    }
                    if((*k2).mayDefinitelyPass) {
                        index++;
                        continue;
                    }
                    NBEdge *e = fromE->getToNode()->getOppositeIncoming(fromE);
                    if (e!=*i2) {
                        index++;
                        continue;
                    }
                    if (needsCont(fromE, toE, *i2, (*k2).toEdge, *k2)) {
                        std::string nid = (*i2)->getID() + "_" + toString(j2);
                        if (find(tmp.begin(), tmp.end(), nid)==tmp.end()) {
                            tmp.push_back(nid);
                        }
                    }
                    index++;
                }
            }
        }
    }
    break;
    default:
        break;
    }
    for (std::vector<std::string>::iterator i=tmp.begin(); i!=tmp.end(); ++i) {
        if (ret.length()>0) {
            ret = ret + " ";
        }
        ret = ret + *i;
    }
    return ret;
}


void
NBNode::computeLogic(const NBEdgeCont &ec, OptionsCont &oc) {
    delete myRequest; // possibly recomputation step
    if (myIncomingEdges.size()==0||myOutgoingEdges.size()==0) {
        // no logic if nothing happens here
        myType = NODETYPE_NOJUNCTION;
        return;
    }
    // check whether the node was set to be unregulated by the user
    if (oc.getBool("keep-nodes-unregulated") || oc.isInStringVector("keep-nodes-unregulated.explicit", getID())
            || (oc.getBool("keep-nodes-unregulated.district-nodes")&&(isNearDistrict()||isDistrict()))) {
        myType = NODETYPE_NOJUNCTION;
        return;
    }
    // compute the logic if necessary or split the junction
    if (myType!=NODETYPE_NOJUNCTION&&myType!=NODETYPE_DISTRICT) {
        // build the request
        myRequest = new NBRequest(ec, this, 
                myAllEdges, myIncomingEdges, myOutgoingEdges, myBlockedConnections);
        // check whether it is not too large
        if (myRequest->getSizes().second>=64) {
            // yep -> make it untcontrolled, warn
            MsgHandler::getWarningInstance()->inform("Junction '" + getID() + "' is too complicated (#links>64); will be set to unregulated.");
            delete myRequest;
            myRequest = 0;
            myType = NODETYPE_NOJUNCTION;
        } else {
            myRequest->buildBitfieldLogic(ec.isLeftHanded());
        }
    }
}


bool
NBNode::writeLogic(OutputDevice &into) {
	if (myRequest) {
		myRequest->writeLogic(myID, into);
		return true;
	}
	return false;
}


void
NBNode::sortNodesEdges(bool leftHand, const NBTypeCont &tc) {
    // sort the edges
    sort(myAllEdges.begin(), myAllEdges.end(), NBContHelper::edge_by_junction_angle_sorter(this));
    sort(myIncomingEdges.begin(), myIncomingEdges.end(), NBContHelper::edge_by_junction_angle_sorter(this));
    sort(myOutgoingEdges.begin(), myOutgoingEdges.end(), NBContHelper::edge_by_junction_angle_sorter(this));
    if (myAllEdges.size()==0) {
        return;
    }
    EdgeVector::iterator i;
    for (i=myAllEdges.begin(); i!=myAllEdges.end()-1&&i!=myAllEdges.end(); i++) {
        swapWhenReversed(leftHand ,i, i+1);
    }
    if (myAllEdges.size()>1 && i!=myAllEdges.end()) {
        swapWhenReversed(leftHand, myAllEdges.end()-1, myAllEdges.begin());
    }
    if (myType==NODETYPE_UNKNOWN) {
        myType = computeType(tc);
    }
    setPriorities();
}


void
NBNode::computeNodeShape(bool leftHand) {
    if (myIncomingEdges.size()==0&&myOutgoingEdges.size()==0) {
        return;
    }
    try {
        NBNodeShapeComputer computer(*this);
        myPoly = computer.compute(leftHand);
    } catch (InvalidArgument &) {
        MsgHandler::getWarningInstance()->inform("For node '" + getID() + "': could not compute shape.");
    }
}


void
NBNode::computeLanes2Lanes() {
    // special case a):
    //  one in, one out, the outgoing has one lane more
    if (myIncomingEdges.size()==1&&myOutgoingEdges.size()==1
            && myIncomingEdges[0]->getNoLanes()==myOutgoingEdges[0]->getNoLanes()-1
            && myIncomingEdges[0]!=myOutgoingEdges[0]
            && myIncomingEdges[0]->isConnectedTo(myOutgoingEdges[0])) {

        NBEdge *incoming = myIncomingEdges[0];
        NBEdge *outgoing = myOutgoingEdges[0];
        // check if it's not the turnaround
        if (incoming->getTurnDestination()==outgoing) {
            // will be added later or not...
            return;
        }
        for (int i=0; i<(int) incoming->getNoLanes(); ++i) {
            incoming->setConnection(i, outgoing, i+1, NBEdge::L2L_COMPUTED);
        }
        incoming->setConnection(0, outgoing, 0, NBEdge::L2L_COMPUTED);
        return;
    }
    // special case b):
    //  two in, one out, the outgoing has the same number of lanes as the sum of the incoming
    //  and a high speed, too
    //  --> highway on-ramp
    bool check = false;
    if (myIncomingEdges.size()==2&&myOutgoingEdges.size()==1) {
        check = myIncomingEdges[0]->getNoLanes()+myIncomingEdges[1]->getNoLanes()==myOutgoingEdges[0]->getNoLanes();
        check &= (myIncomingEdges[0]->getStep() <= NBEdge::LANES2EDGES);
        check &= (myIncomingEdges[1]->getStep() <= NBEdge::LANES2EDGES);
    }
    if (check
            && myIncomingEdges[0]!=myOutgoingEdges[0]
            && myIncomingEdges[0]->isConnectedTo(myOutgoingEdges[0])) {
        NBEdge *inc1 = myIncomingEdges[0];
        NBEdge *inc2 = myIncomingEdges[1];
        // for internal: check which one is the rightmost
        SUMOReal a1 = inc1->getAngle(*this);
        SUMOReal a2 = inc2->getAngle(*this);
        SUMOReal ccw = GeomHelper::getCCWAngleDiff(a1, a2);
        SUMOReal cw = GeomHelper::getCWAngleDiff(a1, a2);
        if (ccw<cw) {
            std::swap(inc1, inc2);
        }
        //
        inc1->addLane2LaneConnections(0, myOutgoingEdges[0], 0, inc1->getNoLanes(), NBEdge::L2L_VALIDATED, true, true);
        inc2->addLane2LaneConnections(0, myOutgoingEdges[0], inc1->getNoLanes(), inc2->getNoLanes(), NBEdge::L2L_VALIDATED, true, true);
        return;
    }

    // go through this node's outgoing edges
    //  for every outgoing edge, compute the distribution of the node's
    //  incoming edges on this edge when approaching this edge
    // the incoming edges' steps will then also be marked as LANE2LANE_RECHECK...
    EdgeVector::reverse_iterator i;
    for (i=myOutgoingEdges.rbegin(); i!=myOutgoingEdges.rend(); i++) {
        NBEdge *currentOutgoing = *i;
        // get the information about edges that do approach this edge
        EdgeVector *approaching = getEdgesThatApproach(currentOutgoing);
        if (approaching->size()!=0) {
            ApproachingDivider divider(approaching, currentOutgoing);
            Bresenham::compute(&divider, static_cast<unsigned int>(approaching->size()),
                               currentOutgoing->getNoLanes());
        }
        delete approaching;
    }
    // ... but we may have the case that there are no outgoing edges
    //  In this case, we have to mark the incoming edges as being in state
    //   LANE2LANE( not RECHECK) by hand
    if (myOutgoingEdges.size()==0) {
        for (i=myIncomingEdges.rbegin(); i!=myIncomingEdges.rend(); i++) {
            (*i)->markAsInLane2LaneState();
        }
    }
}


EdgeVector *
NBNode::getEdgesThatApproach(NBEdge *currentOutgoing) {
    // get the position of the node to get the approaching nodes of
    EdgeVector::const_iterator i = find(myAllEdges.begin(),
            myAllEdges.end(), currentOutgoing);
    // get the first possible approaching edge
    NBContHelper::nextCW(myAllEdges, i);
    // go through the list of edges clockwise and add the edges
    EdgeVector *approaching = new EdgeVector();
    for (; *i!=currentOutgoing;) {
        // check only incoming edges
        if ((*i)->getToNode()==this&&(*i)->getTurnDestination()!=currentOutgoing) {
            std::vector<int> connLanes = (*i)->getConnectionLanes(currentOutgoing);
            if (connLanes.size()!=0) {
                approaching->push_back(*i);
            }
        }
        NBContHelper::nextCW(myAllEdges, i);
    }
    return approaching;
}


void
NBNode::reshiftPosition(SUMOReal xoff, SUMOReal yoff) {
    myPosition.reshiftRotate(xoff, yoff, 0);
    myPoly.reshiftRotate(xoff, yoff, 0);
}


void
NBNode::replaceOutgoing(NBEdge *which, NBEdge *by, unsigned int laneOff) {
    // replace the edge in the list of outgoing nodes
    EdgeVector::iterator i=find(myOutgoingEdges.begin(), myOutgoingEdges.end(), which);
    if (i!=myOutgoingEdges.end()) {
        (*i) = by;
        i = find(myAllEdges.begin(), myAllEdges.end(), which);
        (*i) = by;
    }
    // replace the edge in connections of incoming edges
    for (i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); ++i) {
        (*i)->replaceInConnections(which, by, laneOff);
    }
    // replace within the connetion prohibition dependencies
    replaceInConnectionProhibitions(which, by, 0, laneOff);
}


void
NBNode::replaceOutgoing(const EdgeVector &which, NBEdge *by) {
    // replace edges
    unsigned int laneOff = 0;
    for (EdgeVector::const_iterator i=which.begin(); i!=which.end(); i++) {
        replaceOutgoing(*i, by, laneOff);
        laneOff += (*i)->getNoLanes();
    }
    // removed SUMOReal occurences
    removeDoubleEdges();
    // check whether this node belongs to a district and the edges
    //  must here be also remapped
    if (myDistrict!=0) {
        myDistrict->replaceOutgoing(which, by);
    }
}


void
NBNode::replaceIncoming(NBEdge *which, NBEdge *by, unsigned int laneOff) {
    // replace the edge in the list of incoming nodes
    EdgeVector::iterator i=find(myIncomingEdges.begin(), myIncomingEdges.end(), which);
    if (i!=myIncomingEdges.end()) {
        (*i) = by;
        i = find(myAllEdges.begin(), myAllEdges.end(), which);
        (*i) = by;
    }
    // replace within the connetion prohibition dependencies
    replaceInConnectionProhibitions(which, by, laneOff, 0);
}


void
NBNode::replaceIncoming(const EdgeVector &which, NBEdge *by) {
    // replace edges
    unsigned int laneOff = 0;
    for (EdgeVector::const_iterator i=which.begin(); i!=which.end(); i++) {
        replaceIncoming(*i, by, laneOff);
        laneOff += (*i)->getNoLanes();
    }
    // removed SUMOReal occurences
    removeDoubleEdges();
    // check whether this node belongs to a district and the edges
    //  must here be also remapped
    if (myDistrict!=0) {
        myDistrict->replaceIncoming(which, by);
    }
}



void
NBNode::replaceInConnectionProhibitions(NBEdge *which, NBEdge *by,
                                        unsigned int whichLaneOff, unsigned int byLaneOff) {
    // replace in keys
    NBConnectionProhibits::iterator j = myBlockedConnections.begin();
    while (j!=myBlockedConnections.end()) {
        bool changed = false;
        NBConnection c = (*j).first;
        if (c.replaceFrom(which, whichLaneOff, by, byLaneOff)) {
            changed = true;
        }
        if (c.replaceTo(which, whichLaneOff, by, byLaneOff)) {
            changed = true;
        }
        if (changed) {
            myBlockedConnections[c] = (*j).second;
            myBlockedConnections.erase(j);
            j = myBlockedConnections.begin();
        } else {
            j++;
        }
    }
    // replace in values
    for (j=myBlockedConnections.begin(); j!=myBlockedConnections.end(); j++) {
        NBConnectionVector &prohibiting = (*j).second;
        for (NBConnectionVector::iterator k=prohibiting.begin(); k!=prohibiting.end(); k++) {
            NBConnection &sprohibiting = *k;
            sprohibiting.replaceFrom(which, whichLaneOff, by, byLaneOff);
            sprohibiting.replaceTo(which, whichLaneOff, by, byLaneOff);
        }
    }
}



void
NBNode::removeDoubleEdges() {
    unsigned int i, j;
    // check incoming
    for (i=0; myIncomingEdges.size()>0&&i<myIncomingEdges.size()-1; i++) {
        j = i + 1;
        while (j<myIncomingEdges.size()) {
            if (myIncomingEdges[i]==myIncomingEdges[j]) {
                myIncomingEdges.erase(myIncomingEdges.begin()+j);
            } else {
                j++;
            }
        }
    }
    // check outgoing
    for (i=0; myOutgoingEdges.size()>0&&i<myOutgoingEdges.size()-1; i++) {
        j = i + 1;
        while (j<myOutgoingEdges.size()) {
            if (myOutgoingEdges[i]==myOutgoingEdges[j]) {
                myOutgoingEdges.erase(myOutgoingEdges.begin()+j);
            } else {
                j++;
            }
        }
    }
    // check all
    for (i=0; myAllEdges.size()>0&&i<myAllEdges.size()-1; i++) {
        j = i + 1;
        while (j<myAllEdges.size()) {
            if (myAllEdges[i]==myAllEdges[j]) {
                myAllEdges.erase(myAllEdges.begin()+j);
            } else {
                j++;
            }
        }
    }
}


bool
NBNode::hasIncoming(const NBEdge * const e) const throw() {
    return find(myIncomingEdges.begin(), myIncomingEdges.end(), e)!=myIncomingEdges.end();
}


bool
NBNode::hasOutgoing(const NBEdge * const e) const throw() {
    return find(myOutgoingEdges.begin(), myOutgoingEdges.end(), e)!=myOutgoingEdges.end();
}


NBEdge *
NBNode::getOppositeIncoming(NBEdge *e) const {
    EdgeVector edges = myIncomingEdges;
    if (find(edges.begin(), edges.end(), e)!=edges.end()) {
        edges.erase(find(edges.begin(), edges.end(), e));
    }
    if (e->getToNode()==this) {
        sort(edges.begin(), edges.end(), NBContHelper::edge_opposite_direction_sorter(e, this));
    } else {
        sort(edges.begin(), edges.end(), NBContHelper::edge_similar_direction_sorter(e));
    }
    return edges[0];
}


void
NBNode::addSortedLinkFoes(const NBConnection &mayDrive,
                          const NBConnection &mustStop) {
    if (mayDrive.getFrom()==0 ||
            mayDrive.getTo()==0 ||
            mustStop.getFrom()==0 ||
            mustStop.getTo()==0) {

        WRITE_WARNING("Something went wrong during the building of a connection...");
        return; // !!! mark to recompute connections
    }
    NBConnectionVector conn = myBlockedConnections[mustStop];
    conn.push_back(mayDrive);
    myBlockedConnections[mustStop] = conn;
}


NBEdge *
NBNode::getPossiblySplittedIncoming(const std::string &edgeid) {
    unsigned int size = (unsigned int) edgeid.length();
    for (EdgeVector::iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        std::string id = (*i)->getID();
        if (id.substr(0, size)==edgeid) {
            return *i;
        }
    }
    return 0;
}


NBEdge *
NBNode::getPossiblySplittedOutgoing(const std::string &edgeid) {
    unsigned int size = (unsigned int) edgeid.length();
    for (EdgeVector::iterator i=myOutgoingEdges.begin(); i!=myOutgoingEdges.end(); i++) {
        std::string id = (*i)->getID();
        if (id.substr(0, size)==edgeid) {
            return *i;
        }
    }
    return 0;
}


unsigned int
NBNode::eraseDummies(NBDistrictCont &dc, NBEdgeCont &ec, NBTrafficLightLogicCont &tc) {
    unsigned int ret = 0;
    unsigned int pos = 0;
    EdgeVector::const_iterator j=myIncomingEdges.begin();
    while (j!=myIncomingEdges.end()) {
        // skip edges which are only incoming and not outgoing
        if (find(myOutgoingEdges.begin(), myOutgoingEdges.end(), *j)==myOutgoingEdges.end()) {
            j++;
            pos++;
            continue;
        }
        // an edge with both its origin and destination being the current
        //  node should be removed
        NBEdge *dummy = *j;
        WRITE_WARNING(" Removing dummy edge '" + dummy->getID() + "'");
        // get the list of incoming edges connected to the dummy
        EdgeVector incomingConnected;
        EdgeVector::const_iterator i;
        for (i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
            if ((*i)->isConnectedTo(dummy)&&*i!=dummy) {
                incomingConnected.push_back(*i);
            }
        }
        // get the list of outgoing edges connected to the dummy
        EdgeVector outgoingConnected;
        for (i=myOutgoingEdges.begin(); i!=myOutgoingEdges.end(); i++) {
            if (dummy->isConnectedTo(*i)&&*i!=dummy) {
                outgoingConnected.push_back(*i);
            }
        }
        // let the dummy remap its connections
        dummy->remapConnections(incomingConnected);
        remapRemoved(tc, dummy, incomingConnected, outgoingConnected);
        // delete the dummy
        ec.erase(dc, dummy);
        j = myIncomingEdges.begin() + pos;
        ret++;
    }
    return ret;
}


void
NBNode::removeOutgoing(NBEdge *edge) {
    EdgeVector::iterator i = find(myOutgoingEdges.begin(), myOutgoingEdges.end(), edge);
    if (i!=myOutgoingEdges.end()) {
        myOutgoingEdges.erase(i);
        i = find(myAllEdges.begin(), myAllEdges.end(), edge);
        myAllEdges.erase(i);
        for (i=myAllEdges.begin(); i!=myAllEdges.end(); ++i) {
            (*i)->removeFromConnections(edge);
        }
    }
}


void
NBNode::removeIncoming(NBEdge *edge) {
    EdgeVector::iterator i = find(myIncomingEdges.begin(), myIncomingEdges.end(), edge);
    if (i!=myIncomingEdges.end()) {
        myIncomingEdges.erase(i);
        i = find(myAllEdges.begin(), myAllEdges.end(), edge);
        myAllEdges.erase(i);
        for (i=myAllEdges.begin(); i!=myAllEdges.end(); ++i) {
            (*i)->removeFromConnections(edge);
        }
    }
}




Position
NBNode::getEmptyDir() const {
    Position pos(0, 0);
    EdgeVector::const_iterator i;
    for (i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        NBNode *conn = (*i)->getFromNode();
        Position toAdd = conn->getPosition();
        toAdd.sub(myPosition);
        toAdd.mul((SUMOReal) 1.0/sqrt(toAdd.x()*toAdd.x()+toAdd.y()*toAdd.y()));
        pos.add(toAdd);
    }
    for (i=myOutgoingEdges.begin(); i!=myOutgoingEdges.end(); i++) {
        NBNode *conn = (*i)->getToNode();
        Position toAdd = conn->getPosition();
        toAdd.sub(myPosition);
        toAdd.mul((SUMOReal) 1.0/sqrt(toAdd.x()*toAdd.x()+toAdd.y()*toAdd.y()));
        pos.add(toAdd);
    }
    pos.mul((SUMOReal) -1.0/(myIncomingEdges.size()+myOutgoingEdges.size()));
    if (pos.x()==0&&pos.y()==0) {
        pos = Position(1, 0);
    }
    pos.norm();
    return pos;
}



void
NBNode::invalidateIncomingConnections() {
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        (*i)->invalidateConnections();
    }
}


void
NBNode::invalidateOutgoingConnections() {
    for (EdgeVector::const_iterator i=myOutgoingEdges.begin(); i!=myOutgoingEdges.end(); i++) {
        (*i)->invalidateConnections();
    }
}


bool
NBNode::mustBrake(const NBEdge * const from, const NBEdge * const to, int toLane) const throw() {
    // check whether it is participant to a traffic light
    //  - controlled links are set by the traffic lights, not the normal
    //    right-of-way rules
    //  - uncontrolled participants (spip lanes etc.) should always break
    if (myTrafficLights.size()!=0) {
        // ok, we have a traffic light, return true by now, it will be later
        //  controlled by the tls
        return true;
    }
    // unregulated->does not need to brake
    if (myRequest==0) {
        return false;
    }
    // vehicles which do not have a following lane must always decelerate to the end
    if (to==0) {
        return true;
    }
    // check whether any other connection on this node prohibits this connection
    bool try1 = myRequest->mustBrake(from, to);
    if (!try1||toLane==-1) {
        return try1;
    }
    if (from->getSpeed()<70./3.6) {
        return try1;
    }
    // on highways (on-ramps, in fact):
    // check whether any other connection uses the same destination edge
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        if ((*i)==from) {
            continue;
        }
        const std::vector<NBEdge::Connection> &connections = (*i)->getConnections();
        for (std::vector<NBEdge::Connection>::const_iterator j=connections.begin(); j!=connections.end(); ++j) {
            if ((*j).toEdge==to&&((*j).toLane<0||(*j).toLane==toLane)) {
                return true;
            }
        }
    }
    return false;
}



bool
NBNode::isLeftMover(const NBEdge * const from, const NBEdge * const to) const throw() {
    // when the junction has only one incoming edge, there are no
    //  problems caused by left blockings
    if (myIncomingEdges.size()==1) {
        return false;
    }
    SUMOReal ccw = GeomHelper::getCCWAngleDiff(from->getAngle(*this), to->getAngle(*this));
    SUMOReal cw = GeomHelper::getCWAngleDiff(from->getAngle(*this), to->getAngle(*this));
    return cw<ccw;
}


bool
NBNode::forbids(const NBEdge * const possProhibitorFrom, const NBEdge * const possProhibitorTo,
                const NBEdge * const possProhibitedFrom, const NBEdge * const possProhibitedTo,
                bool regardNonSignalisedLowerPriority) const throw() {
    return myRequest!=0&&myRequest->forbids(possProhibitorFrom, possProhibitorTo,
                                            possProhibitedFrom, possProhibitedTo,
                                            regardNonSignalisedLowerPriority);
}


bool
NBNode::foes(const NBEdge * const from1, const NBEdge * const to1,
             const NBEdge * const from2, const NBEdge * const to2) const throw() {
    return myRequest!=0&&myRequest->foes(from1, to1, from2, to2);
}


void
NBNode::remapRemoved(NBTrafficLightLogicCont &tc,
                     NBEdge *removed, const EdgeVector &incoming,
                     const EdgeVector &outgoing) {
    assert(find(incoming.begin(), incoming.end(), removed)==incoming.end());
    bool changed = true;
    while (changed) {
        changed = false;
        NBConnectionProhibits blockedConnectionsTmp = myBlockedConnections;
        NBConnectionProhibits blockedConnectionsNew;
        // remap in connections
        for (NBConnectionProhibits::iterator i=blockedConnectionsTmp.begin(); i!=blockedConnectionsTmp.end(); i++) {
            const NBConnection &blocker = (*i).first;
            const NBConnectionVector &blocked = (*i).second;
            // check the blocked connections first
            // check whether any of the blocked must be changed
            bool blockedChanged = false;
            NBConnectionVector newBlocked;
            NBConnectionVector::const_iterator j;
            for (j=blocked.begin(); j!=blocked.end(); j++) {
                const NBConnection &sblocked = *j;
                if (sblocked.getFrom()==removed||sblocked.getTo()==removed) {
                    blockedChanged = true;
                }
            }
            // adapt changes if so
            for (j=blocked.begin(); blockedChanged&&j!=blocked.end(); j++) {
                const NBConnection &sblocked = *j;
                if (sblocked.getFrom()==removed&&sblocked.getTo()==removed) {
                    /*                    for(EdgeVector::const_iterator k=incoming.begin(); k!=incoming.end(); k++) {
                    !!!                        newBlocked.push_back(NBConnection(*k, *k));
                                        }*/
                } else if (sblocked.getFrom()==removed) {
                    assert(sblocked.getTo()!=removed);
                    for (EdgeVector::const_iterator k=incoming.begin(); k!=incoming.end(); k++) {
                        newBlocked.push_back(NBConnection(*k, sblocked.getTo()));
                    }
                } else if (sblocked.getTo()==removed) {
                    assert(sblocked.getFrom()!=removed);
                    for (EdgeVector::const_iterator k=outgoing.begin(); k!=outgoing.end(); k++) {
                        newBlocked.push_back(NBConnection(sblocked.getFrom(), *k));
                    }
                } else {
                    newBlocked.push_back(NBConnection(sblocked.getFrom(), sblocked.getTo()));
                }
            }
            if (blockedChanged) {
                blockedConnectionsNew[blocker] = newBlocked;
                changed = true;
            }
            // if the blocked were kept
            else {
                if (blocker.getFrom()==removed&&blocker.getTo()==removed) {
                    changed = true;
                    /*                    for(EdgeVector::const_iterator k=incoming.begin(); k!=incoming.end(); k++) {
                    !!!                        blockedConnectionsNew[NBConnection(*k, *k)] = blocked;
                                        }*/
                } else if (blocker.getFrom()==removed) {
                    assert(blocker.getTo()!=removed);
                    changed = true;
                    for (EdgeVector::const_iterator k=incoming.begin(); k!=incoming.end(); k++) {
                        blockedConnectionsNew[NBConnection(*k, blocker.getTo())] = blocked;
                    }
                } else if (blocker.getTo()==removed) {
                    assert(blocker.getFrom()!=removed);
                    changed = true;
                    for (EdgeVector::const_iterator k=outgoing.begin(); k!=outgoing.end(); k++) {
                        blockedConnectionsNew[NBConnection(blocker.getFrom(), *k)] = blocked;
                    }
                } else {
                    blockedConnectionsNew[blocker] = blocked;
                }
            }
        }
        myBlockedConnections = blockedConnectionsNew;
    }
    // remap in traffic lights
    tc.remapRemoved(removed, incoming, outgoing);
}


LinkDirection
NBNode::getDirection(const NBEdge * const incoming, const NBEdge * const outgoing) const throw() {
    // ok, no connection at all -> dead end
    if (outgoing==0) {
        return LINKDIR_NODIR;
    }
    // turning direction
    if (incoming->isTurningDirectionAt(this, outgoing)) {
        return LINKDIR_TURN;
    }
    // get the angle between incoming/outgoing at the junction
    SUMOReal angle =
        NBHelpers::normRelAngle(incoming->getAngle(*this), outgoing->getAngle(*this));
    // ok, should be a straight connection
    if (abs((int) angle)+1<45) {
        return LINKDIR_STRAIGHT;
    }

    // check for left and right, first
    if (angle>0) {
        // check whether any other edge goes further to the right
        EdgeVector::const_iterator i =
            find(myAllEdges.begin(), myAllEdges.end(), outgoing);
        NBContHelper::nextCW(myAllEdges, i);
        while ((*i)!=incoming) {
            if ((*i)->getFromNode()==this) {
                return LINKDIR_PARTRIGHT;
            }
            NBContHelper::nextCW(myAllEdges, i);
        }
        return LINKDIR_RIGHT;
    }
    // check whether any other edge goes further to the left
    EdgeVector::const_iterator i =
        find(myAllEdges.begin(), myAllEdges.end(), outgoing);
    NBContHelper::nextCCW(myAllEdges, i);
    while ((*i)!=incoming) {
        if ((*i)->getFromNode()==this&&!incoming->isTurningDirectionAt(this, *i)) {
            return LINKDIR_PARTLEFT;
        }
        NBContHelper::nextCCW(myAllEdges, i);
    }
    return LINKDIR_LEFT;
}


std::string
NBNode::stateCode(const NBEdge *incoming, NBEdge *outgoing, int fromlane, bool mayDefinitelyPass) const throw() {
    if (outgoing==0) { // always off
        return toString(LINKSTATE_TL_OFF_NOSIGNAL);
    }
    if (myType==NODETYPE_RIGHT_BEFORE_LEFT) {
        return toString(LINKSTATE_EQUAL); // all the same
    }
    if ((!incoming->isInnerEdge()&&mustBrake(incoming, outgoing, fromlane)) && !mayDefinitelyPass) {
        return toString(LINKSTATE_MINOR); // minor road
    }
    // traffic lights are not regardedm here
    return toString(LINKSTATE_MAJOR);
}


bool
NBNode::checkIsRemovable() const {
    // check whether this node is included in a traffic light
    if (myTrafficLights.size()!=0) {
        return false;
    }
    EdgeVector::const_iterator i;
    // one in, one out -> just a geometry ...
    if (myOutgoingEdges.size()==1&&myIncomingEdges.size()==1) {
        // ... if types match ...
        if (!myIncomingEdges[0]->expandableBy(myOutgoingEdges[0])) {
            return false;
        }
        //
        return myIncomingEdges[0]->getFromNode()!=myOutgoingEdges[0]->getToNode();
    }
    // two in, two out -> may be something else
    if (myOutgoingEdges.size()==2&&myIncomingEdges.size()==2) {
        // check whether the origin nodes of the incoming edges differ
        std::set<NBNode*> origSet;
        for (i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
            origSet.insert((*i)->getFromNode());
        }
        if (origSet.size()<2) {
            return false;
        }
        // check whether this node is an intermediate node of
        //  a two-directional street
        for (i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
            // try to find the opposite direction
            NBNode *origin = (*i)->getFromNode();
            // find the back direction of the current edge
            EdgeVector::const_iterator j =
                find_if(myOutgoingEdges.begin(), myOutgoingEdges.end(),
                        NBContHelper::edge_with_destination_finder(origin));
            // check whether the back direction exists
            if (j!=myOutgoingEdges.end()) {
                // check whether the edge from the backdirection (must be
                //  the counter-clockwise one) may be joined with the current
                NBContHelper::nextCCW(myOutgoingEdges, j);
                // check whether the types allow joining
                if (!(*i)->expandableBy(*j)) {
                    return false;
                }
            } else {
                // ok, at least one outgoing edge is not an opposite
                //  of an incoming one
                return false;
            }
        }
        return true;
    }
    // ok, a real node
    return false;
}


std::vector<std::pair<NBEdge*, NBEdge*> >
NBNode::getEdgesToJoin() const {
    assert(checkIsRemovable());
    std::vector<std::pair<NBEdge*, NBEdge*> > ret;
    // one in, one out-case
    if (myOutgoingEdges.size()==1&&myIncomingEdges.size()==1) {
        ret.push_back(
            std::pair<NBEdge*, NBEdge*>(
                myIncomingEdges[0], myOutgoingEdges[0]));
        return ret;
    }
    // two in, two out-case
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        NBNode *origin = (*i)->getFromNode();
        EdgeVector::const_iterator j =
            find_if(myOutgoingEdges.begin(), myOutgoingEdges.end(),
                    NBContHelper::edge_with_destination_finder(origin));
        NBContHelper::nextCCW(myOutgoingEdges, j);
        ret.push_back(std::pair<NBEdge*, NBEdge*>(*i, *j));
    }
    return ret;
}


const PositionVector &
NBNode::getShape() const {
    return myPoly;
}

std::string
NBNode::getInternalLaneID(const NBEdge *from, unsigned int fromlane,
                          NBEdge *to, unsigned int tolane) const {
    unsigned int l = 0;
    for (EdgeVector::const_iterator i=myIncomingEdges.begin(); i!=myIncomingEdges.end(); i++) {
        unsigned int noLanesEdge = (*i)->getNoLanes();
        for (unsigned int j=0; j<noLanesEdge; j++) {
            std::vector<NBEdge::Connection> elv = (*i)->getConnectionsFromLane(j);
            for (std::vector<NBEdge::Connection>::iterator k=elv.begin(); k!=elv.end(); ++k) {
                if ((*k).toEdge==0) {
                    continue;
                }
                if ((from==*i)&&(j==fromlane)&&((*k).toEdge==to)&&((*k).toLane==tolane)) {
                    return ":" + myID + "_" + toString(l);
                }
                l++;
            }
        }
    }
    throw 1;
}


SUMOReal
NBNode::getMaxEdgeWidth() const {
    EdgeVector::const_iterator i=myAllEdges.begin();
    assert(i!=myAllEdges.end());
    SUMOReal ret = (*i)->width();
    ++i;
    for (; i!=myAllEdges.end(); i++) {
        ret = ret > (*i)->width()
              ? ret
              : (*i)->width();
    }
    return ret;
}


NBEdge *
NBNode::getConnectionTo(NBNode *n) const {
    for (EdgeVector::const_iterator i=myOutgoingEdges.begin(); i!=myOutgoingEdges.end(); i++) {
        if ((*i)->getToNode()==n) {
            return (*i);
        }
    }
    return 0;
}


bool
NBNode::isNearDistrict() const {
    if (isDistrict()) {
        return false;
    }
    EdgeVector edges;
    copy(getIncomingEdges().begin(), getIncomingEdges().end(),
         back_inserter(edges));
    copy(getOutgoingEdges().begin(), getOutgoingEdges().end(),
         back_inserter(edges));
    for (EdgeVector::const_iterator j=edges.begin(); j!=edges.end(); ++j) {
        NBEdge *t = *j;
        NBNode *other = 0;
        if (t->getToNode()==this) {
            other = t->getFromNode();
        } else {
            other = t->getToNode();
        }
        EdgeVector edges2;
        copy(other->getIncomingEdges().begin(), other->getIncomingEdges().end(), back_inserter(edges2));
        copy(other->getOutgoingEdges().begin(), other->getOutgoingEdges().end(), back_inserter(edges2));
        for (EdgeVector::const_iterator k=edges2.begin(); k!=edges2.end(); ++k) {
            if ((*k)->getFromNode()->isDistrict()||(*k)->getToNode()->isDistrict()) {
                return true;
            }
        }
    }
    return false;
}


bool
NBNode::isDistrict() const {
    return myType==NODETYPE_DISTRICT;
}

/****************************************************************************/

