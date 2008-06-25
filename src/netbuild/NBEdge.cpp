/****************************************************************************/
/// @file    NBEdge.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Methods for the representation of a single edge
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

#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include "NBEdgeCont.h"
#include "NBNode.h"
#include "NBNodeCont.h"
#include "NBContHelper.h"
#include "NBHelpers.h"
#include <cmath>
#include <iomanip>
#include "NBTypeCont.h"
#include <utils/geom/GeomHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StdDefs.h>
#include "NBEdge.h"
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// debug definitions
// ===========================================================================
#define DEBUG_OUT cout


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * NBEdge::ToEdgeConnectionsAdder-methods
 * ----------------------------------------------------------------------- */
void
NBEdge::ToEdgeConnectionsAdder::execute(SUMOReal lane, SUMOReal virtEdge) throw()
{
    // check
    assert(virtEdge>=0);
    assert(myTransitions.size()>(size_t) virtEdge);
    assert(lane>=0&&lane<10);
    // get the approached edge
    NBEdge *succEdge = myTransitions[(int) virtEdge];
    vector<size_t> lanes;

    // check whether the currently regarded, approached edge has already
    //  a connection starting at the edge which is currently being build
    map<NBEdge*, vector<size_t> >::iterator i=myConnections.find(succEdge);
    if (i!=myConnections.end()) {
        // if there were already lanes assigned, get them
        lanes = (*i).second;
    }

    // check whether the current lane was already used to connect the currently
    //  regarded approached edge
    vector<size_t>::iterator j=find(lanes.begin(), lanes.end(), (size_t) lane);
    if (j==lanes.end()) {
        // if not, add it to the list
        lanes.push_back((size_t) lane);
    }
    // set information about connecting lanes
    myConnections[succEdge] = lanes;
}



/* -------------------------------------------------------------------------
 * NBEdge::MainDirections-methods
 * ----------------------------------------------------------------------- */
NBEdge::MainDirections::MainDirections(const vector<NBEdge*> &outgoing,
                                       NBEdge *parent, NBNode *to)
{
    if (outgoing.size()==0) {
        return;
    }
    // check whether the right turn has a higher priority
    assert(outgoing.size()>0);
    if (outgoing[0]->getJunctionPriority(to)==1) {
        myDirs.push_back(MainDirections::DIR_RIGHTMOST);
    }
    // check whether the left turn has a higher priority
    if (outgoing[outgoing.size()-1]->getJunctionPriority(to)==1) {
        // ok, the left turn belongs to the higher priorised edges on the junction
        //  let's check, whether it has also a higher priority (lane number/speed)
        //  than the current
        vector<NBEdge*> tmp(outgoing);
        sort(tmp.begin(), tmp.end(), NBContHelper::edge_similar_direction_sorter(parent));
        if (outgoing[outgoing.size()-1]->getPriority()>tmp[0]->getPriority()) {
            myDirs.push_back(MainDirections::DIR_LEFTMOST);
        } else {
            if (outgoing[outgoing.size()-1]->getNoLanes()>tmp[0]->getNoLanes()) {
                myDirs.push_back(MainDirections::DIR_LEFTMOST);
            }
        }
    }
    // check whether the forward direction has a higher priority
    //  try to get the forward direction
    vector<NBEdge*> tmp(outgoing);
    sort(tmp.begin(), tmp.end(),
         NBContHelper::edge_similar_direction_sorter(parent));
    NBEdge *edge = *(tmp.begin());
    // check whether it has a higher priority and is going straight
    if (edge->getJunctionPriority(to)==1 && to->getMMLDirection(parent, edge)==MMLDIR_STRAIGHT) {
        myDirs.push_back(MainDirections::DIR_FORWARD);
    }
}


NBEdge::MainDirections::~MainDirections()
{}


bool
NBEdge::MainDirections::empty() const
{
    return myDirs.empty();
}


bool
NBEdge::MainDirections::includes(Direction d) const
{
    return find(myDirs.begin(), myDirs.end(), d)!=myDirs.end();
}



/* -------------------------------------------------------------------------
 * NBEdge-methods
 * ----------------------------------------------------------------------- */
NBEdge::NBEdge(const string &id, NBNode *from, NBNode *to,
               string type, SUMOReal speed, unsigned int nolanes,
               int priority, LaneSpreadFunction spread) throw(ProcessError) :
        myStep(INIT), myID(StringUtils::convertUmlaute(id)),
        myType(StringUtils::convertUmlaute(type)),
        myFrom(from), myTo(to), myAngle(0),
        myPriority(priority), mySpeed(speed),
        myTurnDestination(0),
        myFromJunctionPriority(-1), myToJunctionPriority(-1),
        myLaneSpreadFunction(spread),
        myLoadedLength(-1), myAmTurningWithAngle(0), myAmTurningOf(0),
        myAmInnerEdge(false)
{
	init(nolanes);
}


NBEdge::NBEdge(const string &id, NBNode *from, NBNode *to,
               string type, SUMOReal speed, unsigned int nolanes,
               int priority,
               Position2DVector geom, LaneSpreadFunction spread) throw(ProcessError) :
        myStep(INIT), myID(StringUtils::convertUmlaute(id)),
        myType(StringUtils::convertUmlaute(type)),
        myFrom(from), myTo(to), myAngle(0),
        myPriority(priority), mySpeed(speed),
        myTurnDestination(0),
        myFromJunctionPriority(-1), myToJunctionPriority(-1),
        myGeom(geom), myLaneSpreadFunction(spread),
        myLoadedLength(-1), myAmTurningWithAngle(0), myAmTurningOf(0),
        myAmInnerEdge(false)
{
	init(nolanes);
}


void
NBEdge::init(unsigned int noLanes) throw(ProcessError)
{
    if (noLanes==0) {
        throw ProcessError("Edge '" + myID + "' needs at least one lane.");
    }
    if (myFrom==0||myTo==0) {
        throw ProcessError("At least one of edge's '" + myID + "' nodes is not known.");
    }
    if (myFrom->getPosition().almostSame(myTo->getPosition())) {
        throw ProcessError("Edge '" + myID + "' starts at the same position it ends on.");
    }
    myGeom.push_back_noDoublePos(myTo->getPosition());
    myGeom.push_front_noDoublePos(myFrom->getPosition());
    myAngle = NBHelpers::angle(
                  myFrom->getPosition().x(), myFrom->getPosition().y(),
                  myTo->getPosition().x(), myTo->getPosition().y()
              );
    myFrom->addOutgoingEdge(this);
    myTo->addIncomingEdge(this);
    // prepare container
    myLength = GeomHelper::distance(myFrom->getPosition(), myTo->getPosition());
    assert(myGeom.size()>=2);
    for (size_t i=0; i<noLanes; i++) {
        Lane l;
        l.speed = mySpeed;
        myLanes.push_back(l);
    }
	try {
		computeLaneShapes();
	} catch (InvalidArgument &ia) {
		throw ProcessError(string(ia.what()) + " in edge " + myID + ".");
	}
}


NBEdge::~NBEdge() throw()
{}


// ----------- Edge geometry access and computation
void
NBEdge::setGeometry(const Position2DVector &s) throw()
{
    myGeom = s;
    computeLaneShapes();
}


