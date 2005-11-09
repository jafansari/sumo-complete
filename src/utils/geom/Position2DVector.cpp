//---------------------------------------------------------------------------//
//                        Position2DVector.cpp -
//  A list of 2D-positions
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.32  2005/11/09 06:45:15  dkrajzew
// complete geometry building rework (unfinished)
//
// Revision 1.31  2005/10/07 11:44:16  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.30  2005/09/23 06:07:01  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.29  2005/09/15 12:18:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.28  2005/07/12 12:44:17  dkrajzew
// access function improved
//
// Revision 1.27  2005/04/28 09:02:48  dkrajzew
// level3 warnings removed
//
// Revision 1.26  2004/11/23 10:34:46  dkrajzew
// debugging
//
// Revision 1.3  2004/11/22 12:53:15  dksumo
// added 'pop_back' and 'inserAt'
//
// Revision 1.2  2004/10/29 06:25:23  dksumo
// boundery renamed to boundary
//
// Revision 1.1  2004/10/22 12:50:44  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.25  2004/07/02 09:44:40  dkrajzew
// changes for 0.8.0.2
//
// Revision 1.24  2004/04/02 11:29:02  dkrajzew
// computation of moving a line strip to the side patched (is still not correct to 100%)
//
// Revision 1.23  2004/03/19 13:01:11  dkrajzew
// methods needed for the new selection within the gui added; some style adaptions
//
// Revision 1.22  2004/02/16 14:00:00  dkrajzew
// some further work on edge geometry
//
// Revision 1.21  2004/01/12 15:57:21  dkrajzew
// catching the case of not crossing lines on concatanation
//
// Revision 1.20  2003/12/09 11:33:49  dkrajzew
// made the assignment operator and copy constructor explicite in the wish to
//  save memory
//
// Revision 1.19  2003/11/18 14:21:20  dkrajzew
// computation of junction-inlanes geometry added
//
// Revision 1.18  2003/11/11 08:01:24  dkrajzew
// some further methods implemented
//
// Revision 1.17  2003/10/21 14:39:11  dkrajzew
// the rotation information now returns the last valid value if the length
//  is exceeded
//
// Revision 1.16  2003/10/17 06:50:02  dkrajzew
// patched the false usage of a reference
//
// Revision 1.15  2003/10/15 11:56:30  dkrajzew
// further work on vissim-import
//
// Revision 1.14  2003/10/02 14:55:56  dkrajzew
// visualisation of E2-detectors implemented
//
// Revision 1.13  2003/09/25 09:03:53  dkrajzew
// some methods added, needed for the computation of line rotation
//
// Revision 1.12  2003/09/05 15:27:38  dkrajzew
// changes from adding internal lanes and further work on node geometry
//
// Revision 1.11  2003/08/20 11:47:38  dkrajzew
// bug in sorting the values by x, then y patched
//
// Revision 1.10  2003/08/14 14:05:50  dkrajzew
// functions to process a nodes geometry added
//
// Revision 1.9  2003/07/16 15:38:04  dkrajzew
// some work on computation and handling of geometry information
//
// Revision 1.8  2003/07/07 08:48:35  dkrajzew
// remved an unneeded debug-output
//
// Revision 1.7  2003/06/05 14:33:45  dkrajzew
// class templates applied; documentation added
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <queue>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "AbstractPoly.h"
#include "Position2D.h"
#include "Position2DVector.h"
#include "GeomHelper.h"
#include "Line2D.h"
#include "Helper_ConvexHull.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * debug definitions
 * ======================================================================= */
#define DEBUG_OUT cout


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
Position2DVector::Position2DVector()
{
}


Position2DVector::Position2DVector(size_t fieldSize)
    : myCont(fieldSize)
{
}


Position2DVector::~Position2DVector()
{
}


void
Position2DVector::push_back(const Position2D &p)
{
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
    if(myCont.size()!=0&&myCont[myCont.size()-1]==p) {
        DEBUG_OUT << "on push_back" << endl;
        DEBUG_OUT << *this << endl;
    }
#endif
#endif
//    assert(myCont.size()==0||myCont[myCont.size()-1]!=p);
    myCont.push_back(p);
}


void
Position2DVector::push_back(const Position2DVector &p)
{
    copy(p.myCont.begin(), p.myCont.end(), back_inserter(myCont));
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
    if(!assertNonEqual()) {
        DEBUG_OUT << "on push_back2" << endl;
        throw 1;
    }
#endif
#endif
}


