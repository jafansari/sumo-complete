/****************************************************************************/
/// @file    NIVissimNodeDef_Edges.cpp
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



#include <string>
#include <map>
#include <algorithm>
#include <cassert>
#include <utils/geom/Boundary.h>
#include "NIVissimNodeParticipatingEdgeVector.h"
#include "NIVissimNodeDef.h"
#include "NIVissimEdge.h"
#include "NIVissimNodeDef_Edges.h"
#include "NIVissimDisturbance.h"
#include "NIVissimConnection.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS
// ===========================================================================
// used namespaces
// ===========================================================================

using namespace std;

NIVissimNodeDef_Edges::NIVissimNodeDef_Edges(int id,
        const std::string &name, const NIVissimNodeParticipatingEdgeVector &edges)
        : NIVissimNodeDef(id, name), myEdges(edges)
{}


NIVissimNodeDef_Edges::~NIVissimNodeDef_Edges()
{
    for (NIVissimNodeParticipatingEdgeVector::iterator i=myEdges.begin(); i!=myEdges.end(); i++) {
        delete(*i);
    }
    myEdges.clear();
}


bool
NIVissimNodeDef_Edges::dictionary(int id, const std::string &name,
                                  const NIVissimNodeParticipatingEdgeVector &edges)
{
    NIVissimNodeDef_Edges *o = new NIVissimNodeDef_Edges(id, name, edges);
    if (!NIVissimNodeDef::dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}


/*
void
NIVissimNodeDef_Edges::computeBounding()
{
    assert(myBoundary==0);
    Boundary *boundary = new Boundary();
    for(NIVissimNodeParticipatingEdgeVector::const_iterator i=myEdges.begin(); i!=myEdges.end(); i++) {
        NIVissimNodeParticipatingEdge *edge = *i;
        NIVissimConnection *c = NIVissimConnection::dictionary(edge->getID());
        NIVissimEdge *e = NIVissimEdge::dictionary(edge->getID());
        if(c!=0) {
            // both connected edges should be a part of the junction? !!!
            boundary->add(c->getFromGeomPosition());
            boundary->add(c->getToGeomPosition());
        }
        if(e!=0) {
            boundary->add(e->getGeomPosition(edge->getFromPos()));
            boundary->add(e->getGeomPosition(edge->getToPos()));
        }
    }
    myBoundary = boundary;
    assert(myBoundary!=0&&myBoundary->xmax()>=myBoundary->xmin());
}
*/

void
NIVissimNodeDef_Edges::searchAndSetConnections()
{
    IntVector connections;
    IntVector edges;
    Boundary boundary;
    for (NIVissimNodeParticipatingEdgeVector::const_iterator i=myEdges.begin(); i!=myEdges.end(); i++) {
        NIVissimNodeParticipatingEdge *edge = *i;
        NIVissimConnection *c =
            NIVissimConnection::dictionary(edge->getID());
        NIVissimEdge *e =
            NIVissimEdge::dictionary(edge->getID());
        if (c!=0) {
            connections.push_back(edge->getID());
            boundary.add(c->getFromGeomPosition());
            boundary.add(c->getToGeomPosition());
            c->setNodeCluster(myID);
        }
        if (e!=0) {
            edges.push_back(edge->getID());
            boundary.add(e->getGeomPosition(edge->getFromPos()));
            boundary.add(e->getGeomPosition(edge->getToPos()));
        }
    }
    NIVissimConnectionCluster *c =
        new NIVissimConnectionCluster(connections, boundary, myID, edges);
    for (IntVector::iterator j=edges.begin(); j!=edges.end(); j++) {
        NIVissimEdge *edge = NIVissimEdge::dictionary(*j);
        edge->myConnectionClusters.push_back(c);
    }
}



SUMOReal
NIVissimNodeDef_Edges::getEdgePosition(int edgeid) const
{
    for (NIVissimNodeParticipatingEdgeVector::const_iterator i=myEdges.begin(); i!=myEdges.end(); i++) {
        NIVissimNodeParticipatingEdge *edge = *i;
        if (edge->getID()==edgeid) {
            return (edge->getFromPos() + edge->getToPos()) / (SUMOReal) 2.0;
        }
    }
    return -1;
}



/****************************************************************************/

