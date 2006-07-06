#ifndef MSLane_H
#define MSLane_H
/***************************************************************************
                          MSLane.h  -  The place where Vehicles
                          operate.
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
// Revision 1.40  2006/07/06 07:33:22  dkrajzew
// rertrieval-methods have the "get" prependix; EmitControl has no dictionary; MSVehicle is completely scheduled by MSVehicleControl; new lanechanging algorithm
//
// Revision 1.39  2006/05/15 05:50:40  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.39  2006/05/08 10:59:34  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.38  2006/04/05 05:27:34  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.37  2006/03/28 06:20:43  dkrajzew
// removed the unneded slow lane changer
//
// Revision 1.36  2006/03/17 09:01:12  dkrajzew
// .icc-files removed
//
// Revision 1.35  2006/03/08 13:12:29  dkrajzew
// real density visualization added (slow, unfinished)
//
// Revision 1.34  2006/01/09 11:55:52  dkrajzew
// shape retrival added
//
// Revision 1.33  2005/11/09 06:40:05  dkrajzew
// removed unneeded stuff
//
// Revision 1.32  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.31  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.30  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.29  2005/07/12 12:24:17  dkrajzew
// further work on mean data usage
//
// Revision 1.28  2005/05/04 07:55:28  dkrajzew
// added the possibility to load lane geometries into the non-gui simulation; simulation speedup due to avoiding multiplication with 1;
//
// Revision 1.27  2005/02/17 10:33:37  dkrajzew
// code beautifying;
// Linux building patched;
// warnings removed;
// new configuration usage within guisim
//
// Revision 1.26  2005/02/01 10:10:41  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.25  2004/12/16 12:25:26  dkrajzew
// started a better vss handling
//
// Revision 1.24  2004/11/23 10:20:09  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.23  2004/08/02 12:14:29  dkrajzew
// raw-output extracted; debugging of collision handling
//
// Revision 1.22  2004/07/02 09:58:08  dkrajzew
// MeanData refactored (moved to microsim/output); numerical id for online
//  routing added
//
// Revision 1.21  2004/03/19 13:09:40  dkrajzew
// debugging
//
// Revision 1.20  2003/12/12 12:37:42  dkrajzew
// proper usage of lane states applied; scheduling of vehicles into the
//  beamer on push failures added
//
// Revision 1.19  2003/12/04 13:30:41  dkrajzew
// work on internal lanes
//
// Revision 1.18  2003/11/20 14:43:45  dkrajzew
// push() debugged; dead code removed
//
// Revision 1.17  2003/10/20 07:59:43  dkrajzew
// grid lock dissolving by vehicle teleportation added
//
// Revision 1.16  2003/09/05 15:10:30  dkrajzew
// first tries for an implementation of aggregated views
//
// Revision 1.15  2003/08/20 11:44:11  dkrajzew
// min and max-functions moved to an own definition file
//
// Revision 1.14  2003/07/16 15:28:00  dkrajzew
// MSEmitControl now only simulates lanes which do have vehicles;
//  the edges do not go through the lanes, the EdgeControl does
//
// Revision 1.13  2003/07/07 08:18:43  dkrajzew
// due to an ugly inheritance between lanes, sourcelanes and their
//  gui-versions, a method for the retrival of a GUILaneWrapper had to be
//  added; we should redesign it in the future
//
// Revision 1.12  2003/06/05 16:02:55  dkrajzew
// min and max-methods added (MSVC++ does not know them
//
// Revision 1.11  2003/05/21 16:20:44  dkrajzew
// further work detectors
//
// Revision 1.10  2003/05/21 15:15:41  dkrajzew
// yellow lights implemented (vehicle movements debugged
//
// Revision 1.9  2003/05/20 09:31:46  dkrajzew
// emission debugged; movement model reimplemented (seems ok);
//  detector output debugged; setting and retrieval of some parameter added
//
// Revision 1.8  2003/04/16 10:05:05  dkrajzew
// uah, debugging
//
// Revision 1.7  2003/04/14 08:32:58  dkrajzew
// some further bugs removed
//
// Revision 1.6  2003/03/20 16:21:12  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.5  2003/03/03 14:56:20  dkrajzew
// some debugging; new detector types added; actuated traffic lights added
//
// Revision 1.4  2003/02/07 10:41:51  dkrajzew
// updated
//
// Revision 1.3  2002/10/17 10:43:59  dkrajzew
// MSLaneSpeedTrigger is now friend of MSLane
//
// Revision 1.2  2002/10/16 16:43:48  dkrajzew
// regard of artifactsarising from traffic lights implemented; debugged
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.8  2002/09/25 17:14:42  roessel
// MeanData calculation and output implemented.
//
// Revision 1.7  2002/07/31 17:33:00  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.8  2002/07/23 16:36:25  croessel
// Added method isEmissionSuccess( MSVehicle* ).
//
// Revision 1.7  2002/07/03 15:45:58  croessel
// Signature of findNeigh changed.
//
// Revision 1.6  2002/05/29 17:06:03  croessel
// Inlined some methods. See the .icc files.
//
// Revision 1.5  2002/04/24 13:06:47  croessel
// Changed signature of void detectCollisions() to void detectCollisions(
// MSNet::Time )
//
// Revision 1.4  2002/04/11 15:25:56  croessel
// Changed SUMOReal to SUMOReal.
//
// Revision 1.3  2002/04/11 12:32:07  croessel
// Added new lookForwardState "URGENT_LANECHANGE_WISH" for vehicles that
// may drive beyond the lane but are not on a lane that is linked to
// their next route-edge. A second succLink method, named succLinkSec was
// needed.
//
// Revision 1.2  2002/04/10 16:17:00  croessel
// Added friend detectors.
// Added public id() member-function.
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.7  2002/03/20 15:58:32  croessel
// Return to previous revision.
//
// Revision 2.5  2002/03/20 11:11:09  croessel
// Splitted friend from class-declaration.
//
// Revision 2.4  2002/03/13 17:40:59  croessel
// Calculation of rearPos in setLookForwardState() fixed by introducing
// the new member myLastVeh in addition to myLastVehState. We need both,
// the state and the length of the vehicle.
//
// Revision 2.3  2002/03/13 16:56:35  croessel
// Changed the simpleOutput to XMLOutput by introducing nested classes
// XMLOut. Output is now indented.
//
// Revision 2.2  2002/02/27 14:31:57  croessel
// Changed access of "typedef VehCont" to public because it is used by
// MSLaneChanger.
//
// Revision 2.1  2002/02/27 13:14:05  croessel
// Prefixed ostream with "std::".
//
// Revision 2.0  2002/02/14 14:43:16  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.16  2002/02/05 13:51:52  croessel
// GPL-Notice included.
// In *.cpp files also config.h included.
//
// Revision 1.15  2002/02/05 11:48:48  croessel
// Made MSLaneChanger a friend class and provided a temporary
// vehicle-container for lane-changing.
//
// Revision 1.14  2002/01/23 11:16:28  croessel
// emit() rewritten to avoid redundant code and make things clearer. This
// leads to some overloaded emitTry()'s and a new enoughSpace() method.
//
// Revision 1.13  2002/01/16 15:41:42  croessel
// New implementation of emit(). Should be collision free now. Introduced
// a new protected method "bool emitTry()" for convenience.
//
// Revision 1.12  2002/01/09 14:58:42  croessel
// Added MSLane::Link::setPriority( bool ) for StopLights to modify the
// link's priority. Used to implement the "green arrow".
//
// Revision 1.11  2001/12/20 14:28:14  croessel
// using namespace std replaced by std::
//
// Revision 1.10  2001/12/19 16:30:57  croessel
// Changes due to new junction-hierarchy.
//
// Revision 1.9  2001/11/21 15:18:09  croessel
// Renamed buffer2lane() into integrateNewVehicle() and moved it to
// public, so that Junctions can perform this action.
// Introduction of private member myLastState, a temporary that is used
// to return the correct position of the last vehicle to lookForward
// (position before it was moved).
//
// Revision 1.8  2001/11/15 17:17:36  croessel
// Outcommented the inclusion of the inline *.iC files. Currently not needed.
// Vehicle-State introduced. Simulation is now independant of vehicle's
// speed. Still, the position is fundamental, also a gap between
// vehicles. But that's it.
//
// Revision 1.7  2001/11/14 11:45:55  croessel
// Resolved conflicts which appeared during suffix-change and
// CR-line-end commits.
//
// Revision 1.6  2001/11/14 10:49:07  croessel
// CR-line-end removed.
//
// Revision 1.5  2001/10/23 09:31:07  traffic
// parser bugs removed
//
// Revision 1.3  2001/09/06 15:47:08  croessel
// Numerous changes during debugging session.
//
// Revision 1.2  2001/07/16 12:55:47  croessel
// Changed id type from unsigned int to string. Added string-pointer
// dictionaries and dictionary methods.
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

#include "MSLogicJunction.h"
#include "MSEdge.h"
#include "MSVehicle.h"
#include "MSEdgeControl.h"
#include <bitset>
#include <deque>
#include <vector>
#include <utility>
#include <map>
#include <string>
#include <iostream>
#include "MSNet.h"
#include "output/meandata/MSLaneMeanDataValues.h"
#include <utils/geom/Position2DVector.h>
#include <utils/common/SUMOTime.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSLaneChanger;
class MSEmitter;
class MSLink;
class MSMoveReminder;
class GUILaneWrapper;
class GUIGlObjectStorage;
class MSVehicleTransfer;
class OutputDevice;
class SSLaneMeanData;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class MSLane
 * Class which represents a single lane. Somekind of the main class of the
 * simulation. Allows moving vehicles.
 */
