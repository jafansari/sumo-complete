/****************************************************************************/
/// @file    NIVissimConnection.h
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
#ifndef NIVissimConnection_h
#define NIVissimConnection_h


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
#include <utils/common/IntVector.h>
#include "NIVissimExtendedEdgePoint.h"
#include <utils/geom/Position2D.h>
#include <utils/geom/AbstractPoly.h>
#include "NIVissimAbstractEdge.h"
#include "NIVissimClosedLanesVector.h"
#include "NIVissimBoundedClusterObject.h"


class NBEdgeCont;


class NIVissimConnection
            : public NIVissimBoundedClusterObject,
            public NIVissimAbstractEdge
{
public:
    enum Direction {
        NIVC_DIR_RIGHT,
        NIVC_DIR_LEFT,
        NIVC_DIR_ALL
    };

    NIVissimConnection(int id, const std::string &name,
                       const NIVissimExtendedEdgePoint &from_def,
                       const NIVissimExtendedEdgePoint &to_def,
                       const Position2DVector &geom,
                       Direction direction, SUMOReal dxnothalt, SUMOReal dxeinordnen,
                       SUMOReal zuschlag1, SUMOReal zuschlag2, SUMOReal seglength,
                       const IntVector &assignedVehicles,
                       const NIVissimClosedLanesVector &clv);
    virtual ~NIVissimConnection();
    void computeBounding();
    int getFromEdgeID() const;
    int getToEdgeID() const;
    SUMOReal getFromPosition() const;
    SUMOReal getToPosition() const;
    Position2D getFromGeomPosition() const;
    Position2D getToGeomPosition() const;
    void setNodeCluster(int nodeid);
    void unsetCluster();
    const Boundary &getBoundingBox() const;

    void buildGeom();

public:
    static bool dictionary(int id, const std::string &name,
                           const NIVissimExtendedEdgePoint &from_def,
                           const NIVissimExtendedEdgePoint &to_def,
                           const Position2DVector &geom,
                           Direction direction, SUMOReal dxnothalt, SUMOReal dxeinordnen,
                           SUMOReal zuschlag1, SUMOReal zuschlag2, SUMOReal seglength,
                           const IntVector &assignedVehicles,
                           const NIVissimClosedLanesVector &clv);
    static bool dictionary(int id, NIVissimConnection *o);
    static NIVissimConnection *dictionary(int id);
    static IntVector getWithin(const AbstractPoly &poly);
    static void buildNodeClusters();
    static IntVector getForEdge(int edgeid, bool omitNodeAssigned=true);
    static void dict_buildNBEdgeConnections(NBEdgeCont &ec);
    static void dict_assignToEdges();
    static int getMaxID();
    const IntVector &getFromLanes() const;
    const IntVector &getToLanes() const;
    /*    static void assignNodes();
        static void buildFurtherNodes();
        static IntVector getOutgoingForEdge(int edgeid);
        static IntVector getIncomingForEdge(int edgeid);*/

private:
    std::string myName;
    NIVissimExtendedEdgePoint myFromDef, myToDef;
    Direction myDirection;
    SUMOReal myDXNothalt, myDXEinordnen;
    SUMOReal myZuschlag1, myZuschlag2;
    IntVector myAssignedVehicles;
    NIVissimClosedLanesVector myClosedLanes;
private:
    typedef std::map<int, NIVissimConnection*> DictType;
    static DictType myDict;
    static int myMaxID;
};


#endif

/****************************************************************************/