void
NBEdge::computeEdgeShape() throw()
{
    size_t i;
    for (i=0; i<myLanes.size(); i++) {
        Position2DVector &shape = myLanes[i].shape;
        // get lane begin and end
        Line2D lb = Line2D(shape[0], shape[1]);
        Line2D le = Line2D(shape[-1], shape[-2]);
        lb.extrapolateBy(100.0);
        le.extrapolateBy(100.0);
        //
        Position2DVector old = shape;
        Position2D nb, ne;
        // lane begin
        if (myFrom->getShape().intersects(shape)) {
            // get the intersection position with the junction
            DoubleVector pbv = shape.intersectsAtLengths(myFrom->getShape());
            if (pbv.size()>0) {
                SUMOReal pb = VectorHelper<SUMOReal>::maxValue(pbv);
                if (pb>=0&&pb<=shape.length()) {
                    shape = shape.getSubpart(pb, shape.length());
                }
            }
        } else if (myFrom->getShape().intersects(lb.p1(), lb.p2())) {
            DoubleVector pbv = lb.intersectsAtLengths(myFrom->getShape());
            if (pbv.size()>0) {
                SUMOReal pb = VectorHelper<SUMOReal>::maxValue(pbv);
                if (pb>=0) {
                    shape.eraseAt(0);
                    shape.push_front_noDoublePos(lb.getPositionAtDistance(pb));
                }
            }
        }
        // lane end
        if (myTo->getShape().intersects(shape)) {
            // get the intersection position with the junction
            DoubleVector pev = shape.intersectsAtLengths(myTo->getShape());
            if (pev.size()>0) {
                SUMOReal pe = VectorHelper<SUMOReal>::minValue(pev);
                if (pe>=0&&pe<=shape.length()) {
                    shape = shape.getSubpart(0, pe);
                }
            }
        } else if (myTo->getShape().intersects(le.p1(), le.p2())) {
            DoubleVector pev = le.intersectsAtLengths(myTo->getShape());
            if (pev.size()>0) {
                SUMOReal pe = VectorHelper<SUMOReal>::maxValue(pev);
                if (pe>=0) {
                    shape.eraseAt(shape.size()-1);
                    shape.push_back_noDoublePos(le.getPositionAtDistance(pe));
                }
            }
        }
        if (((int) shape.length())==0) {
            shape = old;
        } else {
            Line2D lc(shape[0], shape[-1]);
            Line2D lo(old[0], old[-1]);
            if (135<MIN2(GeomHelper::getCCWAngleDiff(lc.atan2DegreeAngle(), lo.atan2DegreeAngle()), GeomHelper::getCWAngleDiff(lc.atan2DegreeAngle(), lo.atan2DegreeAngle()))) {
                shape = shape.reverse();
            }
        }
    }
    // recompute edge's length
    SUMOReal length = 0;
    for (i=0; i<myLanes.size(); i++) {
        assert(myLanes[i].shape.length()>0);
        length += myLanes[i].shape.length();
    }
    myLength = length / (SUMOReal) myLanes.size();
}


const Position2DVector &
NBEdge::getLaneShape(unsigned int i) const throw()
{
    return myLanes[i].shape;
}







int
NBEdge::getJunctionPriority(NBNode *node)
{
    if (node==myFrom) {
        return myFromJunctionPriority;
    } else {
        return myToJunctionPriority;
    }
}


void
NBEdge::setJunctionPriority(NBNode *node, int prio)
{
    if (node==myFrom) {
        myFromJunctionPriority = prio;
    } else {
        myToJunctionPriority = prio;
    }
}


EdgeLaneVector
NBEdge::getEdgeLanesFromLane(size_t lane) const
{
    EdgeLaneVector ret;
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).fromLane==lane) {
            ret.push_back((*i).getEdgeLane());
        }
    }
    return ret;
}


void
NBEdge::computeTurningDirections()
{
    myTurnDestination = 0;
    EdgeVector outgoing = myTo->getOutgoingEdges();
    for (EdgeVector::iterator i=outgoing.begin(); i!=outgoing.end(); i++) {
        NBEdge *outedge = *i;

        SUMOReal relAngle =
            NBHelpers::relAngle(getAngle(*myTo), outedge->getAngle(*myTo));
        // do not append the turnaround
        if (fabs(relAngle)>160) {
            setTurningDestination(outedge);
        }
    }
}


SUMOReal
NBEdge::getAngle(const NBNode &atNode) const
{
    if (&atNode==myFrom) {
        return myGeom.getBegLine().atan2DegreeAngle();
    } else {
        assert(&atNode==myTo);
        return myGeom.getEndLine().atan2DegreeAngle();
    }
}


void
NBEdge::setTurningDestination(NBEdge *e)
{
    SUMOReal cur = fabs(NBHelpers::relAngle(getAngle(), e->getAngle()));
    SUMOReal old =
        myTurnDestination==0
        ? 0
        : fabs(NBHelpers::relAngle(getAngle(), myTurnDestination->getAngle()));
    if (cur>old
            &&
            e->acceptBeingTurning(this)) {

        myTurnDestination = e;
    }
}


bool
NBEdge::acceptBeingTurning(NBEdge *e)
{
    if (e==myAmTurningOf) {
        return true;
    }
    SUMOReal angle = fabs(NBHelpers::relAngle(getAngle(), e->getAngle()));
    if (myAmTurningWithAngle>angle) {
        return false;
    }
    if (myAmTurningWithAngle==angle) {
        return false; // !!! ok, this happens only within a cell-network (backgrnd), we have to take a further look sometime
    }
    NBEdge *previous = myAmTurningOf;
    myAmTurningWithAngle = angle;
    myAmTurningOf = e;
    if (previous!=0) {
        previous->computeTurningDirections();
    }
    return true;
}


void
NBEdge::writeXMLStep1(OutputDevice &into)
{
    // write the edge's begin
    into << "   <edge id=\"" << myID <<
    "\" from=\"" << myFrom->getID() <<
    "\" to=\"" << myTo->getID() <<
    "\" priority=\"" << myPriority <<
    "\" type=\"" << myType <<
    "\" function=\"normal";
    if (myAmInnerEdge) {
        into << "\" inner=\"x";
    }
    into << "\">\n";
    // write the lanes
    into << "      <lanes>\n";
    for (size_t i=0; i<myLanes.size(); i++) {
        writeLane(into, myLanes[i], i);
    }
    into << "      </lanes>\n";
    // write the list of connected edges
    const vector<NBEdge*> *tmp = getConnectedSorted();
    vector<NBEdge*> sortedConnected = *tmp;
    if (getTurnDestination()!=0) {
        sortedConnected.push_back(getTurnDestination());
    }
    delete tmp;
    for (vector<NBEdge*>::iterator l=sortedConnected.begin(); l!=sortedConnected.end(); l++) {
        LaneVector lanes;
        for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
            if((*i).toEdge==*l && find(lanes.begin(), lanes.end(), (*i).fromLane)==lanes.end()) {
                lanes.push_back((*i).fromLane);
            }
        }
        if (lanes.size()!=0) {
            writeConnected(into, *l, lanes);
        }
    }
    // close the edge
    into << "   </edge>\n\n";
}


void
NBEdge::writeXMLStep2(OutputDevice &into, bool includeInternal)
{
    for (size_t i=0; i<myLanes.size(); i++) {
        writeSucceeding(into, i, includeInternal);
    }
}


void
NBEdge::writeLane(OutputDevice &into, NBEdge::Lane &lane, unsigned int index) const
{
    // output the lane's attributes
    into << "         <lane id=\"" << myID << '_' << index << "\"";
    // the first lane of an edge will be the depart lane
    if (index==0) {
        into << " depart=\"1\"";
    } else {
        into << " depart=\"0\"";
    }
    // write the list of allowed/disallowed vehicle classes
    {
        into << " vclasses=\"";
        vector<SUMOVehicleClass>::const_iterator i;
        bool hadOne = false;
        for (i=lane.allowed.begin(); i!=lane.allowed.end(); ++i) {
            if (hadOne) {
                into << ';';
            }
            into << getVehicleClassName(*i);
            hadOne = true;
        }
        for (i=lane.notAllowed.begin(); i!=lane.notAllowed.end(); ++i) {
            if (hadOne) {
                into << ';';
            }
            into << '-' << getVehicleClassName(*i);
            hadOne = true;
        }
        into << '\"';
    }
    // some further information
    if (lane.speed==0) {
        WRITE_WARNING("Lane #" + toString(index) + " of edge '" + myID + "' has a maximum velocity of 0.");
    } else if (lane.speed<0) {
        throw ProcessError("Negative velocity (" + toString(lane.speed) + " on edge '" + myID + "' lane#" + toString(index) + ".");

    }
    SUMOReal length = myLength;
    if (myLoadedLength>0) {
        length = myLoadedLength;
    }
    into << " maxspeed=\"" << lane.speed << "\" length=\"" << length << "\">";
    // the lane's shape
    into << lane.shape;
    // close
    into << "</lane>\n";
}