class MSLane
{
public:
    /// needs access to myTmpVehicles (this maybe should be done via SUMOReal-buffering!!!)
    friend class MSLaneChanger;

    /// needs access to myTmpVehicles (this maybe should be done via SUMOReal-buffering!!!)
    friend class GUILaneChanger;

    /// needs direct access to maxSpeed
    friend class MSLaneSpeedTrigger;

    friend class GUILaneWrapper;

    friend class MSMeanData_Net;
    friend class SSMeanData_Net;

    friend class MSXMLRawOut;

    /** Function-object in order to find the vehicle, that has just
        passed the detector. */
    struct VehPosition : public std::binary_function< const MSVehicle*,
                         SUMOReal, bool >
    {
        /// compares vehicle position to the detector position
        bool operator() ( const MSVehicle* cmp, SUMOReal pos ) const {
            return cmp->getPositionOnLane() >= pos;
        }
    };


    /// Destructor.
    virtual ~MSLane();

    /** Use this constructor only. Later use initialize to complete
        lane initialization. */
    MSLane( //MSNet &net,
            std::string id,
            SUMOReal maxSpeed,
            SUMOReal length,
            MSEdge* edge,
            size_t numericalID,
			const Position2DVector &shape
            );

    /** Not all lane-members are known at the time the lane is born,
        above all the pointers to other lanes, so we have to
        initialize later. */
    void initialize( /*MSJunction* backJunction,*/
                     MSLinkCont* succs);
    void resetApproacherDistance();
    void resetApproacherDistance(MSVehicle *v);

