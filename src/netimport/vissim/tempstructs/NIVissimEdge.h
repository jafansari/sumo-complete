/****************************************************************************/
/// @file    NIVissimEdge.h
/// @author  Daniel Krajzewicz
/// @date    End of 2002
/// @version $Id$
///
// An edge imported from Vissim together for a container for
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
#ifndef NIVissimEdge_h
#define NIVissimEdge_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "NIVissimConnectionCluster.h"
#include <vector>
#include <string>
#include <map>
#include <netbuild/NBEdge.h>
#include <utils/geom/Position2DVector.h>
#include <utils/common/VectorHelper.h>
#include "NIVissimAbstractEdge.h"
#include "NIVissimClosedLanesVector.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NIVissimDistrictConnection;
class NBDistribution;
class NBDistrictCont;


// ===========================================================================
// class definitions
// ===========================================================================
class NIVissimEdge
            : public NIVissimAbstractEdge
{
public:
    /// Constructor
    NIVissimEdge(int id, const std::string &name,
                 const std::string &type, int noLanes, SUMOReal zuschlag1,
                 SUMOReal zuschlag2, SUMOReal length,
                 const Position2DVector &geom,
                 const NIVissimClosedLanesVector &clv);

    /// Destructor
    ~NIVissimEdge();

    void setNodeCluster(int nodeid);
    void buildGeom();

    /// Adds a connection where this edge is the destination
    void addIncomingConnection(int id);

    /// Adds a connection where this edge is the source
    void addOutgoingConnection(int id);

    /** @brief Returns the node at the given position
        As this may be ambigous, a second node not to return may be supplied */
    NBNode *getNodeAt(const Position2D &p, NBNode *other=0);

    /** Returns the begin position of the edge */
    Position2D getBegin2D() const;

    /// Returns the end position of the edge
    Position2D getEnd2D() const;

    /// Returns the length of the node
    SUMOReal getLength() const;

    void checkDistrictConnectionExistanceAt(SUMOReal pos);

    void mergedInto(NIVissimConnectionCluster *old,
                    NIVissimConnectionCluster *act);

    void removeFromConnectionCluster(NIVissimConnectionCluster *c);
    void addToConnectionCluster(NIVissimConnectionCluster *c);
    void setSpeed(size_t lane, int speedDist);
    void addToTreatAsSame(NIVissimEdge *e);

    NIVissimConnection* getConnectionTo(NIVissimEdge *e);
    const std::vector<NIVissimEdge*> &getToTreatAsSame() const;

    friend class NIVissimNodeDef_Edges;
    friend class NIVissimNodeDef_Poly;

public:
    /** @brief Adds the described item to the dictionary
        Builds the edge first */
    static bool dictionary(int id, const std::string &name,
                           const std::string &type, int noLanes, SUMOReal zuschlag1,
                           SUMOReal zuschlag2, SUMOReal length,
                           const Position2DVector &geom,
                           const NIVissimClosedLanesVector &clv);

    /// Adds the edge to the dictionary
    static bool dictionary(int id, NIVissimEdge *o);

    /// Returns the named edge from the dictionary
    static NIVissimEdge *dictionary(int id);

    static void buildConnectionClusters();

    /// Builds NBEdges from the VissimEdges within the dictionary
    static void dict_buildNBEdges(NBDistrictCont &dc, NBNodeCont &nc,
                                  NBEdgeCont &ec, SUMOReal offset);

    static void dict_propagateSpeeds(/* NBDistribution &dc */);

    static void dict_checkEdges2Join();

//    static void replaceSpeed(int id, int lane, SUMOReal speed);

private:
    /// The definition for a container for connection clusters
    typedef std::vector<NIVissimConnectionCluster*> ConnectionClusters;

private:
    /// Builds the NBEdge from this VissimEdge
    void buildNBEdge(NBDistrictCont &dc, NBNodeCont &nc,
                     NBEdgeCont &ec, SUMOReal offset);

    /// Returns the origin node
    std::pair<NIVissimConnectionCluster*, NBNode*>
    getFromNode(NBNodeCont &nc, ConnectionClusters &clusters);

    /// Returns the destination node
    std::pair<NIVissimConnectionCluster*, NBNode*>
    getToNode(NBNodeCont &nc, ConnectionClusters &clusters);

    /// Tries to resolve the problem that the same node has been returned as origin and destination node
    std::pair<NBNode*, NBNode*> resolveSameNode(NBNodeCont &nc,
            SUMOReal offset, NBNode *prevFrom, NBNode *prevTo);

//    SUMOReal recheckSpeedPatches();

    std::vector<NIVissimConnection*> getOutgoingConnected(int lane) const;

    void propagateSpeed(/* NBDistribution &dc */SUMOReal speed, IntVector forLanes);


    void setDistrictSpeed(/* NBDistribution &dc */);
    SUMOReal getRealSpeed(/* NBDistribution &dc */int distNo);
    void checkUnconnectedLaneSpeeds(/* NBDistribution &dc */);
    void propagateOwn(/* NBDistribution &dc */);



private:
    static NBNode *getNodeSecure(int nodeid, const Position2D &pos,
                                 const std::string &possibleName);

    std::pair<NBNode*, NBNode*>
    remapOneOfNodes(NBNodeCont &nc,
                    NIVissimDistrictConnection *d,
                    NBNode *fromNode, NBNode *toNode);

private:
    /**
     * Sorts connections the edge participates in by their position along
     * the given edge
     */
    class connection_position_sorter
    {
    public:
        /// constructor
        explicit connection_position_sorter(int edgeid);

        /// comparing operation
        int operator()(int c1id, int c2id) const;

    private:
        /// The id of the edge
        int myEdgeID;

    };


    /**
     * Sorts connection clusters the edge participates in by their
     * position along the given edge
     */
    class connection_cluster_position_sorter
    {
    public:
        /// constructor
        explicit connection_cluster_position_sorter(int edgeid);

        /// comparing operation
        int operator()(NIVissimConnectionCluster *cc1,
                       NIVissimConnectionCluster *cc2) const;

    private:
        /// The id of the edge
        int myEdgeID;

    };

private:
    /// The name of the edge
    std::string myName;

    /// The tape of the edge
    std::string myType;

    /// The number of lanes the edge has
    size_t myNoLanes;

    /// Additional load values for this edge
    SUMOReal myZuschlag1, myZuschlag2;

    /// List of lanes closed on this edge
    NIVissimClosedLanesVector myClosedLanes;

    /// List of connection clusters along this edge
    ConnectionClusters myConnectionClusters;

    /// List of connections incoming to this edge
    IntVector myIncomingConnections;

    /// List of connections outgoing from this edge
    IntVector myOutgoingConnections;

    /// List of districts referencing this edge
//    IntVector myDistricts;

    DoubleVector myDistrictConnections;

    IntVector myPatchedSpeeds;

//    SUMOReal mySpeed;

    std::vector<SUMOReal> myLaneSpeeds;

    std::vector<NIVissimEdge*> myToTreatAsSame;

private:
    /// Definition of the dictionary type
    typedef std::map<int, NIVissimEdge*> DictType;

    /// The dictionary
    static DictType myDict;

    /** @brief The current maximum id;
        needed for further id assignment */
    static int myMaxID;

};


#endif

/****************************************************************************/