void
NBEdge::setLaneSpeed(unsigned int lane, SUMOReal speed)
{
    myLanes[lane].speed = speed;
}


SUMOReal
NBEdge::getLaneSpeed(unsigned int lane) const
{
    return myLanes[lane].speed;
}


void
NBEdge::computeLaneShapes() throw(InvalidArgument)
{
    // vissim needs this
    if (myFrom==myTo) {
        return;
    }
    // build the shape of each lane
    for (size_t i=0; i<myLanes.size(); i++) {
        myLanes[i].shape = computeLaneShape(i);
    }
}


Position2DVector
NBEdge::computeLaneShape(size_t lane) throw(InvalidArgument)
{
    Position2DVector shape;

    for (size_t i=0; i<myGeom.size(); i++) {
        if (/*i==myGeom.size()-2||*/i==0) {
            Position2D from = myGeom[i];
            Position2D to = myGeom[i+1];
            pair<SUMOReal, SUMOReal> offsets =
                laneOffset(from, to, SUMO_const_laneWidthAndOffset, myLanes.size()-1-lane);
            shape.push_back_noDoublePos(//.push_back(
                // (methode umbenennen; was heisst hier "-")
                Position2D(from.x()-offsets.first, from.y()-offsets.second));
        } else if (i==myGeom.size()-1) {
            Position2D from = myGeom[i-1];
            Position2D to = myGeom[i];
            pair<SUMOReal, SUMOReal> offsets =
                laneOffset(from, to, SUMO_const_laneWidthAndOffset, myLanes.size()-1-lane);
            shape.push_back_noDoublePos(//.push_back(
                // (methode umbenennen; was heisst hier "-")
                Position2D(to.x()-offsets.first, to.y()-offsets.second));
        } else {
            Position2D from = myGeom[i-1];
            Position2D me = myGeom[i];
            Position2D to = myGeom[i+1];
            pair<SUMOReal, SUMOReal> offsets =
                laneOffset(from, me, SUMO_const_laneWidthAndOffset, myLanes.size()-1-lane);
            pair<SUMOReal, SUMOReal> offsets2 =
                laneOffset(me, to, SUMO_const_laneWidthAndOffset, myLanes.size()-1-lane);
            Line2D l1(
                Position2D(from.x()-offsets.first, from.y()-offsets.second),
                Position2D(me.x()-offsets.first, me.y()-offsets.second));
            l1.extrapolateBy(100);
            Line2D l2(
                Position2D(me.x()-offsets2.first, me.y()-offsets2.second),
                Position2D(to.x()-offsets2.first, to.y()-offsets2.second));
            l2.extrapolateBy(100);
            if (l1.intersects(l2)) {
                shape.push_back_noDoublePos(l1.intersectsAt(l2));
            } else {
                // !!! should never happen
                //   throw 1;
            }
        }
    }
    return shape;
}


pair<SUMOReal, SUMOReal>
NBEdge::laneOffset(const Position2D &from, const Position2D &to,
                   SUMOReal lanewidth, size_t lane) throw(InvalidArgument)
{
    SUMOReal x1 = from.x();
    SUMOReal y1 = from.y();
    SUMOReal x2 = to.x();
    SUMOReal y2 = to.y();
    SUMOReal length = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    pair<SUMOReal, SUMOReal> offsets =
        GeomHelper::getNormal90D_CW(x1, y1, x2, y2, length, lanewidth);
    SUMOReal xoff = offsets.first / (SUMOReal) 2.0;
    SUMOReal yoff = offsets.second / (SUMOReal) 2.0;
    if (myLaneSpreadFunction==LANESPREAD_RIGHT) {
        xoff += (offsets.first * (SUMOReal) lane);
        yoff += (offsets.second * (SUMOReal) lane);
    } else {
        xoff += (offsets.first * (SUMOReal) lane) - (offsets.first * (SUMOReal) myLanes.size() / (SUMOReal) 2.0);
        yoff += (offsets.second * (SUMOReal) lane) - (offsets.second * (SUMOReal) myLanes.size() / (SUMOReal) 2.0);
    }
    return pair<SUMOReal, SUMOReal>(xoff, yoff);
}


void
NBEdge::writeConnected(OutputDevice &into, NBEdge *edge, LaneVector &lanes)
{
    if (edge==0) {
        return;
    }
    into << "      <cedge id=\"" << edge->getID() << "\">";
    size_t noApproachers = lanes.size();
    for (size_t i=0; i<noApproachers; i++) {
        assert(i<lanes.size());
        into << myID << '_' << lanes[i];
        if (i<noApproachers-1) {
            into << ' ';
        }
    }
    into << "</cedge>\n";
}


void
NBEdge::writeSucceeding(OutputDevice &into, size_t lane,
                        bool includeInternal)
{
    into << "   <succ edge=\"" << myID << "\" lane=\"" << myID << "_" << lane << "\" junction=\"" << myTo->getID() << "\">\n";
    // output list of connected lanes
    size_t count = 0;
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
    //for (size_t j=0; j<noApproached; j++) {
        if((*i).fromLane==lane) {
            writeSingleSucceeding(into, *i, includeInternal);
            ++count;
        }
    }
    // the lane may be unconnented; output information about being invalid
    if (count==0) {
        into << "      <succlane lane=\"SUMO_NO_DESTINATION\" yield=\"1\"/>\n";
    }
    into << "   </succ>\n\n";
}


void
NBEdge::writeSingleSucceeding(OutputDevice &into, const NBEdge::Connection &c, //size_t fromlane, size_t destidx,
                              bool includeInternal)
{
    // check whether the connected lane is invalid
    //  (should not happen; this is an artefact left from previous versions)
    if (c.toEdge==0) {
        into << "      <succlane lane=\"SUMO_NO_DESTINATION\" yield=\"1\" " << "dir=\"s\" state=\"O\"/>\n"; // !!! check dummy values
        return;
    }
    // write the id
    into << "      <succlane lane=\"" << c.toEdge->getID() << '_' << c.toLane << '\"'; // !!! classe LaneEdge mit getLaneID
    if (includeInternal) {
        into << " via=\"" << myTo->getInternalLaneID(this, c.fromLane, c.toEdge, c.toLane) << "_0\"";
    }
    // set information about the controlling tl if any
    if (c.tlID!="") {
        into << " tl=\"" << c.tlID << "\"";
        into << " linkno=\"" << c.tlLinkNo << "\"";
    }
    // write information whether the connection yields
    if (myAmInnerEdge||!myTo->mustBrake(this, c.toEdge, c.toLane)) {
        into << " yield=\"0\"";
    } else {
        into << " yield=\"1\"";
    }
    // write the direction information
    NBMMLDirection dir = myTo->getMMLDirection(this, c.toEdge);
    into << " dir=\"";
    switch (dir) {
    case MMLDIR_STRAIGHT:
        into << "s";
        break;
    case MMLDIR_LEFT:
        into << "l";
        break;
    case MMLDIR_RIGHT:
        into << "r";
        break;
    case MMLDIR_TURN:
        into << "t";
        break;
    case MMLDIR_PARTLEFT:
        into << "L";
        break;
    case MMLDIR_PARTRIGHT:
        into << "R";
        break;
    default:
        // should not happen
        assert(false);
        break;
    }
    into << "\" ";
    // write the state information
    if (c.tlID!="") {
        into << "state=\"t";
    } else {
        into << "state=\"" << myTo->stateCode(this, c.toEdge, c.toLane);
    }
    if (myTo->getCrossingPosition(this, c.fromLane, c.toEdge, c.toLane).first>=0) {
        into << "\" int_end=\"x";
    }

    // close
    into << "\"/>\n";
}