    virtual void moveNonCritical();

    virtual void moveCritical();

    /// Check if vehicles are too close.
    void detectCollisions( SUMOTime timestep );

    /// Emit vehicle with speed 0 into lane if possible.
    virtual bool emit( MSVehicle& newVeh );

    /** @brief Try to emit a vehicle with speed > 0
        i.e. from a source with initial speed values. */
    virtual bool isEmissionSuccess( MSVehicle* aVehicle, const MSVehicle::State &vstate );

    /** Moves the critical vehicles
        This step is done after the responds have been set */
    virtual void setCritical( );

    /// Insert buffered vehicle into the real lane.
    virtual void integrateNewVehicle();

    //--------------- Methods used by Vehicles  ---------------------
    /** Returns the information whether this lane may be used to continue
        the current route */
    virtual bool appropriate(const MSVehicle *veh);

    //--------------- Methods used by Junctions  ---------------------

    /// returns the container with all links !!!
    const MSLinkCont &getLinkCont() const;

    //-------------- End of junction-used methods --------------------------------

    /// Returns true if there is not a single vehicle on the lane.
    bool empty() const {
        assert( myVehBuffer == 0 );
        return myVehicles.empty();
    }

    /// Returns the lane's maximum speed.
    SUMOReal maxSpeed() const {
        assert(myMaxSpeed>=0);
        return myMaxSpeed;
    }

    /// Returns the lane's length.
    SUMOReal length() const {
        return myLength;
    }

