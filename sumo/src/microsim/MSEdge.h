#ifndef MSEdge_H
#define MSEdge_H
/***************************************************************************
                          MSEdge.h  -  Provides routing. Superior to Lanes.
                             -------------------
    begin                : Mon, 12 Mar 2001
    copyright            : (C) 2001 by ZAIK http://www.zaik.uni-koeln.de/AFS
    author               : Christian Roessel
    email                : roessel@zpr.uni-koeln.de
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
// Revision 1.23  2006/04/11 10:59:06  dkrajzew
// all structures now return their id via getID()
//
// Revision 1.22  2006/01/26 08:30:29  dkrajzew
// patched MSEdge in order to work with a generic router
//
// Revision 1.21  2005/11/09 06:39:38  dkrajzew
// usage of internal lanes is now optional at building
//
// Revision 1.20  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.19  2005/09/22 13:45:50  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.18  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.17  2005/07/13 10:22:46  dkrajzew
// debugging
//
// Revision 1.16  2005/07/12 12:23:07  dkrajzew
// debugging vehicle emission on source edges
//
// Revision 1.15  2005/05/04 08:23:52  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; speed-ups by checked emission and avoiding looping over all edges
//
// Revision 1.14  2005/02/01 10:10:40  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.13  2004/11/23 10:20:09  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.12  2004/08/02 12:08:39  dkrajzew
// raw-output extracted; output device handling rechecked
//
// Revision 1.11  2004/07/02 09:55:13  dkrajzew
// MeanData refactored (moved to microsim/output)
//
// Revision 1.10  2004/02/16 14:19:51  dkrajzew
// getLane should be const
//
// Revision 1.9  2003/12/04 13:30:41  dkrajzew
// work on internal lanes
//
// Revision 1.8  2003/09/05 15:07:53  dkrajzew
// tried to improve the building/deletion usage
//
// Revision 1.7  2003/07/16 15:28:00  dkrajzew
// MSEmitControl now only simulates lanes which do have vehicles; the edges
//  do not go through the lanes, the EdgeControl does
//
// Revision 1.6  2003/06/04 16:12:05  roessel
// Added methods getEdgeVector and getLanes needed by MSTravelcostDetector.
//
// Revision 1.5  2003/04/15 09:09:14  dkrajzew
// documentation added
//
// Revision 1.4  2003/02/07 10:41:50  dkrajzew
// updated
//
// Revision 1.3  2002/10/17 10:41:12  dkrajzew
// retrival of the id added
//
// Revision 1.2  2002/10/16 16:40:35  dkrajzew
// usage of MSPerson removed; will be reimplemented later
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.3  2002/09/25 17:14:42  roessel
// MeanData calculation and output implemented.
//
// Revision 1.2  2002/04/24 13:06:47  croessel
// Changed signature of void detectCollisions() to void detectCollisions(
// MSNet::Time )
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.3  2002/03/20 11:11:08  croessel
// Splitted friend from class-declaration.
//
// Revision 2.2  2002/03/13 16:56:34  croessel
// Changed the simpleOutput to XMLOutput by introducing nested classes
// XMLOut. Output is now indented.
//
// Revision 2.1  2002/02/27 13:14:05  croessel
// Prefixed ostream with "std::".
//
// Revision 2.0  2002/02/14 14:43:13  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.14  2002/02/05 13:51:51  croessel
// GPL-Notice included.
// In *.cpp files also config.h included.
//
// Revision 1.13  2002/02/05 11:42:35  croessel
// Lane-change implemented.
//
// Revision 1.12  2001/12/19 16:51:01  croessel
// Replaced using namespace std with std::
//
// Revision 1.11  2001/12/06 13:08:28  traffic
// minor change
//
// Revision 1.10  2001/11/15 17:12:13  croessel
// Outcommented the inclusion of the inline *.iC files. Currently not
// needed.
//
// Revision 1.9  2001/11/14 11:45:53  croessel
// Resolved conflicts which appeared during suffix-change and
// CR-line-end commits.
//
// Revision 1.8  2001/11/14 10:49:07  croessel
// CR-line-end removed.
//
// Revision 1.7  2001/10/23 09:29:25  traffic
// parser bugs removed
//
// Revision 1.5  2001/09/06 15:29:55  croessel
// Added operator<< to class MSEdge for simple text output.
//
// Revision 1.4  2001/07/18 09:41:02  croessel
// Added public method nLanes() that returns the edge's number of lanes.
//
// Revision 1.3  2001/07/16 12:55:46  croessel
// Changed id type from unsigned int to string. Added string-pointer
//  dictionaries and dictionary methods.
//
// Revision 1.2  2001/07/13 16:09:17  traffic
// Just a test, changed id-type from unsigned int to string.
//
// Revision 1.1.1.1  2001/07/11 15:51:13  traffic
// new start
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

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "MSLinkCont.h"
#include <utils/common/SUMOTime.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSLane;
class MSLaneChanger;
class OutputDevice;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class MSEdge
 * A single connection between two junctions. As there is no certain
 * relationship over junctions, the word "street" or "road" may be ambigous.
 * Holds lanes which are reponsible for vehicle movements.
 */