void
Position2DVector::push_front(const Position2D &p)
{
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
    if(myCont.size()!=0&&myCont[0]==p) {
        DEBUG_OUT << "on push_front" << endl;
        DEBUG_OUT << *this << endl;
        throw 1;
    }
#endif
#endif
	assert(myCont.size()==0||myCont[0]!=p);
    myCont.push_front(p);
}


bool
Position2DVector::around(const Position2D &p, SUMOReal offset) const
{
    if(offset!=0) {
        //throw 1; // !!! not yet implemented
    }
    SUMOReal angle=0;
    for (ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
       Position2D p1(
           (*i).x() - p.x(),
           (*i).y() - p.y());
       Position2D p2(
           (*(i+1)).x() - p.x(),
           (*(i+1)).y() - p.y());
       angle += GeomHelper::Angle2D(p1.x(), p1.y(), p2.x(), p2.y());
    }
    Position2D p1(
        (*(myCont.end()-1)).x() - p.x(),
        (*(myCont.end()-1)).y() - p.y());
    Position2D p2(
        (*(myCont.begin())).x() - p.x(),
        (*(myCont.begin())).y() - p.y());
    angle += GeomHelper::Angle2D(p1.x(), p1.y(), p2.x(), p2.y());
    return (!(fabs(angle) < 3.1415926535897932384626433832795));
}


bool
Position2DVector::overlapsWith(const AbstractPoly &poly, SUMOReal offset) const
{
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(poly.around(*i, offset)) {
            return true;
        }
    }
    return false;
}


bool
Position2DVector::intersects(const Position2D &p1, const Position2D &p2) const
{
    if(size()<2) {
        return false;
    }
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(GeomHelper::intersects(*i, *(i+1), p1, p2)) {
            return true;
        }
    }
    //return GeomHelper::intersects(*(myCont.end()-1), *(myCont.begin()), p1, p2);
    return false;
}


bool
Position2DVector::intersects(const Position2DVector &v1) const
{
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(v1.intersects(*i, *(i+1))) {
            return true;
        }
    }
    //return v1.intersects(*(myCont.end()-1), *(myCont.begin()));
    return false;
}


Position2D
Position2DVector::intersectsAtPoint(const Position2D &p1,
                                    const Position2D &p2) const
{
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(GeomHelper::intersects(*i, *(i+1), p1, p2)) {
            return GeomHelper::intersection_position(*i, *(i+1), p1, p2);
        }
    }
    return Position2D(-1, -1);
}


Position2D
Position2DVector::intersectsAtPoint(const Position2DVector &v1) const
{
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(v1.intersects(*i, *(i+1))) {
            return v1.intersectsAtPoint(*i, *(i+1));
        }
    }
    /*
    if(v1.intersects(*(myCont.end()-1), *(myCont.begin()))) {
        return v1.intersectsAtPoint(*(myCont.end()-1), *(myCont.begin()));
    }
    */
    return Position2D(-1, -1);
}


void
Position2DVector::clear()
{
    myCont.clear();
}


const Position2D &
Position2DVector::at(int i) const
{
    if(i>=0) {
        return myCont[i];
    } else {
        return myCont[myCont.size()+i];
    }
}


size_t
Position2DVector::size() const
{
    return myCont.size();
}



Position2D
Position2DVector::positionAtLengthPosition(SUMOReal pos) const
{
    ContType::const_iterator i=myCont.begin();
    SUMOReal seenLength = 0;
    do {
        SUMOReal nextLength = GeomHelper::distance(*i, *(i+1));
        if(seenLength+nextLength>pos) {
            return positionAtLengthPosition(*i, *(i+1), pos-seenLength);
        }
        seenLength += nextLength;
    } while(++i!=myCont.end()-1);
    return myCont[myCont.size()-1];
    assert(false);
//    return positionAtLengthPosition(*(myCont.end()-1),
//        *(myCont.begin()), pos-seenLength);
}


SUMOReal
Position2DVector::rotationDegreeAtLengthPosition(SUMOReal pos) const
{
    ContType::const_iterator i=myCont.begin();
    SUMOReal seenLength = 0;
    do {
        SUMOReal nextLength = GeomHelper::distance(*i, *(i+1));
        if(seenLength+nextLength>pos) {
            Line2D l(*i, *(i+1));
            return l.atan2DegreeAngle();
        }
        seenLength += nextLength;
    } while(++i!=myCont.end()-1);
    Line2D l(*(myCont.end()-2), *(myCont.end()-1));
    return l.atan2DegreeAngle();
//    Line2D l(*(myCont.end()-1), *(myCont.begin()));
//    return l.atan2DegreeAngle();
}