    /// Returns the lane's Edge.
    const MSEdge * const getEdge() const {
        return myEdge;
    }

    /** @brief Inserts a MSLane into the static dictionary
        Returns true if the key id isn't already in the dictionary.
        Otherwise returns false. */
    static bool dictionary( std::string id, MSLane* lane );

    /** @brief Returns the MSEdgeControl associated to the key id if exists
       Otherwise returns 0. */
    static MSLane* dictionary( std::string id );

    /** Clears the dictionary */
    static void clear();

    static size_t dictSize() { return myDict.size(); }

    /// simple output operator
    friend std::ostream& operator<<( std::ostream& os, const MSLane& lane );

    /// Container for vehicles.
    typedef std::deque< MSVehicle* > VehCont;

    /* Adds Data for MeanValue calculation. Use this if vehicle
        leaves a lane during move ( hasFinishedLane=true) or during
        lanechange (false) or if interval is over (false). */
/*    virtual void addVehicleData( SUMOReal contTimesteps,
        unsigned discreteTimesteps, SUMOReal speedSum, SUMOReal speedSquareSum,
        unsigned index, bool hasFinishedEntireLane, bool hasLeftLane,
        bool hasEnteredLane, SUMOReal travelTimesteps = 0 );
*/

    /// Returns the lane which may be used from succLinkSource to get to nRouteEdge
    MSLinkCont::const_iterator succLinkOneLane(const MSEdge* nRouteEdge,
        const MSLane& succLinkSource) const;

    /** Same as succLink, but does not throw any assertions when
        the succeeding link could not be found;
        Returns the myLinks.end() instead; Further, the number of edges to
        look forward may be given */
    MSLinkCont::const_iterator succLinkSec( const MSVehicle& veh,
                                            unsigned int nRouteSuccs,
                                            const MSLane& succLinkSource ) const;


    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::const_iterator &i) const;

    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::iterator &i);

    /// returns the information whether the given edge is the parent edge
    virtual bool inEdge(const MSEdge *edge) const;

    /// returns the last vehicle
    const MSVehicle * const getLastVehicle() const;

    MSVehicle *getLastVehicle(MSLaneChanger &lc) const;

    MSVehicle::State myLastState;


    void init(MSEdgeControl &ctrl, MSEdgeControl::LaneUsage *useDefinition);


    /** does nothing; needed for GUI-versions where vehicles are locked
        when being displayed */
    virtual void releaseVehicles();

    /// returns the vehicles
    virtual const VehCont &getVehiclesSecure();

    size_t getVehicleNumber() const;

    void setApproaching(SUMOReal dist, MSVehicle *veh);

    VehCont::const_iterator findNextVehicleByPosition(SUMOReal pos) const;

    VehCont::const_iterator findPrevVehicleByPosition(
        const VehCont::const_iterator &beginAt,
        SUMOReal pos) const;

    typedef std::vector< MSMoveReminder* > MoveReminderCont;
    /// Add a move-reminder to move-reminder container
    virtual void addMoveReminder( MSMoveReminder* rem );
    MoveReminderCont getMoveReminders( void );

    // valid for gui-version only
    virtual GUILaneWrapper *buildLaneWrapper(GUIGlObjectStorage &idStorage);

    MSVehicle *removeFirstVehicle();
    MSVehicle *myApproaching;

    size_t getNumericalID() const;

    void insertMeanData(unsigned int number);

    const std::string &getID() const;

    void addMean2(SUMOReal v, SUMOReal l);

	/// The shape of the lane
    Position2DVector myShape;

    inline MSLaneMeanDataValues &getMeanData(int index) const {
        assert((int) myMeanData.size()>index);
        return myMeanData[index];
    }

    const Position2DVector &getShape() const { return myShape; }

    virtual SUMOReal getDensity() const;

    MSLane * const getLeftLane() const;
    MSLane * const getRightLane() const;


