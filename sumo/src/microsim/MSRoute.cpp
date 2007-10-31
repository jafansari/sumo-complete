/****************************************************************************/
/// @file    MSRoute.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A vehicle route
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

#include <cassert>
#include <algorithm>
#include "MSRoute.h"
#include "MSEdge.h"
#include <utils/common/FileHelpers.h>
#include <utils/iodevices/BinaryInputDevice.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef ABS_DEBUG
#include "MSNet.h"
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// static member variables
// ===========================================================================
MSRoute::RouteDict MSRoute::myDict;


// ===========================================================================
// member method definitions
// ===========================================================================
MSRoute::MSRoute(const std::string &id,
                 const MSEdgeVector &edges,
                 bool multipleReferenced)
        : Named(id), myEdges(edges),
        myMultipleReferenced(multipleReferenced)
{}


MSRoute::~MSRoute()
{}

MSRouteIterator
MSRoute::begin() const
{
    return myEdges.begin();
}

MSRouteIterator
MSRoute::end() const
{
    return myEdges.end();
}

unsigned
MSRoute::size() const
{
    return myEdges.size();
}


const MSEdge *
MSRoute::getLastEdge() const
{
    assert(myEdges.size()>0);
    return myEdges[myEdges.size()-1];
}

bool
MSRoute::dictionary(const string &id, MSRoute* route)
{
    RouteDict::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        myDict.insert(RouteDict::value_type(id, route));
        return true;
    }
    return false;
}


MSRoute*
MSRoute::dictionary(const string &id)
{
    RouteDict::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        return 0;
    }
    return it->second;
}


void
MSRoute::clear()
{
    for (RouteDict::iterator i=myDict.begin(); i!=myDict.end(); ++i) {
        delete(*i).second;
    }
    myDict.clear();
}


void
MSRoute::erase(std::string id)
{
    RouteDict::iterator i=myDict.find(id);
    assert(i!=myDict.end());
    delete(*i).second;
    myDict.erase(id);
}


bool
MSRoute::inFurtherUse() const
{
    return myMultipleReferenced;
}


bool
MSRoute::replaceBy(const MSEdgeVector &edges, MSRouteIterator &currentEdge)
{
    // do not replace if the vehicle is already out of the route
    MSEdgeVector::const_iterator i =
        std::find(edges.begin(), edges.end(), *currentEdge);
    if (i==edges.end()) {
        return false;
    }
    MSEdgeVector n;
    copy(myEdges.begin(), std::find(myEdges.begin(), myEdges.end(), *currentEdge),
         back_inserter(n));
    copy(i, edges.end(), back_inserter(n));
    myEdges = n;
    return true;
}


MSRouteIterator
MSRoute::find(const MSEdge *e) const
{
    return std::find(myEdges.begin(), myEdges.end(), e);
}


void
MSRoute::writeEdgeIDs(OutputDevice &os) const
{
    MSEdgeVector::const_iterator i = myEdges.begin();
    for (;i!=myEdges.end(); ++i) {
        if (i!=myEdges.begin()) {
            os << ' ';
        }
        os << (*i)->getID();
    }
}


bool
MSRoute::contains(MSEdge *edge) const
{
    return find(edge)!=myEdges.end();
}


bool
MSRoute::containsAnyOf(const std::vector<MSEdge*> &edgelist) const
{
    std::vector<MSEdge*>::const_iterator i = edgelist.begin();
    for (; i!=edgelist.end(); ++i) {
        if (contains(*i)) {
            return true;
        }
    }
    return false;
}


const MSEdge *
MSRoute::operator[](unsigned index)
{
    return myEdges[index];
}


void
MSRoute::dict_saveState(std::ostream &os)
{
    FileHelpers::writeUInt(os, myDict.size());
    for (RouteDict::iterator it = myDict.begin(); it!=myDict.end(); ++it) {
        (*it).second->saveState(os);
    }
}


void
MSRoute::saveState(std::ostream &os)
{
    FileHelpers::writeString(os, getID());
    FileHelpers::writeUInt(os, myEdges.size());
    FileHelpers::writeByte(os, myMultipleReferenced);
    for (MSEdgeVector::const_iterator i = myEdges.begin(); i!=myEdges.end(); ++i) {
        FileHelpers::writeUInt(os, (*i)->getNumericalID());
    }
}


void
MSRoute::dict_loadState(BinaryInputDevice &bis)
{
    unsigned int noRoutes;
    bis >> noRoutes;
    while (noRoutes>0) {
        string id;
        bis >> id;
        unsigned int no;
        bis >> no;
        bool multipleReferenced;
        bis >> multipleReferenced;
        bool had = dictionary(id)!=0;
        MSEdgeVector edges;
        if (!had) {
            edges.reserve(no);
        }
        while (no>0) {
            unsigned int edgeID;
            bis >> edgeID;
            if (!had) {
                MSEdge *e = MSEdge::dictionary(edgeID);
                assert(e!=0);
                edges.push_back(e);
            }
            no--;
        }
        if (!had) {
            MSRoute *r = new MSRoute(id, edges, multipleReferenced);
            dictionary(id, r);
        }
        noRoutes--;
    }
}


unsigned
MSRoute::posInRoute(const MSRouteIterator &currentEdge) const
{
    return distance(myEdges.begin(), currentEdge);
}


void
MSRoute::clearLoadedState()
{
    std::vector<MSRoute*> toDel;
    for (RouteDict::iterator it = myDict.begin(); it!=myDict.end(); ++it) {
        if (!(*it).second->inFurtherUse()) {
            toDel.push_back((*it).second);
        }
    }
    for (std::vector<MSRoute*>::iterator i=toDel.begin(); i!=toDel.end(); ++i) {
        erase((*i)->getID());
    }
}



/****************************************************************************/