Position2D
Position2DVector::positionAtLengthPosition(const Position2D &p1,
                                           const Position2D &p2,
                                           SUMOReal pos)
{
    SUMOReal dist = GeomHelper::distance(p1, p2);
    if(dist<pos) {
        return Position2D(-1, -1);
    }
    SUMOReal x = p1.x() + (p2.x() - p1.x()) / dist * pos;
    SUMOReal y = p1.y() + (p2.y() - p1.y()) / dist * pos;
    return Position2D(x, y);
}


Boundary
Position2DVector::getBoxBoundary() const
{
    Boundary ret;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end(); i++) {
        ret.add(*i);
    }
    return ret;
}


Position2D
Position2DVector::center() const
{
    SUMOReal x = 0;
    SUMOReal y = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end(); i++) {
        x += (*i).x();
        y += (*i).y();
    }
    return Position2D(x/(SUMOReal) myCont.size(), y/(SUMOReal) myCont.size());
}


SUMOReal
Position2DVector::length() const
{
    SUMOReal len = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        len += GeomHelper::distance(*i, *(i+1));
    }
    return len;
}


bool
Position2DVector::partialWithin(const AbstractPoly &poly, SUMOReal offset) const
{
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(poly.around(*i, offset)) {
            return true;
        }
    }
    return false;
}



bool
Position2DVector::crosses(const Position2D &p1, const Position2D &p2) const
{
    return intersects(p1, p2);
}



const Position2D &
Position2DVector::getBegin() const
{
    return myCont[0];
}


const Position2D &
Position2DVector::getEnd() const
{
    return myCont[myCont.size()-1];
}


std::pair<Position2DVector, Position2DVector>
Position2DVector::splitAt(SUMOReal where) const
{
    assert(size()>=2);
    assert(where!=0);
    Position2DVector one, two;
    Position2D last = myCont[0];
    Position2D curr = myCont[0];
    SUMOReal seen = 0;
    SUMOReal currdist = 0;
    one.push_back(myCont[0]);
    ContType::const_iterator i=myCont.begin()+1;

    do {
        last = curr;
        curr = *i;
        currdist = GeomHelper::distance(last, curr);
        if(seen+currdist<where&&i!=myCont.begin()+1) {
            one.push_back(last);
        }
        i++;
        seen += currdist;
    } while(seen<where&&i!=myCont.end());
    seen -= currdist;
    i--;

    if(seen+currdist==where) {
        one.push_back(curr);
    } else {
        Line2D tmpL(last, curr);
        assert(seen+currdist>where);
        Position2D p = tmpL.getPositionAtDistance(seen+currdist-where);
        one.push_back(p);
        two.push_back(p);
    }

    for(; i!=myCont.end(); i++) {
        two.push_back(*i);
    }
    assert(one.size()>=2);
    assert(two.size()>=2);
    return std::pair<Position2DVector, Position2DVector>(one, two);
}



std::ostream &
operator<<(std::ostream &os, const Position2DVector &geom)
{
    for(Position2DVector::ContType::const_iterator i=geom.myCont.begin(); i!=geom.myCont.end(); i++) {
        if(i!=geom.myCont.begin()) {
            os << " ";
        }
        os << (*i).x() << "," << (*i).y();
    }
    return os;
}


Position2DVector
Position2DVector::resettedBy(SUMOReal x, SUMOReal y) const
{
    Position2DVector ret;
    for(size_t i=0; i<size(); i++) {
        Position2D pos = at(i);
        pos.add(x, y);
        ret.push_back(pos);
    }
    return ret;
}


void
Position2DVector::resetBy(SUMOReal x, SUMOReal y)
{
    for(size_t i=0; i<size(); i++) {
        myCont[i].add(x, y);
    }
}


void
Position2DVector::resetBy(const Position2D &by)
{
    for(size_t i=0; i<size(); i++) {
        myCont[i].add(by.x(), by.y());
    }
}


void
Position2DVector::sortAsPolyCWByAngle()
{
    Position2D c = center();
    std::sort(myCont.begin(), myCont.end(), as_poly_cw_sorter(c));
}