protected:
    /** @brief Function Object for use with Function Adapter on vehicle containers.
        Returns the information whether the position of the first vehicle
        is greater than the one of the second vehicle */
    class PosGreater
    {
    public:
        /// the first vehicle
        typedef const MSVehicle* first_argument_type;
        /// the second vehicle
        typedef const MSVehicle* second_argument_type;
        /// returns bool
        typedef bool result_type;

        /** Returns true if position of first vehicle is greater
            then positition of the second one. */
        result_type operator() ( first_argument_type veh1,
                                 second_argument_type veh2 ) const;
    };


    /** @brief Find nearest vehicle on neighboured lanes (there may be more than one) which isn't allowed to be overtaken.
        Position and speed are conditional parameters. Returns veh if there is no
        neigh to regard. */
    const MSVehicle* findNeigh( MSVehicle* veh,
                                MSEdge::LaneCont::const_iterator first,
                                MSEdge::LaneCont::const_iterator last );

    /** @brief Insert a vehicle into the lane's vehicle buffer.
        After processing done from moveCritical, when a vehicle exits it's lane.
        Returned is the information whether the vehicle was removed. */
    virtual bool push( MSVehicle* veh );

    /** Returns the first/front vehicle of the lane and removing it from the lane. */
    MSVehicle* pop();

    /** @brief Tries to emit veh into lane.
        There are four kind of possible emits that have to be handled differently:
        - The line is empty,
        - emission as last veh (in driving direction) (front insert),
        - as first veh (back insert)
        - between a follower and a leader.
        Regard that some of these methods are private, as the source lanes must
        not insert vehicles in front of other vehicles.
        True is returned for successful emission.
        Use this when the lane is empty */
    virtual bool emitTry( MSVehicle& veh );

    /** Use this, when there is only a vehicle in front of the vehicle
        to insert */
    virtual bool emitTry( MSVehicle& veh, VehCont::iterator leaderIt );

    /** Resets the MeanData container at the beginning of a new interval.*/
    virtual void resetMeanData( unsigned index );

    /** Retrieves Data from all vehicles on the lane at the end of an interval. */
//    virtual void collectVehicleData( unsigned index );

    /// moves myTmpVehicles int myVehicles after a lane change procedure
    virtual void swapAfterLaneChange();

protected:
    /// Unique ID.
    std::string myID;

    /// Unique numerical ID (set on reading by netload)
    size_t myNumericalID;

    /** @brief The lane's vehicles.
        The entering vehicles are inserted at the front
        of  this container and the leaving ones leave from the back, e.g. the
        vehicle in front of the junction (often called first) is
        myVehicles.back() (if it exists). And if it is an iterator at a
        vehicle, ++it points to the vehicle in front. This is the interaction
        vehicle. */
    VehCont myVehicles;

    /// Lane length [m]
    SUMOReal myLength;

    /// The lane's edge, for routing only.
    MSEdge* myEdge;

    /// Lane-wide speedlimit [m/s]
    SUMOReal myMaxSpeed;

    /** Container for lane-changing vehicles. After completion of lane-change-
        process, the two containers will be swapped. */
    VehCont myTmpVehicles;


    SUMOReal myBackDistance;

    /** Vehicle-buffer for vehicle that was put onto this lane by a
        junction. The  buffer is neccessary, because of competing
        push- and pop-operations on myVehicles during
        Junction::moveFirst() */
    MSVehicle* myVehBuffer;

    MSEdgeControl::LaneUsage *myUseDefinition;

private:

    /** Use this, when there is only a vehicle behind the vehicle to insert */
    bool emitTry( VehCont::iterator followIt, MSVehicle& veh ); // back ins.

    /** Use this, when the new vehicle shall be inserted between two other vehicles */
    bool emitTry( VehCont::iterator followIt, MSVehicle& veh,
                  VehCont::iterator leaderIt ); // in between ins.

    /// index of the first vehicle that may drive over the lane's end
    size_t myFirstUnsafe;

    /** The lane's Links to it's succeeding lanes and the default
        right-of-way rule, i.e. blocked or not blocked. */
    MSLinkCont myLinks;

//----------- Declarations for mean-data calculation


    /** Container of MeanDataValues, one element for each intervall. */
    mutable std::vector< MSLaneMeanDataValues > myMeanData;

    //----------- End of declarations for mean-data calculation

    /// definition of the tatic dictionary type
    typedef std::map< std::string, MSLane* > DictType;

    /// Static dictionary to associate string-ids with objects.
    static DictType myDict;

private:
    /// Default constructor.
    MSLane();

    /// Copy constructor.
    MSLane( const MSLane& );

    /// Assignment operator.
    MSLane& operator=( const MSLane& );

    MoveReminderCont moveRemindersM;

};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
