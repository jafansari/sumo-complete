#ifndef ROEdge_h
#define ROEdge_h
//---------------------------------------------------------------------------//
//                        ROEdge.h -
//  An edge the router may route through
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
// $Log$
// Revision 1.20  2006/04/11 11:03:18  dkrajzew
// all structures now return their id via getID()
//
// Revision 1.19  2006/01/26 08:42:50  dkrajzew
// made lanes and edges being aware to vehicle classes
//
// Revision 1.18  2006/01/16 10:46:24  dkrajzew
// some initial work on  the dfrouter
//
// Revision 1.17  2005/10/17 09:21:36  dkrajzew
// memory leaks removed
//
// Revision 1.16  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.15  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.14  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.13  2005/05/04 08:46:09  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.12  2004/11/23 10:25:51  dkrajzew
// debugging
//
// Revision 1.11  2004/07/02 09:39:41  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived
//  for an online-routing
//
// Revision 1.10  2004/04/14 13:53:50  roessel
// Changes and additions in order to implement supplementary-weights.
//
// Revision 1.9  2004/01/27 10:31:50  dkrajzew
// patched some linux-warnings
//
// Revision 1.8  2004/01/26 08:01:10  dkrajzew
// loaders and route-def types are now renamed in an senseful way;
//  further changes in order to make both new routers work; documentation added
//
// Revision 1.7  2003/11/11 08:04:46  dkrajzew
// avoiding emissions of vehicles on too short edges
//
// Revision 1.6  2003/09/17 10:14:27  dkrajzew
// handling of unset values patched
//
// Revision 1.5  2003/04/15 09:09:18  dkrajzew
// documentation added
//
// Revision 1.4  2003/04/09 15:39:11  dkrajzew
// router debugging & extension: no routing over sources, random routes added
//
// Revision 1.3  2003/02/07 10:45:07  dkrajzew
// updated
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
#include <map>
#include <vector>
#include <utils/router/FloatValueTimeLine.h>
#include <utils/common/SUMOVehicleClass.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class ROLane;
class ROVehicle;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class ROEdge
 * An edge.
 */
class ROEdge {
public:
    /**
     * @enum EdgeType
     * Possible types of edges
     */
    enum EdgeType {
        /// A normal edge
        ET_NORMAL,
        /// An edge where vehicles are emitted at (no vehicle may come from back)
        ET_SOURCE,
        /// An edge where vehicles disappear (no vehicle may leave this edge)
        ET_SINK
    };


    /// Constructor
    ROEdge(const std::string &id, int index);

    /// Desturctor
    virtual ~ROEdge();

    /// Adds a lane to the edge while loading
    virtual void addLane(ROLane *lane);

    void addWeight(SUMOReal value, SUMOTime timeBegin, SUMOTime timeEnd);

    /// Adds information about a connected edge
    virtual void addFollower(ROEdge *s);

    /// returns the information whether this edge is directly connected to the given
    bool isConnectedTo(ROEdge *e);

    /** @brief Returns the number of edges this edge is connected to
        (size of the list of reachable edges) */
    size_t getNoFollowing() const;

    /// Returns the edge at the given position from the list of reachable edges
    ROEdge *getFollower(size_t pos) const;

    /// retrieves the cost of this edge at the given time
    SUMOReal getCost(SUMOTime time) const;

    /// Retrieves the time a vehicle needs to pass this edge starting at the given time
    SUMOReal getDuration(SUMOTime time) const;

    /// Adds a connection, marking the effort to pas the connection (!!!)
    bool addConnection(ROEdge *to, SUMOReal effort);

    /// Returns the id of the edge
    const std::string &getID() const;

    /// Sets the type of te edge
    void setType(EdgeType type);

    /// Returns the type of the edge
    EdgeType getType() const;

    /// Returns the length of the edge
    SUMOReal getLength() const;

    /// Returns the index (numeric id) of the edge
    size_t getNumericalID() const;

    /// returns the effort for this edge only
    virtual SUMOReal getEffort(SUMOTime time) const;

    /// Takes pointers to FloatValueTimeLines and assigns them to the
    /// classes supplementary weights. You must provide all three
    /// FloatValueTimeLines and they must be valid objects. These
    /// objects will be deleted on deletion of this ROEdge. The flag
    /// hasSupplementaryWeights will be set to true and getMyEffort()
    /// will use this supplementary weights in subsequent calls.
    ///
    /// @param absolut Pointer to the absolut-FloatValueTimeLine.
    /// @param add Pointer to the add-FloatValueTimeLine.
    /// @param mult Pointer to the mult-FloatValueTimeLine.
    ///
    void setSupplementaryWeights( FloatValueTimeLine* absolut,
                                  FloatValueTimeLine* add,
                                  FloatValueTimeLine* mult );

     void clear();

   virtual ROLane *getLane(std::string name) ; //###################

static	bool dictionary(std::string id, ROEdge* ptr);

static ROEdge* dictionary2(std::string id);
////#########################  muss Private sein


		    /// definition of the static dictionary type
    typedef std::map<std::string, ROEdge* > DictType;

    /// Static dictionary to associate string-ids with objects.
    static DictType myDict;

		    /// definition of the static dictionary type
    typedef std::map<std::string, ROLane* > DictLane;

    /// Static dictionary to associate string-ids with objects.
     DictLane myDictLane;

	 ROLane *getLane(size_t index);

    SUMOReal getSpeed() const;

    int getLaneNo() const;

	bool prohibits(const ROVehicle * const vehicle) const;

protected:
    /// The id of the edge
    std::string _id;

    /// The maximum distance of this edge (including all lanes)
    SUMOReal _dist;

    /// The maximum speed allowed on this edge
    SUMOReal _speed;

    /// Container storing passing time varying over time for the edge
    FloatValueTimeLine _ownValueLine;

    /// "Absolut" supplementary weights.
    FloatValueTimeLine* _supplementaryWeightAbsolut;
    /// "Add" supplementary weights.
    FloatValueTimeLine* _supplementaryWeightAdd;
    /// "Multiplication" supplementary weights.
    FloatValueTimeLine* _supplementaryWeightMult;

    /// List of edges that may be approached from this edge
    std::vector<ROEdge*> myFollowingEdges;

    /// infomration whether the time line shall be used instead of the length value
    bool _usingTimeLine;

    /// The type of the edge
    EdgeType myType;

    /// The index (numeric id) of the edge
    size_t myIndex;

    /// Flag that indicates, if the supplementary weights have been
    /// set. Initially false.
    bool _hasSupplementaryWeights;

    /// Information whether the edge has reported missing weights
    //bool myHaveWarned;

    /// The length of the edge
    SUMOReal myLength;

	std::vector<ROLane*> myLanes;

	/// The list of allowed vehicle classes
	std::vector<SUMOVehicleClass> myAllowedClasses;

	/// The list of disallowed vehicle classes
	std::vector<SUMOVehicleClass> myDisAllowedClasses;


private:
    /// we made the copy constructor invalid
    ROEdge(const ROEdge &src);

    /// we made the assignment operator invalid
    ROEdge &operator=(const ROEdge &src);

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