bool
NBEdge::hasRestrictions() const
{
    for (vector<Lane>::const_iterator i=myLanes.begin(); i!=myLanes.end(); ++i) {
        if ((*i).allowed.size()!=0 || (*i).notAllowed.size()!=0) {
            return true;
        }
    }
    return false;
}


void
NBEdge::writeLanesPlain(OutputDevice &into)
{
    for (size_t i=0; i<myLanes.size(); ++i) {
        into << "      <lane id=\"" << i << "\"";
        const Lane &lane = myLanes[i];
        // write allowed lanes
        if (lane.allowed.size()!=0) {
            bool hadOne = false;
            into << " allow=\"";
            vector<SUMOVehicleClass>::const_iterator i;
            for (i=lane.allowed.begin(); i!=lane.allowed.end(); ++i) {
                if (hadOne) {
                    into << ';';
                }
                into << getVehicleClassName(*i);
                hadOne = true;
            }
            into << "\"";
        }
        if (lane.notAllowed.size()!=0) {
            bool hadOne = false;
            into << " disallow=\"";
            vector<SUMOVehicleClass>::const_iterator i;
            for (i=lane.notAllowed.begin(); i!=lane.notAllowed.end(); ++i) {
                if (hadOne) {
                    into << ';';
                }
                into << getVehicleClassName(*i);
                hadOne = true;
            }
            into << "\"";
        }
        into << "/>\n";
    }
}


bool
NBEdge::addEdge2EdgeConnection(NBEdge *dest)
{
    if (myStep==INIT_REJECT_CONNECTIONS) {
        return true;
    }
    // check whether the node was merged and now a connection between
    //  not matching edges is tried to be added
    //  This happens f.e. within the ptv VISSIM-example "Beijing"
    if (myTo!=dest->myFrom) {
        return false;
    }
    if (find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(dest))==myConnections.end()) {
        myConnections.push_back(Connection(-1, dest, -1));
    }
    if (myStep<EDGE2EDGES) {
        myStep = EDGE2EDGES;
    }
    return true;
}


bool
NBEdge::addLane2LaneConnection(size_t from, NBEdge *dest,
                               size_t toLane, bool markAs2Recheck,
                               bool mayUseSameDestination)
{
    if (myStep==INIT_REJECT_CONNECTIONS) {
        return true;
    }
    // check whether the node was merged and now a connection between
    //  not matching edges is tried to be added
    //  This happens f.e. within the ptv VISSIM-example "Beijing"
    if (myTo!=dest->myFrom) {
        return false;
    }
    if(!addEdge2EdgeConnection(dest)) {
        return false;
    }
    setConnection(from, dest, toLane, markAs2Recheck, mayUseSameDestination);
    return true;
}


bool
NBEdge::addLane2LaneConnections(size_t fromLane,
                                NBEdge *dest, size_t toLane,
                                size_t no,
                                bool markAs2Recheck,
                                bool invalidatePrevious)
{
    if (invalidatePrevious) {
        invalidateConnections(true);
    }
    bool ok = true;
    for (size_t i=0; i<no&&ok; i++) {
        ok &= addLane2LaneConnection(fromLane+i, dest, toLane+i, markAs2Recheck);
    }
    return ok;
}


bool
NBEdge::computeEdge2Edges()
{
    // return if this relationship has been build in previous steps or
    //  during the import
    if (myStep>=EDGE2EDGES) {
        return true;
    }
    if (myConnections.size()==0) {
        const EdgeVector &o = myTo->getOutgoingEdges();
        for(EdgeVector::const_iterator i=o.begin(); i!=o.end(); ++i) {
            myConnections.push_back(Connection(-1, *i, -1));
        }
    }
    myStep = EDGE2EDGES;
    return true;
}


bool
NBEdge::computeLanes2Edges()
{
    // return if this relationship has been build in previous steps or
    //  during the import
    if (myStep>=LANES2EDGES) {
        return true;
    }
    assert(myStep==EDGE2EDGES);
    // get list of possible outgoing edges sorted by direction clockwise
    //  the edge in the backward direction (turnaround) is not in the list
    const vector<NBEdge*> *edges = getConnectedSorted();
    // divide the lanes on reachable edges
    divideOnEdges(edges);
    delete edges;
    myStep = LANES2EDGES;
    return true;
}


bool
NBEdge::recheckLanes()
{
    vector<unsigned int> connNumbersPerLane(myLanes.size(), 0);
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ) {
        if((*i).toEdge==0||(*i).fromLane<0||(*i).toLane<0) {
            i = myConnections.erase(i);
        } else {
            if((*i).fromLane>=0) {
                ++connNumbersPerLane[(*i).fromLane];
            }
            ++i;
        }
    }
    if (myStep!=LANES2LANES) {
        // check #1:
        // If there is a lane with no connections and any neighbour lane has
        //  more than one connections, try to move one of them.
        // This check is only done for edges which connections were assigned
        //  using the standard algorithm.
        for (size_t i=0; i<myLanes.size(); i++) {
            if (connNumbersPerLane[i]==0) {
                if (i>0&&connNumbersPerLane[i-1]>1) {
                    moveConnectionToLeft(i-1);
                } else if (i<myLanes.size()-1&&connNumbersPerLane[i+1]>1) {
                    moveConnectionToRight(i+1);
                }
            }
        }
    }
    // check:
    // Go through all lanes and add an empty connection if no connection
    //  is yet set.
    // This check must be done for all lanes to assert that there is at
    //  least a dead end information (needed later for building the
    //  node request
    for (size_t i=0; i<myLanes.size(); i++) {
        connNumbersPerLane[i] = 0;
    }
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ) {
        if((*i).toEdge==0||(*i).fromLane<0||(*i).toLane<0) {
            i = myConnections.erase(i);
        } else {
            if((*i).fromLane>=0) {
                ++connNumbersPerLane[(*i).fromLane];
            }
            ++i;
        }
    }
    for (size_t i=0; i<myLanes.size(); i++) {
        if (connNumbersPerLane[i]==0) {
            setConnection(i, 0, 0, false);
        }
    }
    return true;
}


void
NBEdge::moveConnectionToLeft(size_t lane)
{
    size_t index = 0;
    for(size_t i=0; i<myConnections.size(); ++i) {
        if(myConnections[i].fromLane==lane) {
            index = i;
        }
    }
    vector<Connection>::iterator i = myConnections.begin() + index;
    Connection c = *i;
    myConnections.erase(i);
    setConnection(lane+1, c.toEdge, c.toLane, false);
}


void
NBEdge::moveConnectionToRight(size_t lane)
{
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).fromLane==lane) {
            Connection c = *i;
            i = myConnections.erase(i);
            setConnection(lane-1, c.toEdge, c.toLane, false);
            return;
        }
    }
}


vector<int>
NBEdge::getConnectionLanes(NBEdge *currentOutgoing) const
{
    vector<int> ret;
    if (currentOutgoing!=myTurnDestination) {
        for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
            if((*i).toEdge==currentOutgoing) {
                ret.push_back((*i).fromLane); 
            }
        }
    }
    return ret;
}


const vector<NBEdge*> *
NBEdge::getConnectedSorted()
{
    // check whether connections exist and if not, use edges from the node
    EdgeVector outgoing;
    if (myConnections.size()==0) {
        outgoing = myTo->getOutgoingEdges();
    } else {
        for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
            if(find(outgoing.begin(), outgoing.end(), (*i).toEdge)==outgoing.end()) {
                outgoing.push_back((*i).toEdge);
            }
        }
    }
    // allocate the sorted container
    size_t size = outgoing.size();
    vector<NBEdge*> *edges = new vector<NBEdge*>();
    edges->reserve(size);
    for (EdgeVector::const_iterator i=outgoing.begin(); i!=outgoing.end(); i++) {
        NBEdge *outedge = *i;
        if (outedge!=0&&outedge!=myTurnDestination) {
            edges->push_back(outedge);
        }
    }
    sort(edges->begin(), edges->end(),
         NBContHelper::relative_edge_sorter(this, myTo));
    return edges;
}


