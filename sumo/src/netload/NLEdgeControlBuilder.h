#ifndef NLEdgeControlBuilder_h
#define NLEdgeControlBuilder_h
/***************************************************************************
                          NLEdgeControlBuilder.h
              Holds the edges while they are build
                             -------------------
    project              : SUMO
    begin                : Mon, 9 Jul 2001
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// $Log$
// Revision 1.11  2006/09/18 10:13:48  dkrajzew
// added vehicle class support to microsim
//
// Revision 1.10  2005/11/09 06:42:54  dkrajzew
// TLS-API: MSEdgeContinuations added
//
// Revision 1.9  2005/10/07 11:41:49  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/23 06:04:11  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 12:04:36  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/07/12 12:37:15  dkrajzew
// code style adapted
//
// Revision 1.5  2005/05/04 08:40:16  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; added the possibility to load lane shapes into the non-gui simulation
//
// Revision 1.4  2004/07/02 09:37:53  dkrajzew
// lanes now get a numerical id (for online-routing)
//
// Revision 1.3  2003/06/16 14:43:34  dkrajzew
// documentation added
//
// Revision 1.2  2003/02/07 11:18:56  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:36:48  dkrajzew
// moved from ROOT/sumo/netload to ROOT/src/netload; new format definition parseable in one step
//
// Revision 1.4  2002/06/11 14:39:24  dkrajzew
// windows eol removed
//
// Revision 1.3  2002/06/11 13:44:33  dkrajzew
// Windows eol removed
//
// Revision 1.2  2002/06/07 14:39:58  dkrajzew
// errors occured while building larger nets and adaption of new
//  netconverting methods debugged
//
// Revision 1.1.1.1  2002/04/08 07:21:24  traffic
// new project name
//
// Revision 2.0  2002/02/14 14:43:21  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.4  2002/02/13 15:40:41  croessel
// Merge between SourgeForgeRelease and tesseraCVS.
//
// Revision 1.1  2001/12/06 13:36:05  traffic
// moved from netbuild
//
// Revision 1.4  2001/08/16 12:53:59  traffic
// further exception handling (now validated) and new comments
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

#include <string>
#include <vector>
#include <microsim/MSEdge.h>
#include <utils/geom/Position2DVector.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSEdgeControl;
class MSLane;
class MSNet;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class NLEdgeControlBuilder
 * This class is the container for MSEdge-instances while they are build.
 * As instances of the MSEdge-class contain references to other instances of
 * this class which may not yet be known at their generation, they are
 * prebuild first and initialised with their correct values in a second step.
 *
 * While building an intialisation of the MSEdge, the value are stored in a
 * preallocated list to avoid memory fraction. For the same reason, the list
 * of edges, later splitted into two lists, one containing single-lane-edges
 * and one containing multi-lane-edges, is preallocated to the size that was
 * previously computed by counting the edges in the first parser step.
 * As a result, the build MSEdgeControlBuilder is returned.
 */
class NLEdgeControlBuilder {
public:
    /// definition of the used storage for edges
    typedef std::vector<MSEdge*> EdgeCont;

public:
    /** @brief standard constructor;
        the parameter is a hint for the maximal number of lanes inside an edge */
    NLEdgeControlBuilder(unsigned int storageSize=10);

    /// Destructor
    virtual ~NLEdgeControlBuilder();

    /** Prepares the builder for the building of the specified number of
        edges (preallocates ressources) */
    void prepare(unsigned int no);

    /** @brief Adds an edge with the given id to the list of edges;
        This method throws an XMLIdAlreadyUsedException when the id was
        already used for another edge */
    virtual MSEdge *addEdge(const std::string &id);

    /// chooses the previously added edge as the current edge
    void chooseEdge(const std::string &id,
        MSEdge::EdgeBasicFunction function);

    /** @brief Adds a lane to the current edge;
        This method throws an XMLDepartLaneDuplicationException when the
        lane is marked to be the depart lane and another so marked lane
        was added before */
    virtual MSLane *addLane(/*MSNet &net, */const std::string &id,
        SUMOReal maxSpeed, SUMOReal length, bool isDepart,
        const Position2DVector &shape, const std::string &vclasses);

    /// closes (ends) the addition of lanes to the current edge
    void closeLanes();

    /** Begins the specification of lanes that may be used to reach the given
        edge from the current edge */
    void openAllowedEdge(MSEdge *edge);

    /** @brief Adds a lane that may be used to reach the edge previously specified by "openAllowedEdge"
        This method throws an XMLInvalidChildException when the lane is
        not belonging to the current edge */
    void addAllowed(MSLane *lane);

    /// closes the specification of lanes that may be used to reach an edge
    void closeAllowedEdge();

    /** @brief Closes the building of an edge;
        The edge is completely described by now and may not be opened again */
    virtual MSEdge *closeEdge();

    /// builds the MSEdgeControl-class which holds all edges
    MSEdgeControl *build();

    MSEdge *getActiveEdge() const;

    void parseVehicleClasses(const std::string &allowedS,
        std::vector<SUMOVehicleClass> &allowed,
        std::vector<SUMOVehicleClass> &disallowed);


protected:
    /** storage for edges; to allow the splitting of edges after their number
        is known, they are hold inside this vector and laterly moved into two
        vectors, one for single-lane-edges and one for multi-lane-edges
        respectively */
    EdgeCont                  *m_pEdges;

    /// pointer to the currently chosen edge
    MSEdge                    *m_pActiveEdge;

    /// pointer to a temporary lane storage
    MSEdge::LaneCont          *m_pLaneStorage;

    /// list of the lanes that belong to the current edge
    MSEdge::LaneCont          *m_pLanes;

    /// pointer to the following edge the structure is currently working on
    MSEdge                    *m_pCurrentDestination;

    /// connection to following edges from the current edge
    MSEdge::AllowedLanesCont  *m_pAllowedLanes;

    /// pointer to the depart lane
    MSLane                    *m_pDepartLane;

    /// number of single-lane-edges
    unsigned int              m_iNoSingle;

    /// number of multi-lane-edges
    unsigned int              m_iNoMulti;

    /// the function of the current edge
    MSEdge::EdgeBasicFunction m_Function;

    /// A running numer for lane numbering
    size_t myCurrentNumericalLaneID;

    /// A running numer for edge numbering
    size_t myCurrentNumericalEdgeID;

private:
    /** invalidated copy constructor */
    NLEdgeControlBuilder(const NLEdgeControlBuilder &s);

    /** invalidated assignment operator */
    NLEdgeControlBuilder &operator=(const NLEdgeControlBuilder &s);

};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
