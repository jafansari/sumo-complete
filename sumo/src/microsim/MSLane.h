/****************************************************************************/
/// @file    MSLane.h
/// @author  Christian Roessel
/// @date    Mon, 12 Mar 2001
/// @version $Id$
///
// operate.
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
#ifndef MSLane_h
#define MSLane_h
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
#include <utils/common/SUMOVehicleClass.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLaneChanger;
class MSEmitter;
class MSLink;
class MSMoveReminder;
class GUILaneWrapper;
class GUIGlObjectStorage;
class MSVehicleTransfer;
class OutputDevice;
class SSLaneMeanData;


// ===========================================================================
// class definitions
// ===========================================================================
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
        bool operator()(const MSVehicle* cmp, SUMOReal pos) const
        {
            return cmp->getPositionOnLane() >= pos;
        }
    };


    /// Destructor.
    virtual ~MSLane();

    /** Use this constructor only. Later use initialize to complete
        lane initialization. */
    MSLane(std::string id, SUMOReal maxSpeed, SUMOReal length, MSEdge* edge,
           size_t numericalID, const Position2DVector &shape,
           const std::vector<SUMOVehicleClass> &allowed,
           const std::vector<SUMOVehicleClass> &disallowed);

    /** Not all lane-members are known at the time the lane is born,
        above all the pointers to other lanes, so we have to
        initialize later. */
    void initialize(/*MSJunction* backJunction,*/
        MSLinkCont* succs);
    void resetApproacherDistance();
    void resetApproacherDistance(MSVehicle *v);

    virtual void moveNonCritical();

    virtual void moveCritical();

    /// Check if vehicles are too close.
    virtual void detectCollisions(SUMOTime timestep);

    /// Emit vehicle with speed 0 into lane if possible.
    virtual bool emit(MSVehicle& newVeh);

    /** @brief Try to emit a vehicle with speed > 0
        i.e. from a source with initial speed values. */
    virtual bool isEmissionSuccess(MSVehicle* aVehicle, const MSVehicle::State &vstate);

    /** Moves the critical vehicles
        This step is done after the responds have been set */
    virtual void setCritical();

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
    bool empty() const
    {
        assert(myVehBuffer == 0);
        return myVehicles.empty();
    }

    /// Returns the lane's maximum speed.
    SUMOReal maxSpeed() const
    {
        assert(myMaxSpeed>=0);
        return myMaxSpeed;
    }

    /// Returns the lane's length.
    SUMOReal length() const
    {
        return myLength;
    }

    /// Returns the lane's Edge.
    const MSEdge * const getEdge() const
    {
        return myEdge;
    }

    /** @brief Inserts a MSLane into the static dictionary
        Returns true if the key id isn't already in the dictionary.
        Otherwise returns false. */
    static bool dictionary(std::string id, MSLane* lane);

    /** @brief Returns the MSEdgeControl associated to the key id if exists
       Otherwise returns 0. */
    static MSLane* dictionary(std::string id);

    /** Clears the dictionary */
    static void clear();

    static size_t dictSize()
    {
        return myDict.size();
    }

    /// simple output operator
    friend std::ostream& operator<<(std::ostream& os, const MSLane& lane);

    /// Container for vehicles.
    typedef std::deque< MSVehicle* > VehCont;

    /** Same as succLink, but does not throw any assertions when
        the succeeding link could not be found;
        Returns the myLinks.end() instead; Further, the number of edges to
        look forward may be given */
    MSLinkCont::const_iterator succLinkSec(const MSVehicle& veh,
                                           unsigned int nRouteSuccs,
                                           const MSLane& succLinkSource) const;


    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::const_iterator &i) const;

    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::iterator &i);

    /// returns the information whether the given edge is the parent edge
    virtual bool inEdge(const MSEdge *edge) const;

    /// returns the last vehicle
    virtual const MSVehicle * const getLastVehicle() const;
    virtual const MSVehicle * const getFirstVehicle() const;

    virtual MSVehicle *getLastVehicle(MSLaneChanger &lc) const;

    MSVehicle::State myLastState;


    void init(MSEdgeControl &ctrl, MSEdgeControl::LaneUsage *useDefinition);


    /** does nothing; needed for GUI-versions where vehicles are locked
        when being displayed */
    virtual void releaseVehicles();

    /// returns the vehicles
    virtual const VehCont &getVehiclesSecure();

    size_t getVehicleNumber() const;

    void setApproaching(SUMOReal dist, MSVehicle *veh);

//    VehCont::const_iterator findNextVehicleByPosition(SUMOReal pos) const;
    /*
        VehCont::const_iterator findPrevVehicleByPosition(
            const VehCont::const_iterator &beginAt,
            SUMOReal pos) const;
    */
    typedef std::vector< MSMoveReminder* > MoveReminderCont;
    /// Add a move-reminder to move-reminder container
    virtual void addMoveReminder(MSMoveReminder* rem);
    MoveReminderCont getMoveReminders(void);

    // valid for gui-version only
    virtual GUILaneWrapper *buildLaneWrapper(GUIGlObjectStorage &idStorage);

    MSVehicle *removeFirstVehicle();
    MSVehicle *myApproaching;

    size_t getNumericalID() const;

    void insertMeanData(unsigned int number);

    const std::string &getID() const;

    void addMean2(const MSVehicle &veh, SUMOReal newV, SUMOReal oldV, SUMOReal gap);

    /// The shape of the lane
    Position2DVector myShape;

    inline MSLaneMeanDataValues &getMeanData(int index) const
    {
        assert((int) myMeanData.size()>index);
        return myMeanData[index];
    }

    const Position2DVector &getShape() const
    {
        return myShape;
    }

    virtual SUMOReal getDensity() const;
    virtual SUMOReal getVehLenSum() const;


    MSLane * const getLeftLane() const;
    MSLane * const getRightLane() const;

    const std::vector<SUMOVehicleClass> &getAllowedClasses() const;
    const std::vector<SUMOVehicleClass> &getNotAllowedClasses() const;
    bool allowsVehicleClass(SUMOVehicleClass vclass) const;


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
        result_type operator()(first_argument_type veh1,
                               second_argument_type veh2) const;
    };


    /** @brief Insert a vehicle into the lane's vehicle buffer.
        After processing done from moveCritical, when a vehicle exits it's lane.
        Returned is the information whether the vehicle was removed. */
    virtual bool push(MSVehicle* veh);

    /** Returns the first/front vehicle of the lane and removing it from the lane. */
    virtual MSVehicle* pop();

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
    virtual bool emitTry(MSVehicle& veh);

    /** Use this, when there is only a vehicle in front of the vehicle
        to insert */
    virtual bool emitTry(MSVehicle& veh, VehCont::iterator leaderIt);

    /** Resets the MeanData container at the beginning of a new interval.*/
    virtual void resetMeanData(unsigned index);

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

    /// The list of allowed vehicle classes
    std::vector<SUMOVehicleClass> myAllowedClasses;

    /// The list of disallowed vehicle classes
    std::vector<SUMOVehicleClass> myNotAllowedClasses;


protected:

    /** Use this, when there is only a vehicle behind the vehicle to insert */
    bool emitTry(VehCont::iterator followIt, MSVehicle& veh);   // back ins.

    /** Use this, when the new vehicle shall be inserted between two other vehicles */
    bool emitTry(VehCont::iterator followIt, MSVehicle& veh,
                 VehCont::iterator leaderIt);  // in between ins.

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
    MSLane(const MSLane&);

    /// Assignment operator.
    MSLane& operator=(const MSLane&);

    MoveReminderCont moveRemindersM;

};


#endif

/****************************************************************************/