void
Position2DVector::reshiftRotate(SUMOReal xoff, SUMOReal yoff, SUMOReal rot)
{
    for(size_t i=0; i<size(); i++) {
        myCont[i].reshiftRotate(xoff, yoff, rot);
    }
}


Position2DVector::as_poly_cw_sorter::as_poly_cw_sorter(Position2D center)
    : _center(center)
{
}



int
Position2DVector::as_poly_cw_sorter::operator() (const Position2D &p1,
                                                 const Position2D &p2) const
{
    return atan2(p1.x(), p1.y()) < atan2(p2.x(), p2.y());
}



void
Position2DVector::sortByIncreasingXY()
{
    std::sort(myCont.begin(), myCont.end(), increasing_x_y_sorter());
}



Position2DVector::increasing_x_y_sorter::increasing_x_y_sorter()
{
}



int
Position2DVector::increasing_x_y_sorter::operator() (const Position2D &p1,
                                                 const Position2D &p2) const
{
    if(p1.x()!=p2.x()) {
        return p1.x()<p2.x();
    }
    return p1.y()<p2.y();
}



SUMOReal
Position2DVector::isLeft(const Position2D &P0, const Position2D &P1,
                         const Position2D &P2 ) const
{
    return (P1.x() - P0.x())*(P2.y() - P0.y()) - (P2.x() - P0.x())*(P1.y() - P0.y());
}


Position2DVector
Position2DVector::convexHull() const
{
    Position2DVector ret = *this;
    ret.sortAsPolyCWByAngle();
	return simpleHull_2D(ret);
}


void
Position2DVector::set(size_t pos, const Position2D &p)
{
    myCont[pos] = p;
}



Position2DVector
Position2DVector::intersectsAtPoints(const Position2D &p1,
                                     const Position2D &p2) const
{
    Position2DVector ret;
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
    if(!assertNonEqual()) {
        DEBUG_OUT << *this << endl;
        DEBUG_OUT << "in iintersects at points" << endl;
        throw 1;
    }
#endif
#endif
	for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        if(GeomHelper::intersects(*i, *(i+1), p1, p2)) {
            ret.push_back_noDoublePos(GeomHelper::intersection_position(*i, *(i+1), p1, p2));
        }
    }
    return ret;
}


int
Position2DVector::appendWithCrossingPoint(const Position2DVector &v)
{
    if(GeomHelper::distance(myCont[myCont.size()-1], v.myCont[0])<2) { // !!! heuristic
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
        if(!assertNonEqual()) {
            DEBUG_OUT << "this in appendwithcrossing" << endl;
            DEBUG_OUT << *this << endl;
            throw 1;
        }
        if(!v.assertNonEqual()) {
            DEBUG_OUT << "v in appendwithcrossing" << endl;
            DEBUG_OUT << v << endl;
            throw 1;
        }
#endif
#endif
		copy(v.myCont.begin()+1, v.myCont.end(), back_inserter(myCont));
        return 1;
    }
    //
    Line2D l1(myCont[myCont.size()-2], myCont[myCont.size()-1]);
    l1.extrapolateBy(100);
    Line2D l2(v.myCont[0], v.myCont[1]);
    l2.extrapolateBy(100);
    if(l1.intersects(l2)&&l1.intersectsAtLength(l2)<l1.length()-100) { // !!! heuristic
        Position2D p = l1.intersectsAt(l2);
        myCont[myCont.size()-1] = p;
        copy(v.myCont.begin()+1, v.myCont.end(), back_inserter(myCont));
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
        if(!assertNonEqual()) {
            DEBUG_OUT << "this in appendwithcrossing2" << endl;
            DEBUG_OUT << *this << endl;
            throw 1;
        }
#endif
#endif
		return 2;
    } else {
        copy(v.myCont.begin(), v.myCont.end(), back_inserter(myCont));
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
        if(!assertNonEqual()) {
            DEBUG_OUT << "this in appendwithcrossing3" << endl;
            DEBUG_OUT << *this << endl;
            throw 1;
        }
#endif
#endif
		return 3;
    }
}


