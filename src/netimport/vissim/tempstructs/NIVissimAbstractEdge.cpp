/****************************************************************************/
/// @file    NIVissimAbstractEdge.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// -------------------
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


#include <map>
#include <cassert>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Line2D.h>
#include <utils/geom/GeoConvHelper.h>
#include "NIVissimAbstractEdge.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


NIVissimAbstractEdge::DictType NIVissimAbstractEdge::myDict;

NIVissimAbstractEdge::NIVissimAbstractEdge(int id,
        const Position2DVector &geom)
        : myID(id), myNode(-1)
{
    // convert/publicate geometry
    std::deque<Position2D>::const_iterator i;
    const std::deque<Position2D> &geomC = geom.getCont();
    for (i=geomC.begin(); i!=geomC.end(); ++i) {
        Position2D p = *i;
        GeoConvHelper::x2cartesian(p);
        myGeom.push_back_noDoublePos(p);
    }
    //
    dictionary(id, this);
}


NIVissimAbstractEdge::~NIVissimAbstractEdge()
{}


bool
NIVissimAbstractEdge::dictionary(int id, NIVissimAbstractEdge *e)
{
    DictType::iterator i=myDict.find(id);
    if (i==myDict.end()) {
        myDict[id] = e;
        return true;
    }
    return false;
}


NIVissimAbstractEdge *
NIVissimAbstractEdge::dictionary(int id)
{
    DictType::iterator i=myDict.find(id);
    if (i==myDict.end()) {
        return 0;
    }
    return (*i).second;
}



Position2D
NIVissimAbstractEdge::getGeomPosition(SUMOReal pos) const
{
    if (myGeom.length()<pos) {
        SUMOReal amount = pos - myGeom.length();
        Line2D l(myGeom[-2], GeomHelper::extrapolate_second(myGeom[-2], myGeom[-1], amount*2));
        return l.getPositionAtDistance(pos);
    }
    return myGeom.positionAtLengthPosition(pos);
}
bool
NIVissimAbstractEdge::hasGeom() const
{
    return myGeom.size()>0;
}



void
NIVissimAbstractEdge::splitAndAssignToNodes()
{
    for (DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        NIVissimAbstractEdge *e = (*i).second;
        e->splitAssigning();
    }
}

void
NIVissimAbstractEdge::splitAssigning()
{}





bool
NIVissimAbstractEdge::crossesEdge(NIVissimAbstractEdge *c) const
{
    return myGeom.intersects(c->myGeom);
}


Position2D
NIVissimAbstractEdge::crossesEdgeAtPoint(NIVissimAbstractEdge *c) const
{
    return myGeom.intersectsAtPoint(c->myGeom);
}


SUMOReal
NIVissimAbstractEdge::crossesAtPoint(const Position2D &p1,
                                     const Position2D &p2) const
{
    // !!! not needed
    Position2D p = GeomHelper::intersection_position(
                       myGeom.getBegin(), myGeom.getEnd(), p1, p2);
    return GeomHelper::nearest_position_on_line_to_point(
               myGeom.getBegin(), myGeom.getEnd(), p);
}



IntVector
NIVissimAbstractEdge::getWithin(const AbstractPoly &p, SUMOReal offset)
{
    IntVector ret;
    for (DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        NIVissimAbstractEdge *e = (*i).second;
        if (e->overlapsWith(p, offset)) {
            ret.push_back(e->myID);
        }
    }
    return ret;
}


bool
NIVissimAbstractEdge::overlapsWith(const AbstractPoly &p, SUMOReal offset) const
{
    return myGeom.overlapsWith(p, offset);
}


bool
NIVissimAbstractEdge::hasNodeCluster() const
{
    return myNode!=-1;
}


int
NIVissimAbstractEdge::getID() const
{
    return myID;
}

void
NIVissimAbstractEdge::clearDict()
{
    for (DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete(*i).second;
    }
    myDict.clear();
}


const Position2DVector &
NIVissimAbstractEdge::getGeometry() const
{
    return myGeom;
}


void
NIVissimAbstractEdge::addDisturbance(int disturbance)
{
    myDisturbances.push_back(disturbance);
}


const IntVector &
NIVissimAbstractEdge::getDisturbances() const
{
    return myDisturbances;
}



/****************************************************************************/