void
NBEdge::divideOnEdges(const vector<NBEdge*> *outgoing)
{
    if (outgoing->size()==0) {
        // we have to do this, because the turnaround may have been added before
        myConnections.clear(); 
        return;
    }
    // precompute priorities; needed as some kind of assumptions for
    //  priorities of directions (see preparePriorities)
    vector<size_t> *priorities = preparePriorities(outgoing);

    // compute the sum of priorities (needed for normalisation)
    size_t prioSum = computePrioritySum(priorities);
    // compute the resulting number of lanes that should be used to
    //  reach the following edge
    size_t size = outgoing->size();
    vector<SUMOReal> resultingLanes;
    resultingLanes.reserve(size);
    SUMOReal sumResulting = 0; // the sum of resulting lanes
    SUMOReal minResulting = 10000; // the least number of lanes to reach an edge
    size_t i;
    for (i=0; i<size; i++) {
        // res will be the number of lanes which are meant to reach the
        //  current outgoing edge
        SUMOReal res =
            (SUMOReal)(*priorities)[i] *
            (SUMOReal) myLanes.size() / (SUMOReal) prioSum;
        // do not let this number be greater than the number of available lanes
        if (res>myLanes.size()) {
            res = (SUMOReal) myLanes.size();
        }
        // add it to the list
        resultingLanes.push_back(res);
        sumResulting += res;
        if (minResulting>res) {
            minResulting = res;
        }
    }
    // compute the number of virtual edges
    //  a virtual edge is used as a replacement for a real edge from now on
    //  it shall ollow to divide the existing lanes on this structure without
    //  regarding the structure of outgoing edges
    sumResulting += minResulting / (SUMOReal) 2.;
    size_t noVirtual = (size_t)(sumResulting / minResulting);
    // compute the transition from virtual to real edges
    vector<NBEdge*> transition;
    transition.reserve(size);
    for (i=0; i<size; i++) {
        // tmpNo will be the number of connections from this edge
        //  to the next edge
        assert(i<resultingLanes.size());
        SUMOReal tmpNo = (SUMOReal) resultingLanes[i] / (SUMOReal) minResulting;
        for (SUMOReal j=0; j<tmpNo; j++) {
            assert(outgoing->size()>i);
            transition.push_back((*outgoing)[i]);
        }
    }

    // assign lanes to edges
    //  (conversion from virtual to real edges is done)
    ToEdgeConnectionsAdder adder(/*&myToEdges, */transition);
    Bresenham::compute(&adder, (SUMOReal) myLanes.size(), (SUMOReal) noVirtual);
    const map<NBEdge*, vector<size_t> > &l2eConns = adder.getBuiltConnections();
    myConnections.clear();
    for(map<NBEdge*, vector<size_t> >::const_iterator i=l2eConns.begin(); i!=l2eConns.end(); ++i) {
        const vector<size_t> lanes = (*i).second;
        for(vector<size_t>::const_iterator j=lanes.begin(); j!=lanes.end(); ++j) {
            myConnections.push_back(Connection(*j, (*i).first, -1));
        }
    }
    delete priorities;
}


vector<size_t> *
NBEdge::preparePriorities(const vector<NBEdge*> *outgoing)
{
    // copy the priorities first
    vector<size_t> *priorities = new vector<size_t>();
    if (outgoing->size()==0) {
        return priorities;
    }
    priorities->reserve(outgoing->size());
    vector<NBEdge*>::const_iterator i;
    for (i=outgoing->begin(); i!=outgoing->end(); i++) {
        int prio = (*i)->getJunctionPriority(myTo);
        assert((prio+1)*2>0);
        prio = (prio+1) * 2;
        priorities->push_back(prio);
    }
    // when the right turning direction has not a higher priority, divide
    //  the importance by 2 due to the possibility to leave the junction
    //  faster from this lane
    MainDirections mainDirections(*outgoing, this, myTo);
    vector<NBEdge*> tmp(*outgoing);
    sort(tmp.begin(), tmp.end(), NBContHelper::edge_similar_direction_sorter(this));
    i=find(outgoing->begin(), outgoing->end(), *(tmp.begin()));
    size_t dist = distance(outgoing->begin(), i);
    if (dist!=0&&!mainDirections.includes(MainDirections::DIR_RIGHTMOST)) {
        assert(priorities->size()>0);
        (*priorities)[0] = (*priorities)[0] / 2;
    }
    // HEURISTIC:
    // when no higher priority exists, let the forward direction be
    //  the main direction
    if (mainDirections.empty()) {
        assert(dist<priorities->size());
        (*priorities)[dist] = (*priorities)[dist] * 2;
    }
    if (mainDirections.includes(MainDirections::DIR_FORWARD)&&myLanes.size()>2) {
        (*priorities)[dist] = (*priorities)[dist] * 2;
    }
    // return
    return priorities;
}


size_t
NBEdge::computePrioritySum(vector<size_t> *priorities)
{
    size_t sum = 0;
    for (vector<size_t>::iterator i=priorities->begin(); i!=priorities->end(); i++) {
        sum += (*i);
    }
    return sum;
}


void
NBEdge::appendTurnaround()
{
    if (myTurnDestination!=0) {
        setConnection(myLanes.size()-1, myTurnDestination, myTurnDestination->getNoLanes()-1, false);
    }
}


void
NBEdge::sortOutgoingLanesConnections()
{
    sort(myConnections.begin(), myConnections.end(), connections_relative_edgelane_sorter(this, myTo));
}



void
NBEdge::setConnection(size_t src_lane, NBEdge *dest_edge,
                      size_t dest_lane, bool markAs2Recheck,
                      bool mayUseSameDestination)
{
    if (myStep==INIT_REJECT_CONNECTIONS) {
        return;
    }
    assert(dest_lane<=10);
    assert(src_lane<=10);
    // some kind of a misbehaviour which may occure when the junction's outgoing
    //  edge priorities were not properly computed, what may happen due to
    //  an incomplete or not proper input
    // what happens is that under some circumstances a single lane may set to
    //  be approached more than once by the one of our lanes.
    //  This must not be!
    // we test whether it is the case and do nothing if so - the connection
    //  will be refused
    //
    if (!mayUseSameDestination) {
        if(dest_edge!=0&&find_if(myConnections.begin(), myConnections.end(), connections_toedgelane_finder(dest_edge, dest_lane))!=myConnections.end()) {
            return;
        }
    }
    if(find_if(myConnections.begin(), myConnections.end(), connections_finder(src_lane, dest_edge, dest_lane))!=myConnections.end()) {
        return;
    }
    if (myLanes.size()<=src_lane) {
        MsgHandler::getErrorInstance()->inform("Could not set connection from '" + myID + "_" + toString(src_lane) + "' to '" + dest_edge->getID() + "_" + toString(dest_lane) + "'.");
        return;
    }
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ) {
        if((*i).toEdge==dest_edge && ((*i).fromLane==-1 || (*i).toLane==-1)) {
            i = myConnections.erase(i);
        } else {
            ++i;
        }
    }
    myConnections.push_back(Connection(src_lane, dest_edge, dest_lane));

    // check whether we have to take another look at it later
    if (markAs2Recheck) {
        // yes, the connection was set using an algorithm which requires a recheck
        myStep = LANES2LANES_RECHECK;
    } else {
        // ok, let's only not recheck it if we did no add something that has to be recheked
        if (myStep!=LANES2LANES_RECHECK) {
            myStep = LANES2LANES;
        }
    }
}


