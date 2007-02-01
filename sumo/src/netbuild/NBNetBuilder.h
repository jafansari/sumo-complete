/****************************************************************************/
/// @file    NBNetBuilder.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 29.04.2005
/// @version $Id: $
///
//
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
#ifndef NBNetBuilder_h
#define NBNetBuilder_h
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
#include <iostream>
#include "NBEdgeCont.h"
#include "NBTypeCont.h"
#include "nodes/NBNodeCont.h"
#include "NBTrafficLightLogicCont.h"
#include "NBJunctionLogicCont.h"
#include "NBDistrictCont.h"

class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
class NBNetBuilder
{
public:
    /// Constructor
    NBNetBuilder();

    /// Destructor
    ~NBNetBuilder();

    void buildLoaded();

    static void insertNetBuildOptions(OptionsCont &oc);

    void preCheckOptions(OptionsCont &oc);

    /**
     * @brief computes the structures
     * the order of the computation steps is not variable!!!
     */
    void compute(OptionsCont &oc);

    /** saves the net */
    bool save(std::ostream &os, OptionsCont &oc);

    bool netBuild() const;

    NBEdgeCont &getEdgeCont();
    NBNodeCont &getNodeCont();
    NBTypeCont &getTypeCont();
    NBTrafficLightLogicCont &getTLLogicCont();
    NBJunctionLogicCont &getJunctionLogicCont();
    NBDistrictCont &getDistrictCont();


protected:

    void inform(int &step, const std::string &about);


    /** removes dummy edges from junctions */
    bool removeDummyEdges(int &step);

    /** joins edges which connect the same nodes */
    bool joinEdges(int &step);

    bool removeUnwishedNodes(int &step, OptionsCont &oc);
    bool removeUnwishedEdges(int &step, OptionsCont &oc);

    bool guessRamps(int &step, OptionsCont &oc);
    bool guessTLs(int &step, OptionsCont &oc);

    bool splitGeometry(int &step, OptionsCont &oc);

    /** computes the turning direction for each edge */
    bool computeTurningDirections(int &step);

    /** sorts the edges of a node */
    bool sortNodesEdges(int &step);

    /** sets the node positions in a way that nodes are lying at zero */
    bool normaliseNodePositions(int &step);

    /** computes edges 2 edges - relationships
        (step1: computation of approached edges) */
    bool computeEdge2Edges(int &step);

    /** computes edges 2 edges - relationships
        (step2: computation of which lanes approach the edges) */
    bool computeLanes2Edges(int &step);

    /** computes edges 2 edges - relationships
        (step3: division of lanes to approached edges) */
    bool computeLanes2Lanes(int &step);

    /** rechecks whether all lanes have a following lane/edge */
    bool recheckLanes(int &step);

    void initJoinedEdgesInformation();

    bool computeNodeShapes(int &step, OptionsCont &oc);
    bool computeEdgeShapes(int &step);

    bool setTLControllingInformation(int &step);

    /** appends the turnarounds */
    bool appendTurnarounds(int &step, OptionsCont &oc);

    /** computes nodes' logics */
    bool computeLogic(int &step, OptionsCont &oc);

    /** computes nodes' tl-logics */
    bool computeTLLogic(int &step, OptionsCont &oc);

    bool reshiftRotateNet(int &step, OptionsCont &oc);

    void checkPrint(OptionsCont &oc) ;

    bool saveMap(const std::string &path);

protected:
    bool myHaveBuildNet;

    NBEdgeCont myEdgeCont;
    NBNodeCont myNodeCont;
    NBTypeCont myTypeCont;
    NBTrafficLightLogicCont myTLLCont;
    NBJunctionLogicCont myJunctionLogicCont;
    NBDistrictCont myDistrictCont;

};


#endif

/****************************************************************************/