Position2DVector
Position2DVector::getSubpart(SUMOReal begin, SUMOReal end) const
{
    Position2DVector ret;
    Position2D begPos = positionAtLengthPosition(begin);
    Position2D endPos = myCont[myCont.size()-1];
    if(length()>end) {
        endPos = positionAtLengthPosition(end);
    }
    ret.push_back(begPos);

    SUMOReal seen = 0;
    ContType::const_iterator i = myCont.begin();
    // skip previous segments
    while((i+1)!=myCont.end()
        &&
        seen+GeomHelper::distance((*i), *(i+1))<begin) {
        seen += GeomHelper::distance((*i), *(i+1));
        i++;
    }
    // append segments in between
    while((i+1)!=myCont.end()
        &&
        seen+GeomHelper::distance((*i), *(i+1))<end) {

        ret.push_back_noDoublePos(*(i+1));
        /*
        if(ret.at(-1)!=*(i+1)) {
            ret.push_back(*(i+1));
        }
        */
        seen += GeomHelper::distance((*i), *(i+1));
        i++;
    }
    // append end
    ret.push_back_noDoublePos(endPos);
    return ret;
}


void
Position2DVector::pruneFromBeginAt(const Position2D &p)
{
    // find minimum distance (from the begin)
    size_t pos = 0;
    SUMOReal dist = 1000000;
    size_t currPos = 0;
    SUMOReal currDist = GeomHelper::DistancePointLine(p,
        GeomHelper::extrapolate_first(*(myCont.begin()), *(myCont.begin()+1), 100),
        *(myCont.begin()+1));
//    assert(currDist>=0);
    if(currDist>=0&&currDist<dist) {
        dist = currDist;
        pos = currPos;
    }

    for(ContType::iterator i=myCont.begin(); i!=myCont.end()-1; i++, currPos++) {
        SUMOReal currDist = GeomHelper::DistancePointLine(p, *i, *(i+1));
        if(currDist>=0&&currDist<dist) {
            dist = currDist;
            pos = currPos;
        }
    }
    // remove leading items
    for(size_t j=0; j<pos; j++) {
        myCont.erase(myCont.begin());
    }
    // replace first item by the new position
    SUMOReal lpos = GeomHelper::nearest_position_on_line_to_point(
        myCont[0], myCont[1], p);
    if(lpos==-1) {
        return;
    }
    Position2D np = positionAtLengthPosition(lpos);
    if(np!=*(myCont.begin())) {
        myCont.erase(myCont.begin());
        if(np!=*(myCont.begin())) {
            myCont.push_front(np);
            assert(myCont.size()>1);
            assert(*(myCont.begin())!=*(myCont.end()-1));
        }
    }
}


void
Position2DVector::pruneFromEndAt(const Position2D &p)
{
    // find minimum distance (from the end)
    size_t pos = 0;
    SUMOReal dist = 1000000;
    size_t currPos = 0;
    SUMOReal currDist = GeomHelper::DistancePointLine(p,
        *(myCont.end()-1),
        GeomHelper::extrapolate_second(*(myCont.end()-2), *(myCont.end()-1), 100));
//    assert(currDist>=0);
    if(currDist>=0&&currDist<dist) {
        dist = currDist;
        pos = currPos;
    }

    for(ContType::reverse_iterator i=myCont.rbegin(); i!=myCont.rend()-1; i++, currPos++) {
        SUMOReal currDist = GeomHelper::DistancePointLine(p, *(i), *(i+1));
        if(currDist>=0&&currDist<dist) {
            dist = currDist;
            pos = currPos;
        }
    }
    // remove trailing items
    for(size_t j=0; j<pos; j++) {
        myCont.erase(myCont.end()-1);
    }
    // replace last item by the new position
    SUMOReal lpos =
        GeomHelper::nearest_position_on_line_to_point(
            myCont[myCont.size()-1], myCont[myCont.size()-2], p);
    if(lpos==-1) {
        return;
    }
    Position2D np = positionAtLengthPosition(
        length() - lpos);
    if(np!=*(myCont.end()-1)) {
        myCont.erase(myCont.end()-1);
        if(np!=*(myCont.end()-1)) {
            myCont.push_back(np);
            assert(myCont.size()>1);
            assert(*(myCont.begin())!=*(myCont.end()-1));
        }
    }
}


SUMOReal
Position2DVector::beginEndAngle() const
{
    Line2D tmp(getBegin(), getEnd());
    return tmp.atan2Angle();
}


void
Position2DVector::eraseAt(int i)
{
    if(i>=0) {
        myCont.erase(myCont.begin()+i);
    } else {
        myCont.erase(myCont.end()+i);
    }
}