bool
NBEdge::isTurningDirectionAt(const NBNode *n, NBEdge *edge) const
{
    // maybe it was already set as the turning direction
    if (edge == myTurnDestination) {
        return true;
    } else if (myTurnDestination!=0) {
        // otherwise - it's not if a turning direction exists
        return false;
    }
    // if the same nodes are connected
    if (myFrom==edge->myTo &&myTo==edge->myFrom) {
        return true;
    }
    // we have to checke whether the connection between the nodes is
    //  geometrically similar
    SUMOReal thisFromAngle2 = getAngle(*n);
    SUMOReal otherToAngle2 = edge->getAngle(*n);
    if (thisFromAngle2<otherToAngle2) {
        swap(thisFromAngle2, otherToAngle2);
    }
    if (thisFromAngle2-otherToAngle2>170&&thisFromAngle2-otherToAngle2<190) {
        return true;
    }
    return false;
}



bool
NBEdge::isAlmostSameDirectionAt(const NBNode *n, NBEdge *edge) const
{
    // if the same nodes are connected
    if (myTo==edge->myTo && myFrom==edge->myFrom) {
        return true;
    }
    // we have to checke whether the connection between the nodes is
    //  geometrically similar
    SUMOReal thisFromAngle2 = getAngle(*n);
    SUMOReal otherToAngle2 = edge->getAngle(*n);
    if (thisFromAngle2<otherToAngle2) {
        swap(thisFromAngle2, otherToAngle2);
    }
    if (thisFromAngle2-otherToAngle2<20&&thisFromAngle2-otherToAngle2>-20) {
        return true;
    }
    return false;
}



NBNode *
NBEdge::tryGetNodeAtPosition(SUMOReal pos, SUMOReal tolerance) const
{
    // return the from-node when the position is at the begin of the edge
    if (pos<tolerance) {
        return myFrom;
    }
    // return the to-node when the position is at the end of the edge
    if (pos>myLength-tolerance) {
        return myTo;
    }
    return 0;
}


void
NBEdge::replaceInConnections(NBEdge *which, NBEdge *by, size_t laneOff)
{
    // replace in "_connectedEdges"
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).toEdge==which) {
            (*i).toEdge = by;
        }
    }
    // check whether it was the turn destination
    if (myTurnDestination==which) {
        myTurnDestination = by;
    }
}


void
NBEdge::moveOutgoingConnectionsFrom(NBEdge *e, size_t laneOff,
                                    bool markAs2Recheck)
{
    size_t lanes = e->getNoLanes();
    for (size_t i=0; i<lanes; i++) {
        EdgeLaneVector elv = e->getEdgeLanesFromLane(i);
        for (EdgeLaneVector::const_iterator j=elv.begin(); j!=elv.end(); j++) {
            EdgeLane el = (*j);
            assert(el.tlID=="");
            bool ok = addLane2LaneConnection(i+laneOff, el.edge, el.lane, markAs2Recheck);
            assert(ok);
        }
    }
}


bool
NBEdge::isConnectedTo(NBEdge *e)
{
    return
        find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(e))
        !=
        myConnections.end();

}


vector<NBEdge*> 
NBEdge::getConnectedEdges() const throw()
{
    vector<NBEdge*> ret;
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if(find(ret.begin(), ret.end(), (*i).toEdge)==ret.end()) {
            ret.push_back((*i).toEdge);
        }
    }
    return ret;
}


void
NBEdge::remapConnections(const EdgeVector &incoming)
{
    vector<NBEdge*> connected = getConnectedEdges();
    for (EdgeVector::const_iterator i=incoming.begin(); i!=incoming.end(); i++) {
        NBEdge *inc = *i;
        // We have to do this
        inc->myStep = EDGE2EDGES;
        // add all connections
        for (EdgeVector::iterator j=connected.begin(); j!=connected.end(); j++) {
            inc->addEdge2EdgeConnection(*j);
        }
        inc->removeFromConnections(this);
    }
}


void
NBEdge::removeFromConnections(NBEdge *which, int lane)
{
    // remove from "myConnections"
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ) {
        Connection &c = *i;
        if(c.toEdge==which && (lane<0 || c.fromLane==lane)) { // !!! chek fromlane/toLane
            i = myConnections.erase(i);
        } else {
            ++i;
        }
    }
    // check whether it was the turn destination
    if (myTurnDestination==which&&lane<0) {
        myTurnDestination = 0;
    }
}


void
NBEdge::invalidateConnections(bool reallowSetting)
{
    myTurnDestination = 0;
    myConnections.clear();
    if (reallowSetting) {
        myStep = INIT;
    } else {
        myStep = INIT_REJECT_CONNECTIONS;
    }
}


bool
NBEdge::lanesWereAssigned() const
{
    return myStep==LANES2LANES;
}


SUMOReal
NBEdge::getMaxLaneOffset()
{
    return (SUMOReal) SUMO_const_laneWidthAndOffset * myLanes.size();
}


Position2D
NBEdge::getMinLaneOffsetPositionAt(NBNode *node, SUMOReal width) const
{
    const Position2DVector &shape0 = myLanes[0].shape;
    const Position2DVector &shapel = myLanes[myLanes.size()-1].shape;
    width = width < shape0.length()/(SUMOReal) 2.0
            ? width
            : shape0.length()/(SUMOReal) 2.0;
    if (node==myFrom) {
        Position2D pos =  shapel.positionAtLengthPosition(width);
        GeomHelper::transfer_to_side(pos, shapel[0], shapel[-1], SUMO_const_halfLaneAndOffset);
        return pos;
    } else {
        Position2D pos = shape0.positionAtLengthPosition(shape0.length() - width);
        GeomHelper::transfer_to_side(pos, shape0[-1], shape0[0], SUMO_const_halfLaneAndOffset);
        return pos;
    }
}


Position2D
NBEdge::getMaxLaneOffsetPositionAt(NBNode *node, SUMOReal width) const
{
    const Position2DVector &shape0 = myLanes[0].shape;
    const Position2DVector &shapel = myLanes[myLanes.size()-1].shape;
    width = width < shape0.length()/(SUMOReal) 2.0
            ? width
            : shape0.length()/(SUMOReal) 2.0;
    if (node==myFrom) {
        Position2D pos = shape0.positionAtLengthPosition(width);
        GeomHelper::transfer_to_side(pos, shape0[0], shape0[-1], -SUMO_const_halfLaneAndOffset);
        return pos;
    } else {
        Position2D pos = shapel.positionAtLengthPosition(shapel.length() - width);
        GeomHelper::transfer_to_side(pos, shapel[-1], shapel[0], -SUMO_const_halfLaneAndOffset);
        return pos;
    }
}


bool
NBEdge::setControllingTLInformation(int fromLane, NBEdge *toEdge, int toLane,
                                    const string &tlID, size_t tlPos)
{
    // check whether the connection was not set as not to be controled previously
    TLSDisabledConnection tpl;
    tpl.fromLane = fromLane;
    tpl.to = toEdge;
    tpl.toLane = toLane;
    vector<TLSDisabledConnection>::iterator i = find_if(myTLSDisabledConnections.begin(), myTLSDisabledConnections.end(), tls_disable_finder(tpl));
    if (i!=myTLSDisabledConnections.end()) {
        return false;
    }

    assert(fromLane<0||fromLane<(int) myLanes.size());
    // try to use information about the connections if given
    if (fromLane>=0&&toLane>=0) {
        // find the specified connection
        vector<Connection>::iterator i =
            find_if(myConnections.begin(), myConnections.end(), connections_finder(fromLane, toEdge, toLane));
        // ok, we have to test this as on the removal of dummy edges some connections
        //  will be reassigned
        if (i!=myConnections.end()) {
            // get the connection
            Connection &connection = *i;
            // set the information about the tl
            connection.tlID = tlID;
            connection.tlLinkNo = tlPos;
            return true;
        }
    }
    // if the original connection was not found, set the information for all
    //  connections
    size_t no = 0;
    bool hadError = false;
    for(vector<Connection>::iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).toEdge!=toEdge) {
            continue;
        }
        if(fromLane>=0&&fromLane!=(*i).fromLane) {
            continue;
        }
        if(toLane>=0&&toLane!=(*i).toLane) {
            continue;
        }
        if ((*i).tlID=="") {
            (*i).tlID = tlID;
            (*i).tlLinkNo = tlPos;
            no++;
        } else {
            if ((*i).tlID!=tlID&&(*i).tlLinkNo==tlPos) {
                WRITE_WARNING("The lane " + toString<int>((*i).fromLane)+ " on edge " + getID()+ " already had a traffic light signal.");
                hadError = true;
            }
        }
    }
    if (hadError&&no==0) {
        WRITE_WARNING("Could not set any signal of the traffic light '" + tlID + "' (unknown group)");
    }
    return true;
}