class MSEdge
{
public:
    /**
     * For different purposes, it is necessary to know whether the edge
     * is a normal street or just a sink or a source
     * This information is represented by values from this enumeration
     */
    enum EdgeBasicFunction {
        /// the purpose of the edge is not known
        EDGEFUNCTION_UNKNOWN = -1,
        /// the edge is a normal street
        EDGEFUNCTION_NORMAL = 0,
        /// the edge is only used for vehicle emission (begin of trips)
        EDGEFUNCTION_SOURCE = 1,
        /// the edge is only used for vehicle deletion (end of trips)
        EDGEFUNCTION_SINK = 2,
        EDGEFUNCTION_INTERNAL = 3
    };

public:
    /// for access to the dictionary
    friend class GUIEdgeGrid;
    friend class GUIGrid;
    /// for data collection
    friend class MSMeanData_Edge;


    /// output operator for XML-mean-data output
    friend OutputDevice& operator<<( OutputDevice& os,
                                     const MSMeanData_Edge& obj );


    /// Constructor.
    MSEdge( const std::string &id, size_t numericalID );

    /// Container for lanes.
    typedef std::vector< MSLane* > LaneCont;

    /** Associative container with suceeding edges (keys) and allowed
        lanes to reach these edges. */
    typedef std::map< const MSEdge*, LaneCont* > AllowedLanesCont;

    /// Destructor.
    virtual ~MSEdge();

    /// Initialize the edge.
    virtual void initialize(
        AllowedLanesCont* allowed, MSLane* departLane, LaneCont* lanes,
        EdgeBasicFunction function);

    /* @brief Ask edge's lanes about collisions.
        Shouldn't be neccessary if model is implemented correctly. */
//    void detectCollisions( SUMOTime timestep );

    /** @brief Get the allowed lanes to reach the destination-edge.
        If there is no such edge, get 0. Then you are on the wrong edge. */
    const LaneCont* allowedLanes( const MSEdge& destination ) const;

    /** Returns the left-lane of lane if there is one, 0 otherwise. */
    MSLane* leftLane( const MSLane* lane ) const;

    /** @brief Inserts edge into the static dictionary
        Returns true if the key id isn't already in the dictionary. Otherwise
        returns false. */
    static bool dictionary( std::string id, MSEdge* edge );

    /** Returns the MSEdge associated to the key id if exists, otherwise
     * returns 0. */
    static MSEdge* dictionary( std::string id );

    /** Returns the MSEdge associated to the key id if exists, otherwise
     * returns 0. */
    static MSEdge* dictionary( size_t index );

    static size_t dictSize();

    /** Clears the dictionary */
    static void clear();

    /** Returns the edge's number of lanes. */
    unsigned int nLanes() const;

    /// outputs the id of the edge
    friend std::ostream& operator<<( std::ostream& os, const MSEdge& edge );

    /** Let the edge's vehicles try to change their lanes. */
    virtual void changeLanes();

    /** returns the id of the edge */
    const std::string &getID() const;

    /** @brief Returns the edge type
        Returns information whether the edge is a sink, a source or a
        normal street; see EdgeBasicFunction */
    EdgeBasicFunction getPurpose() const;

    /// returns the information whether the edge is a source
    virtual bool isSource() const;

    /// emits a vehicle on an appropriate lane
    virtual bool emit(MSVehicle &v, SUMOTime time) const;

    static std::vector< MSEdge* > getEdgeVector( void );

    LaneCont* getLanes( void ) const; // !!! not the container itself!

#ifdef HAVE_INTERNAL_LANES
    const MSEdge *getInternalFollowingEdge(MSEdge *followerAfterInternal) const;
#endif

    SUMOTime getLastFailedEmissionTime() const;

    void setLastFailedEmissionTime(SUMOTime time) const;

    std::vector<MSEdge*> getFollowingEdges() const;

    std::vector<MSEdge*> getIncomingEdges() const;

    const std::string &getID() { return myID; }

    SUMOReal getEffort(SUMOTime time) const;

    size_t getNumericalID() const { return myNumericalID; }

    size_t getNoFollowing() const { return myAllowed->size(); }

    const MSEdge *getFollower(size_t num) const {
        AllowedLanesCont::const_iterator i = myAllowed->begin();
        while(num!=0) {
            i++;
            num--;
        }
        return (*i).first;
    }

	virtual bool prohibits(const MSVehicle *veh) const { return false; } // !!!

protected:
    /// Unique ID.
    std::string myID;

    /** @brief Container for the edge's lane.
        Should be sorted: (right-hand-traffic) the more left the lane, the
        higher the
        container-index. */
    LaneCont* myLanes;

    /** Associative container for destination-edge/allowed-lanes
        matching. */
    AllowedLanesCont* myAllowed;

    /** @brief Lane from which vehicles will depart.
        Usually the rightmost,
        except for those countries which drive on the _wrong_
        side. */
    MSLane* myDepartLane;

    /** This member will do the lane-change. */
    MSLaneChanger* myLaneChanger;

    /// the purpose of the edge
    EdgeBasicFunction _function;

    /// definition of the static dictionary type
    typedef std::map< std::string, MSEdge* > DictType;

    /// Static dictionary to associate string-ids with objects.
    static DictType myDict;
    static std::vector<MSEdge*> myEdges;

    mutable SUMOTime myLastFailedEmissionTime;

    size_t myNumericalID;

private:
    /// Default constructor.
    MSEdge();

    /// Copy constructor.
    MSEdge( const MSEdge& );

    /// Assignment operator.
    MSEdge& operator=(const MSEdge&);

};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