SUMOReal
Position2DVector::nearest_position_on_line_to_point(const Position2D &p) const
{
    SUMOReal shortestDist = -1;
    SUMOReal nearestPos = -1;
    SUMOReal seen = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        SUMOReal pos =
            GeomHelper::nearest_position_on_line_to_point(*i, *(i+1), p);
        SUMOReal dist =
            pos < 0 ? -1 : GeomHelper::distance(p, positionAtLengthPosition(pos+seen));
        //
        if(dist>=0&&(shortestDist<0||shortestDist>dist)) {
            nearestPos = pos+seen;
            shortestDist = dist;
        }
        seen += GeomHelper::distance(*i, *(i+1));
        //
    }
    return nearestPos;
}


SUMOReal
Position2DVector::distance(const Position2D &p) const
{
    SUMOReal shortestDist = 10000000;
    SUMOReal nearestPos = 10000;
    SUMOReal seen = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        SUMOReal pos = seen +
            GeomHelper::nearest_position_on_line_to_point(*i, *(i+1), p);
        SUMOReal dist =
            GeomHelper::distance(p, positionAtLengthPosition(pos));
        //
        if(shortestDist>dist) {
            nearestPos = pos;
            shortestDist = dist;
        }
        //
    }
    if(shortestDist==10000000) {
        return -1;
    }
    return shortestDist;
}


DoubleVector
Position2DVector::intersectsAtLengths(const Position2DVector &s) const
{
    DoubleVector ret;
    SUMOReal pos = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        Line2D l((*i), *(i+1));
        DoubleVector atSegment = l.intersectsAtLengths(s);
        DoubleVectorHelper::add2All(atSegment, pos);
        copy(atSegment.begin(), atSegment.end(), back_inserter(ret));
        pos += l.length();
    }
    return ret;
}


DoubleVector
Position2DVector::intersectsAtLengths(const Line2D &s) const
{
    DoubleVector ret;
    SUMOReal pos = 0;
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; i++) {
        Line2D l((*i), *(i+1));
        if(GeomHelper::intersects(l.p1(), l.p2(), s.p1(), s.p2())) {
            Position2D p =
                GeomHelper::intersection_position(l.p1(), l.p2(), s.p1(), s.p2());
            SUMOReal atLength = GeomHelper::distance(l.p1(), p);
            ret.push_back(atLength+pos);
        }
        pos += l.length();
    }
    return ret;
}


void
Position2DVector::extrapolate(SUMOReal val)
{
    assert(myCont.size()>1);
    Position2D nb =
        GeomHelper::extrapolate_first(myCont[0], myCont[1], val);
    Position2D ne =
        GeomHelper::extrapolate_second(
            myCont[myCont.size()-2], myCont[myCont.size()-1], val);
    myCont.erase(myCont.begin());
    push_front(nb);
    myCont.erase(myCont.end()-1);
    push_back(ne);
}


Position2DVector
Position2DVector::reverse() const
{
    Position2DVector ret;
    for(ContType::const_reverse_iterator i=myCont.rbegin(); i!=myCont.rend(); i++) {
        ret.push_back(*i);
    }
    return ret;
}


void
Position2DVector::move2side(SUMOReal amount)
{
    if(myCont.size()<2) {
        return;
    }
    ContType newCont;
    std::pair<SUMOReal, SUMOReal> p;
    Position2D newPos;
    // first point
    newPos = (*(myCont.begin()));
    p = GeomHelper::getNormal90D_CW(*(myCont.begin()), *(myCont.begin()+1), amount);
    newPos.add(p.first, p.second);
    newCont.push_back(newPos);
    // middle points
    for(ContType::const_iterator i=myCont.begin()+1; i!=myCont.end()-1; i++) {
        std::pair<SUMOReal, SUMOReal> oldp = p;
        newPos = *i;
        newPos.add(p.first, p.second);
        newCont.push_back(newPos);
        p = GeomHelper::getNormal90D_CW(*i, *(i+1), amount);
//        Position2D newPos(*i);
//        newPos.add((p.first+oldp.first)/2.0, (p.second+oldp.second)/2.0);
//        newCont.push_back(newPos);
    }
    // last point
    newPos = (*(myCont.end()-1));
    newPos.add(p.first, p.second);
    newCont.push_back(newPos);
    myCont = newCont;
}


Line2D
Position2DVector::lineAt(size_t pos) const
{
    assert(myCont.size()>pos+1);
    return Line2D(myCont[pos], myCont[pos+1]);
}