void
NBEdge::disableConnection4TLS(int fromLane, NBEdge *toEdge, int toLane)
{
    TLSDisabledConnection c;
    c.fromLane = fromLane;
    c.to = toEdge;
    c.toLane = toLane;
    myTLSDisabledConnections.push_back(c);
}


void
NBEdge::normalisePosition()
{
    myGeom.resetBy(GeoConvHelper::getOffset());
    for (size_t i=0; i<myLanes.size(); i++) {
        myLanes[i].shape.resetBy(GeoConvHelper::getOffset());
    }
}


void
NBEdge::reshiftPosition(SUMOReal xoff, SUMOReal yoff, SUMOReal rot)
{
    myGeom.reshiftRotate(xoff, yoff, rot);
    for (size_t i=0; i<myLanes.size(); i++) {
        myLanes[i].shape.reshiftRotate(xoff, yoff, rot);
    }
}


Position2DVector
NBEdge::getCWBoundaryLine(const NBNode &n, SUMOReal offset) const
{
    Position2DVector ret;
    if (myFrom==(&n)) {
        // outgoing
        ret = myLanes[0].shape;
    } else {
        // incoming
        ret = myLanes[myLanes.size()-1].shape.reverse();
    }
    ret.move2side(offset);
    return ret;
}


Position2DVector
NBEdge::getCCWBoundaryLine(const NBNode &n, SUMOReal offset) const
{
    Position2DVector ret;
    if (myFrom==(&n)) {
        // outgoing
        ret = myLanes[myLanes.size()-1].shape;
    } else {
        // incoming
        ret = myLanes[0].shape.reverse();
    }
    ret.move2side(-offset);
    return ret;
}


SUMOReal
NBEdge::width() const
{
    return (SUMOReal) myLanes.size() * SUMO_const_laneWidth + (SUMOReal)(myLanes.size()-1) * SUMO_const_laneOffset;
}


bool
NBEdge::expandableBy(NBEdge *possContinuation) const
{
    // ok, the number of lanes must match
    if (myLanes.size()!=possContinuation->myLanes.size()) {
        return false;
    }
    // the priority, too (?)
    if (getPriority()!=possContinuation->getPriority()) {
        return false;
    }
    // the speed allowed
    if (mySpeed!=possContinuation->mySpeed) {
        return false;
    }
    // the vehicle class constraints, too
    /*!!!
    if (myAllowedOnLanes!=possContinuation->myAllowedOnLanes
            ||
            myNotAllowedOnLanes!=possContinuation->myNotAllowedOnLanes) {
        return false;
    }
    */
    // also, check whether the connections - if any exit do allow to join
    //  both edges
    // This edge must have a one-to-one connection to the following lanes
    switch (myStep) {
    case INIT_REJECT_CONNECTIONS:
        break;
    case INIT:
        break;
    case EDGE2EDGES: {
        // the following edge must be connected
        const EdgeVector &conn = getConnectedEdges();
        if (find(conn.begin(), conn.end(), possContinuation)
                ==conn.end()) {

            return false;
        }
    }
    break;
    case LANES2EDGES:
    case LANES2LANES:        
        {
        // the possible continuation must be connected
        if(find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(possContinuation))==myConnections.end()) {
            return false;
        }
        // all lanes must go to the possible continuation
        vector<int> conns = getConnectionLanes(possContinuation);
        if (conns.size()!=myLanes.size()) {
            return false;
        }
    }
    break;
    default:
        break;
    }
    return true;
}


void
NBEdge::append(NBEdge *e)
{
    // append geometry
    myGeom.appendWithCrossingPoint(e->myGeom);
    for (size_t i=0; i<myLanes.size(); i++) {
        myLanes[i].shape.appendWithCrossingPoint(e->myLanes[i].shape);
    }
    // recompute length
    myLength += e->myLength;
    // copy the connections and the building step if given
    myStep = e->myStep;
    myConnections = e->myConnections;
    myTurnDestination = e->myTurnDestination;
    // set the node
    myTo = e->myTo;
}





bool
NBEdge::hasSignalisedConnectionTo(NBEdge *e) const
{
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).toEdge==e && (*i).tlID!="") {
            return true;
        }
    }
    return false;
}


NBEdge*
NBEdge::getTurnDestination() const
{
    return myTurnDestination;
}


string
NBEdge::getLaneID(size_t lane)
{
    assert(lane<myLanes.size());
    return myID + "_" + toString<size_t>(lane);
}


bool
NBEdge::isNearEnough2BeJoined2(NBEdge *e)
{
    DoubleVector distances = myGeom.distances(e->getGeometry());
    if (distances.size()==0) {
        distances = e->getGeometry().distancesExt(myGeom);
    }
    SUMOReal max = VectorHelper<SUMOReal>::maxValue(distances);
    return max<7;
}


SUMOReal
NBEdge::getNormedAngle(const NBNode &atNode) const
{
    SUMOReal angle = getAngle(atNode);
    if (angle<0) {
        angle = 360 + angle;
    }
    assert(angle>=0&&angle<360);
    return angle;
}


SUMOReal
NBEdge::getNormedAngle() const
{
    SUMOReal angle = myAngle;
    if (angle<0) {
        angle = 360 + angle;
    }
    assert(angle>=0&&angle<360);
    return angle;
}


void
NBEdge::addGeometryPoint(int index, const Position2D &p)
{
    myGeom.insertAt(index, p);
}


void
NBEdge::incLaneNo(unsigned int by)
{
    size_t newLaneNo = myLanes.size() + by;
    while (myLanes.size()<newLaneNo) {
        Lane l;
        l.speed = mySpeed;
        myLanes.push_back(l);
    }
    computeLaneShapes();
    const EdgeVector &incs = myFrom->getIncomingEdges();
    for (EdgeVector::const_iterator i=incs.begin(); i!=incs.end(); ++i) {
        (*i)->invalidateConnections(true);
    }
    invalidateConnections(true);
}


void
NBEdge::decLaneNo(unsigned int by, int dir)
{
    size_t newLaneNo = myLanes.size() - by;
    while (myLanes.size()>newLaneNo) {
        myLanes.pop_back();
    }
    computeLaneShapes();
    const EdgeVector &incs = myFrom->getIncomingEdges();
    for (EdgeVector::const_iterator i=incs.begin(); i!=incs.end(); ++i) {
        (*i)->invalidateConnections(true);
    }
    if (dir==0) {
        invalidateConnections(true);
    } else {
        const EdgeVector &outs = myTo->getOutgoingEdges();
        assert(outs.size()==1);
        NBEdge *out = outs[0];
        if (dir<0) {
            removeFromConnections(out, 0);
        } else {
            removeFromConnections(out, myLanes.size());
        }
    }
}


void
NBEdge::copyConnectionsFrom(NBEdge *src)
{
    myStep = src->myStep;
    myConnections = src->myConnections;
}


void
NBEdge::setLaneSpreadFunction(LaneSpreadFunction spread)
{
    myLaneSpreadFunction = spread;
}


NBEdge::LaneSpreadFunction
NBEdge::getLaneSpreadFunction() const
{
    return myLaneSpreadFunction;
}


void
NBEdge::markAsInLane2LaneState()
{
    assert(myTo->getOutgoingEdges().size()==0);
    myStep = LANES2LANES;
}


bool
NBEdge::splitGeometry(NBEdgeCont &ec, NBNodeCont &nc)
{
    // check whether there any splits to perform
    if (myGeom.size()<3) {
        return false;
    }
    // ok, split
    NBNode *newFrom = myFrom;
    NBNode *myLastNode = myTo;
    NBNode *newTo = 0;
    NBEdge *currentEdge = this;
    for (size_t i=1; i<myGeom.size()-1; i++) {
        // build the node first
        if (i!=myGeom.size()-2) {
            string nodename = myID + "_in_between#" + toString(i);
            if (!nc.insert(nodename, myGeom[i])) {
                throw ProcessError("Error on adding in-between node '" + nodename + "'.");

            }
            newTo = nc.retrieve(nodename);
        } else {
            newTo = myLastNode;
        }
        if (i==1) {
            currentEdge->myTo->removeIncoming(this);
            currentEdge->myTo = newTo;
            newTo->addIncomingEdge(currentEdge);
        } else {
            string edgename = myID + "[" + toString(i-1) + "]";
            currentEdge = new NBEdge(edgename, newFrom, newTo, myType, mySpeed, myLanes.size(),
                                     myPriority, myLaneSpreadFunction);
            if (!ec.insert(currentEdge)) {
                throw ProcessError("Error on adding splitted edge '" + edgename + "'.");

            }
        }
        newFrom = newTo;
    }
    myGeom.clear();
    myGeom.push_back(myFrom->getPosition());
    myGeom.push_back(myTo->getPosition());
    myStep = INIT;
    return true;
}


void
NBEdge::allowVehicleClass(int lane, SUMOVehicleClass vclass)
{
    if (lane<0) {
        // if all lanes are meant...
        for (size_t i=0; i<myLanes.size(); i++) {
            // ... do it for each lane
            allowVehicleClass((int) i, vclass);
        }
        return;
    }
    assert(lane<(int) myLanes.size());
    // add it only if not already done
    if (find(myLanes[lane].allowed.begin(), myLanes[lane].allowed.end(), vclass)==myLanes[lane].allowed.end()) {
        myLanes[lane].allowed.push_back(vclass);
    }
}


void
NBEdge::disallowVehicleClass(int lane, SUMOVehicleClass vclass)
{
    if (OptionsCont::getOptions().getBool("dismiss-vclasses")) {
        return;
    }
    if (lane<0) {
        // if all lanes are meant...
        for (size_t i=0; i<myLanes.size(); i++) {
            // ... do it for each lane
            disallowVehicleClass((int) i, vclass);
        }
        return;
    }
    assert(lane<(int) myLanes.size());
    // add it only if not already done
    if (find(myLanes[lane].notAllowed.begin(), myLanes[lane].notAllowed.end(), vclass)==myLanes[lane].notAllowed.end()) {
        myLanes[lane].notAllowed.push_back(vclass);
    }
}


void 
NBEdge::setVehicleClasses(const vector<SUMOVehicleClass> &allowed, const vector<SUMOVehicleClass> &disallowed, int lane)
{
    for(vector<SUMOVehicleClass>::const_iterator i=allowed.begin(); i!=allowed.end(); ++i) {
        allowVehicleClass(lane, *i);
    }
    for(vector<SUMOVehicleClass>::const_iterator i=disallowed.begin(); i!=disallowed.end(); ++i) {
        disallowVehicleClass(lane, *i);
    }
}

void
NBEdge::recheckEdgeGeomForDoublePositions()
{
    myGeom.removeDoublePoints();
}


void
NBEdge::addAdditionalConnections()
{
    if (myStep==LANES2LANES) {
        return;
    }
    /*
    // go through the lanes
    for (size_t i=0; i<myLanes.size(); i++) {
        // get the connections from the current lane
        EdgeLaneVector reachableFromLane = myReachable[i];
        // go through these connections
        for (int j=0; j<(int) reachableFromLane.size(); ++j) {
            // get the current connection
            const EdgeLane &el = reachableFromLane[j];
            // get all connections that approach the current destination edge
            vector<int> other = getConnectionLanes(el.edge);
            // check whether we may add a connection on the right side
            //  we may not do this directly because the containers will change
            //  during the addition
            bool mayAddRight = false;
            bool mayAddRight2 = false;
            bool mayAddLeft = false;
            bool mayAddLeft2 = false;
            if (i!=0 && el.lane!=0 && find(other.begin(), other.end(), i-1)==other.end()) {
                mayAddRight = true;
            }
            if (i==0 && el.lane!=0 && find_if(reachableFromLane.begin(), reachableFromLane.end(), NBContHelper::edgelane_finder(el.edge, el.lane-1))==reachableFromLane.end()) {
                mayAddRight2 = true;
            }
            if (i+1<myLanes.size() && el.lane+1<el.edge->getNoLanes() && find(other.begin(), other.end(), i+1)==other.end()) {
                mayAddLeft = true;
            }
            if (i+1==myLanes.size() && el.lane+1<el.edge->getNoLanes() && find_if(reachableFromLane.begin(), reachableFromLane.end(), NBContHelper::edgelane_finder(el.edge, el.lane+1))==reachableFromLane.end()) {
                mayAddLeft2 = true;
            }
            // add the connections if possible
            if (mayAddRight) {
                cout << getID() << "->" << el.edge->getID() << endl;
                addLane2LaneConnection(i-1, el.edge, el.lane-1, false);
                j = -1;
            }
            if (mayAddRight2) {
                cout << getID() << "->" << el.edge->getID() << endl;
                addLane2LaneConnection(i, el.edge, el.lane-1, false);
                j = -1;
            }
            if (mayAddLeft) {
                addLane2LaneConnection(i+1, el.edge, el.lane+1, false);
                j = -1;
            }
            if (mayAddLeft2) {
                addLane2LaneConnection(i, el.edge, el.lane+1, false);
                j = -1;
            }
            reachableFromLane = myReachable[i];
        }
    }
    */
}


vector<SUMOVehicleClass>
NBEdge::getAllowedVehicleClasses() const
{
    vector<SUMOVehicleClass> ret;
    for(vector<Lane>::const_iterator i=myLanes.begin(); i!=myLanes.end(); ++i) {
        const vector<SUMOVehicleClass> &allowed = (*i).allowed;
        for (vector<SUMOVehicleClass>::const_iterator j=allowed.begin(); j!=allowed.end(); ++j) {
            if (find(ret.begin(), ret.end(), *j)==ret.end()) {
                ret.push_back(*j);
            }
        }
    }
    return ret;
}




int
NBEdge::getMinConnectedLane(NBEdge *of) const
{
    int ret = -1;
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).toEdge==of && (ret==-1 || (*i).toLane<ret)) {
            ret = (*i).toLane;
        }
    }
    return ret;
}


int
NBEdge::getMaxConnectedLane(NBEdge *of) const
{
    int ret = -1;
    for(vector<Connection>::const_iterator i=myConnections.begin(); i!=myConnections.end(); ++i) {
        if((*i).toEdge==of && (*i).toLane>ret) {
            ret = (*i).toLane;
        }
    }
    return ret;
}


void
NBEdge::setLoadedLength(SUMOReal val)
{
    myLoadedLength = val;
}


void
NBEdge::dismissVehicleClassInformation()
{
    for(vector<Lane>::iterator i=myLanes.begin(); i!=myLanes.end(); ++i) {
        (*i).allowed.clear();
        (*i).notAllowed.clear();
    }
}


/****************************************************************************/