Line2D
Position2DVector::getBegLine() const
{
    return lineAt(0);
}


Line2D
Position2DVector::getEndLine() const
{
    return lineAt(myCont.size()-2);
}


void
Position2DVector::closePolygon()
{
    if(myCont[0]==myCont[myCont.size()-1]) {
        return;
    }
#ifdef _DEBUG
#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
    if(myCont.size()!=0&&myCont[myCont.size()-1]==myCont[0]) {
        DEBUG_OUT << "on closePlygon" << endl;
        DEBUG_OUT << *this << endl;
        throw 1;
    }
    if(size()<2) {
        DEBUG_OUT << *this << endl;
        DEBUG_OUT << "nope1" << endl;
        throw 1;
    }
#endif
#endif
	push_back(myCont[0]);
}


Position2DVector::Position2DVector(const Position2DVector &s)
{
    myCont.assign(s.myCont.begin(), s.myCont.end());
}


Position2DVector &
Position2DVector::operator=(const Position2DVector &s)
{
    myCont.assign(s.myCont.begin(), s.myCont.end());
    return *this;
}


DoubleVector
Position2DVector::distances(const Position2DVector &s) const
{
    DoubleVector ret;
    ContType::const_iterator i;
    for(i=myCont.begin(); i!=myCont.end(); i++) {
        SUMOReal dist = s.distance(*i);
        // !!! aeh, possible at the ends?
        if(dist!=-1) {
            ret.push_back(dist);
        }
    }
    for(i=s.myCont.begin(); i!=s.myCont.end(); i++) {
        SUMOReal dist = distance(*i);
        // !!! aeh, possible at the ends?
        if(dist!=-1) {
            ret.push_back(dist);
        }
    }
    return ret;
}


DoubleVector
Position2DVector::distancesExt(const Position2DVector &s) const
{
    DoubleVector ret = distances(s);
    ContType::const_iterator i;
    for(i=myCont.begin(); i!=myCont.end()-1; i++) {
        Position2D p = Position2D(*i);
        p.add(*(i+1));
        p.mul(0.5);
        SUMOReal dist = s.distance(p);
        // !!! aeh, possible at the ends?
        if(dist!=-1) {
            ret.push_back(dist);
        }
    }
    for(i=s.myCont.begin(); i!=s.myCont.end()-1; i++) {
        Position2D p = Position2D(*i);
        p.add(*(i+1));
        p.mul(0.5);
        SUMOReal dist = s.distance(p);
        // !!! aeh, possible at the ends?
        if(dist!=-1) {
            ret.push_back(dist);
        }
    }
    return ret;
}


Position2D
Position2DVector::pop_back()
{
    Position2D last = myCont.back();
    myCont.erase(myCont.end()-1);
    return last;
}


Position2D
Position2DVector::pop_front()
{
    Position2D first = myCont.front();
    myCont.erase(myCont.begin());
    return first;
}


void
Position2DVector::insertAt(int index, const Position2D &p)
{
    if(index>=0) {
        myCont.insert(myCont.begin()+index, p);
    } else {
        myCont.insert(myCont.end()+index, p);
    }
}


#ifdef CHECK_UNIQUE_POINTS_GEOMETRY
bool
Position2DVector::assertNonEqual() const
{
    if(myCont.size()<2) {
        return true;
    }
    for(ContType::const_iterator i=myCont.begin(); i!=myCont.end()-1; ++i) {
        if(*i==*(i+1)) {
            return false;
        }
    }
    return true;
}
#endif


void
Position2DVector::push_back_noDoublePos(const Position2D &p, float eps)
{
    if(size()==0) {
        myCont.push_back(p);
    } else {
        const Position2D &pp = myCont[myCont.size()-1];
        if(fabs(pp.x()-p.x())>eps||fabs(pp.y()-p.y())>eps) {
            myCont.push_back(p);
        }
    }
}


void
Position2DVector::push_front_noDoublePos(const Position2D &p, float eps)
{
    if(size()==0) {
        myCont.push_front(p);
    } else {
        const Position2D &pp = myCont[0];
        if(fabs(pp.x()-p.x())>eps||fabs(pp.y()-p.y())>eps) {
            myCont.push_front(p);
        }
    }
}


void
Position2DVector::replaceAt(size_t index, const Position2D &by)
{
    assert(size()>index);
    myCont[index] = by;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


