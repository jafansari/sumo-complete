/****************************************************************************/
/// @file    MSVehicle.h
/// @author  Christian Roessel
/// @date    Mon, 12 Mar 2001
/// @version $Id$
///
// Representation of a vehicle in the micro simulation
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef MSVehicle_h
#define MSVehicle_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSEdge.h"
#include "MSNet.h"
#include "MSRoute.h"
#include "MSCORN.h"
#include <list>
#include <deque>
#include <map>
#include <string>
#include <vector>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "MSVehicleQuitReminded.h"
#include <utils/common/SUMOVehicleClass.h>
#include "MSVehicleType.h"
#include <utils/common/SUMOAbstractRouter.h>

#ifdef HAVE_MESOSIM
#include <mesosim/MEVehicle.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSMoveReminder;
class MSLaneChanger;
class MSVehicleTransfer;
class MSAbstractLaneChangeModel;
class MSBusStop;
class OutputDevice;
class MSDevice;
#ifdef _MESSAGES
class MSMessageEmitter;
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicle
 * @brief Representation of a vehicle in the micro simulation
 */
class MSVehicle : public MSVehicleQuitReminded
#ifdef HAVE_MESOSIM
            , public MEVehicle
#endif
{
public:

    /// the lane changer sets myLastLaneChangeOffset
    friend class MSLaneChanger;

    /** container that holds the vehicles driving state. May vary from
        model to model. here: SK, holds position and speed. */
    class State
    {
        /// vehicle sets states directly
        friend class MSVehicle;
        friend class MSLaneChanger;

    public:
        /// Constructor.
        State(SUMOReal pos, SUMOReal speed);

        /// Copy constructor.
        State(const State& state);

        /// Assignment operator.
        State& operator=(const State& state);

        /// Operator !=
        bool operator!=(const State& state);

        /// Position of this state.
        SUMOReal pos() const;

        /// Speed of this state
        SUMOReal speed() const {
            return mySpeed;
        };

    private:
        /// the stored position
        SUMOReal myPos;

        /// the stored speed
        SUMOReal mySpeed;

    };

    /// Destructor.
    virtual ~MSVehicle() throw();



    /// @name emission handling
    //@{

    /** @brief Returns the vehicle's parameter (including departure definition)
     *
     * @return The vehicle's parameter
     */
    const SUMOVehicleParameter &getParameter() const throw() {
        return *myParameter;
    }


    /** @brief Returns the edge the vehicle starts from
     * @return The vehicle's departure edge
     */
    const MSEdge &getDepartEdge() const {
        return **myCurrEdge;
    }


    /** @brief Returns the lanes the vehicle may be emitted onto
     * @return Lanes that are allowed to be used for emission
     * @todo recheck this; are these lanes on which the vehicle's class is allowed, only?
     */
    const MSEdge::LaneCont &getDepartLanes() const {
        return *(myAllowedLanes[0]);
    }


    /// Returns the desired departure time.
    SUMOTime getDesiredDepart() const throw() {
        return myParameter->depart;
    }
    //@}



    void removeOnTripEnd(MSVehicle *veh) throw();



    /// @name interaction with the route
    //@{

    /** @brief Returns whether the vehicle has still to pass nSuccs edges
     * @param[in] nSuccs The number of edge to look forward
     * @return Whether the rest of the route is at least as long as nSuccs
     */
    inline bool hasSuccEdge(unsigned int nSuccs) const throw() {
        return myCurrEdge + nSuccs < myRoute->end();
    }

    /** @brief Returns the nSuccs'th successor of edge the vehicle is currently at
     *
     * If the rest of the route (counted from the current edge) than nSuccs,
     *  0 is returned.
     * @param[in] nSuccs The number of edge to look forward
     * @return The nSuccs'th following edge in the vehicle's route
     */
    const MSEdge* succEdge(unsigned int nSuccs) const throw();


    /** Returns true if vehicle is going to enter it's destination
        edge. False otherwise. Adjusts in both cases the route
        iterator and the allowedLanes-container.
        @todo decribe, choose a proper name*/
    bool destReached(const MSEdge* targetEdge) throw();


    /** @brief Returns the information whether the vehicle should end now
     * @return Whether the route ends
     */
    bool ends() const throw();

    /// Moves vehicle one edge forward, returns true if the route has ended
    bool proceedVirtualReturnWhetherEnded(const MSEdge *const to);

    /// Returns the current route
    const MSRoute &getRoute() const;

    /// Returns the current or a prior route
    const MSRoute &getRoute(int index) const;

    /// Replaces the current route by the given edges
    bool replaceRoute(const MSEdgeVector &edges, SUMOTime simTime);

    bool willPass(const MSEdge * const edge) const;

    void reroute(SUMOTime t, SUMOAbstractRouter<MSEdge, MSVehicle> &router);



    //@}


    /// moves the vehicles after their responds (right-of-way rules) are known
    void moveFirstChecked();


    /** @brief Returns the gap between pred and this vehicle.
     *
     * Assumes both vehicles are on the same or on are on parallel lanes.
     *
     * @param[in] pred The leader
     * @return The gap between this vehicle and the leader (may be <0)
     */
    SUMOReal gap2pred(const MSVehicle& pred) const throw() {
        SUMOReal gap = pred.getPositionOnLane() - pred.getVehicleType().getLength() - getPositionOnLane();
        if (gap<0&&gap>-1.0e-12) {
            gap = 0;
        }
        return gap;
    }


    /** @brief Uses the given values to compute the brutto-gap
     *
     * @param[in] predPos Position of the leader
     * @param[in] predLength Length of the leader
     * @param[in] pos Position of the follower
     * @return The gap between the leader and the follower
     */
    static inline SUMOReal gap(SUMOReal predPos, SUMOReal predLength, SUMOReal pos) throw() {
        return predPos - predLength - pos;
    }


    /// @name Retrieval of CORN values
    //@{

    /** @brief Returns the named CORN integer value
     *
     * The behaviour is undefined for not stored values.
     *
     * @return The named CORN integer value
     * @todo Recheck whether throwing an exception for unset values is more appropriate
     */
    int getCORNIntValue(MSCORN::Function f) const throw();


    /** @brief Returns the named CORN pointer value
     *
     * The behaviour is undefined for not stored values.
     *
     * @return The named CORN pointer value
     * @todo Recheck whether throwing an exception for unset values is more appropriate
     */
    void *getCORNPointerValue(MSCORN::Pointer p) const throw();


    /** Returns whether this vehicle has the named integer value stored
     * @return Whether the named integer value is stored in CORN
     */
    bool hasCORNIntValue(MSCORN::Function f) const throw();


    /** Returns whether this vehicle has the named pointer value stored
     * @return Whether the named pointer value is stored in CORN
     */
    bool hasCORNPointerValue(MSCORN::Pointer p) const throw();
    //@}


    /// @name Interaction with move reminders
    //@{

    /** @brief Processes active move reminder
     *
     * This method goes through all active move reminder, both those for the current
     *  lane, stored in "myMoveReminders" and those of prior lanes stored in
     *  "myOldLaneMoveReminders" calling "MSMoveReminder::isStillActive".
     *
     * When processing move reminder from "myOldLaneMoveReminders",
     *  the offsets (prior lane lengths) are used, which are stored in
     *  "myOldLaneMoveReminderOffsets".
     *
     * Each move reminder which is no longer active is removed from the container.
     *
     * @param[in] oldPos The position the vehicle had before it has moved
     * @param[in] newPos The position the vehicle has after it has moved
     * @param[in] newSpeed The vehicle's speed within this move
     * @see MSMoveReminder
     */
    void workOnMoveReminders(SUMOReal oldPos, SUMOReal newPos, SUMOReal newSpeed) throw();


    /** @brief Returns the vehicle's position in relation to a passed move reminder lane
     *
     * @param[in] searchedLane The lane to search for within move reminder
     * @return The distance to this lane (vehicle position + lane offset); -1 if n move reminder is placed on this lane
     * @todo Maybe this should be rechecked - the name says it: much to complicated and the parameter should be a move reminder?
     */
    SUMOReal getPositionOnActiveMoveReminderLane(const MSLane * const searchedLane) const throw();
    //@}



    /** @brief Moves a vehicle if it is not meant to be running out of the lane
     *
     * @param[in] lane The lane the vehicle is on
     * @param[in] pred The leader (may be 0)
     * @param[in] neigh The neighbor vehicle (may be 0)
     * @return Whether a collision occured (gap2pred(leader)<=0)
     */
    bool move(const MSLane* const lane, const MSVehicle * const pred, const MSVehicle * const neigh) throw();


    /** @brief Moves vehicles which may run out of the lane
     *
     * @param[in] lane The lane the vehicle is on
     * @param[in] pred The leader (may be 0)
     * @param[in] neigh The neighbor vehicle (may be 0)
     * @return Whether a collision occured (gap2pred(leader)<=0)
     */
    bool moveRegardingCritical(const MSLane* const lane, const MSVehicle * const pred, const MSVehicle * const neigh) throw();


    /// @name state setter/getter
    //@{

    /** @brief Returns the vehicle state
     * @return The state of the vehicle
     * @see State
     */
    const State &getState() const throw() {
        return myState;
    }


    /** @brief Get the vehicle's position along the lane
     * @return The position of the vehicle (in m from the lane's begin)
     */
    SUMOReal getPositionOnLane() const throw() {
        return myState.myPos;
    }


    /** @brief Returns the vehicle's current speed
     * @return The vehicle's speed
     */
    SUMOReal getSpeed() const throw() {
        return myState.mySpeed;
    }


    /** brief Returns the vehicle's acceleration
     * @return The acceleration
     */
    SUMOReal getAcceleration() const throw() {
        return myAcceleration;
    }
    //@}



    SUMOReal interactionGap(SUMOReal vF, SUMOReal laneMaxSpeed, SUMOReal vL) const {
        return myType->interactionGap(vF, laneMaxSpeed, vL);
    }

    bool hasSafeGap(SUMOReal speed, SUMOReal gap, SUMOReal predSpeed, SUMOReal laneMaxSpeed) const {
        return myType->hasSafeGap(speed, gap, predSpeed, laneMaxSpeed);
    }

    /**
     * !!! rework this - leader should not be given directly, rather his speed after decel...
     */
    SUMOReal getSecureGap(SUMOReal speed, SUMOReal leaderSpeed, MSVehicle &leader) const {
        SUMOReal leaderSpeedAfterDecel = leader.getSpeedAfterMaxDecel(leaderSpeed);
        SUMOReal speedDiff = speed - leaderSpeedAfterDecel;
        SUMOReal resGap = speedDiff * speedDiff / myType->getMaxDecel() + speed * myType->getTau();
        return resGap;
    }

    SUMOReal ffeV(SUMOReal speed, SUMOReal gap2pred, SUMOReal predSpeed) const {
        return myType->ffeV(speed, gap2pred, predSpeed);
    }

    SUMOReal decelAbility() const {
        return myType->decelAbility(); // !!! really the speed?
    }

    SUMOReal maxNextSpeed(SUMOReal v) const {
        return myType->maxNextSpeed(v);
    }

    SUMOReal getSpeedAfterMaxDecel(SUMOReal v) const {
        return myType->getSpeedAfterMaxDecel(v);
    }

    SUMOReal getMaxSpeed() const {
        if (isIndividualMaxSpeedSet())
            return getIndividualMaxSpeed();

        return myType->getMaxSpeed();
    }

    void setIndividualMaxSpeed(SUMOReal individualMaxSpeed) {
        myIsIndividualMaxSpeedSet = true;
        myIndividualMaxSpeed = individualMaxSpeed;
    }

    void unsetIndividualMaxSpeed(void) {
        myIsIndividualMaxSpeedSet = false;
        myIndividualMaxSpeed = 0.0;
    }

    SUMOReal getIndividualMaxSpeed(void) const {
        return myIndividualMaxSpeed;
    }

    bool isIndividualMaxSpeedSet(void) const {
        return myIsIndividualMaxSpeedSet;
    }

    /** */
    SUMOReal timeHeadWayGap(SUMOReal speed) const {
        assert(speed >= 0);
        return SPEED2DIST(speed);
    }


    /// Returns the name of the vehicle
    const std::string &getID() const;

    /** Return true if the lane is allowed */
    bool onAllowed(const MSLane* lane) const;

    /** Return true if vehicle is on an allowed lane. */
    bool onAllowed() const;


    /** Returns true if the two vehicles overlap. */
    static bool overlap(const MSVehicle* veh1, const MSVehicle* veh2) {
        if (veh1->myState.myPos < veh2->myState.myPos) {
            return veh2->myState.myPos - veh2->getVehicleType().getLength() < veh1->myState.myPos;
        }
        return veh1->myState.myPos - veh1->getVehicleType().getLength() < veh2->myState.myPos;
    }


    /** Returns true if vehicle's speed is below 60km/h. This is only relevant
        on highways. Overtaking on the right is allowed then. */
    bool congested() const {
        return myState.mySpeed < SUMOReal(60) / SUMOReal(3.6);
    }

    /// Return current Position
    Position2D getPosition() const;


    /// @name state io
    //@{

    /// Saves the states of a vehicle
    void saveState(std::ostream &os);
    //@}


    const MSEdge * const getEdge() const;


    /** @brief Update when the vehicle enters a new lane in the move step.
     *
     * @param[in] enteredLane The lane the vehicle enters
     * @param[in] driven The distance driven by the vehicle within this time step
     */
    void enterLaneAtMove(MSLane* enteredLane, SUMOReal driven);



    /** @brief Update when the vehicle enters a new lane in the emit step
     *
     * @param[in] enteredLane The lane the vehicle enters
     * @param[in] pos The position the vehicle was inserted into the lane
     * @param[in] speed The speed with which the vehicle was inserted into the lane
     */
    void enterLaneAtEmit(MSLane* enteredLane, SUMOReal pos, SUMOReal speed);


    /** @brief Update when the vehicle enters a new lane in the laneChange step.
     *
     * @param[in] enteredLane The lane the vehicle enters
     */
    void enterLaneAtLaneChange(MSLane* enteredLane);


    /** @brief Update when the vehicle leaves a new lane in the move step.
     *
     * @param[in] driven The distance driven by the vehicle within this time step
     */
    void leaveLaneAtMove(SUMOReal driven);


    /** @brief Update of members if vehicle leaves a new lane in the lane change step. */
    void leaveLaneAtLaneChange();


    bool reachingCritical(SUMOReal laneLength) const;

    friend class MSVehicleControl;

    void vsafeCriticalCont(SUMOReal minVSafe);

    MSLane *getTargetLane() const;

    /// Returns the lane the vehicle is on
    const MSLane &getLane() const;

    /// @name usage of multiple vehicle emissions
    //@{

    /** @brief Returns the next "periodical" vehicle with the same route
        We have to duplicate the vehicle if a further has to be emitted with
        the same settings. Returns 0 if no further vehicle shall be created. */
    virtual MSVehicle *getNextPeriodical() const;
    //@}


    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    inline bool isOnRoad() const throw() {
        return myLane!=0;
    }



    /** @brief returns the number of steps waited
        A vehicle is meant to be "waiting" when it's speed is less than 0.1
        It is only computed for "critical" vehicles
        The method return a size_t, now, as we assume a vehicle will not wait for
        longer than about 50 hours which still fits into a size_t when the simulation
        runs in ms */
    unsigned int getWaitingTime() const;


    void rebuildAllowedLanes(bool reinit=true);

    void quitRemindedEntered(MSVehicleQuitReminded *r);
    void quitRemindedLeft(MSVehicleQuitReminded *r);


    MSAbstractLaneChangeModel &getLaneChangeModel();
    const MSAbstractLaneChangeModel &getLaneChangeModel() const;
    typedef std::deque<const MSEdge::LaneCont*> NextAllowedLanes;

    /// @name stretegical/tactical lane choosing methods
    /// @{

    /** @struct LaneQ
     * @brief A structure representing the best lanes for continuing the route
     * @todo Describe
     */
    struct LaneQ {
        MSLane *lane;
        SUMOReal length;
        SUMOReal v;
        SUMOReal occupied;
        int bestLaneOffset;
        bool allowsContinuation;
        std::vector<MSLane*> joined;
    };

    /** @brief Returns the description of best lanes to use in order to continue the route
     *
     * The information is rebuilt if the vehicle is on a different edge than
     *  the one stored in "myLastBestLanesEdge" or "forceRebuild" is true.
     *
     * Otherwise, only the density changes on the stored lanes are adapted to
     *  the container only.
     *
     * A rebuild must be done if the vehicle leaves a stop; then, another lane may get
     *  the best one.
     *
     * If no starting lane ("startLane") is given, the vehicle's current lane ("myLane")
     *  is used as start of bect lanes building.
     *
     * @param[in] forceRebuild Whether the best lanes container shall be rebuilt even if the vehicle's edge has not changed
     * @param[in] startLane The lane the process shall start at ("myLane" will be used if ==0)
     * @return The best lanes structure holding matching the current vehicle position and state ahead
     */
    virtual const std::vector<LaneQ> &getBestLanes(bool forceRebuild=false, MSLane *startLane=0) const throw();


    /** @brief Returns the subpart of best lanes that describes the vehicle's current lane and their successors
     * @return The best lane information for the vehicle's current lane
     * @todo Describe better
     */
    const std::vector<MSLane*> &getBestLanesContinuation() const throw();

    /** @brief Returns the subpart of best lanes that describes the given lane and their successors
     * @return The best lane information for the given lane
     * @todo Describe better
     */
    const std::vector<MSLane*> &getBestLanesContinuation(const MSLane * const l) const throw();
    /// @}

    SUMOReal getMovedDistance() const throw() {
        return SPEED2DIST(myState.mySpeed);
    }

    const MSVehicleType &getVehicleType() const;


    void onDepart();

    /** @brief Called when the vehicle leaves the lane */
    void onTripEnd(const MSLane * const lane=0);
    void writeXMLRoute(OutputDevice &os, int index=-1) const;


    /// @name vehicle stops definitions and i/o
    //@{

    /** @struct Stop
     * @brief Definition of vehicle stop (position and duration)
     */
    struct Stop {
        /// @brief The lane to stop at
        const MSLane *lane;
        /// @brief (Optional) bus stop if one is assigned to the stop
        MSBusStop *busstop;
        /// @brief The stopping position
        SUMOReal pos;
        /// @brief The stopping duration
        SUMOTime duration;
        /// @brief The time at which the vehicle may continue its journey
        SUMOTime until;
        /// @brief Information whether the stop has been reached
        bool reached;
    };


    /** @brief Adds a stop
     *
     * The stop is put into the sorted list.
     * @param[in] stop The stop to add
     * @return Whether the stop could be added
     */
    bool addStop(const Stop &stop) throw();


    /** @brief Returns whether the vehicle has to stop somewhere
     * @return Whether the vehicle has to stop somewhere
     */
    bool hasStops() {
        return !myStops.empty();
    }

    /** @brief Returns whether the vehicle is at a stop
     * @return Whether the has stopped
     */
    bool isStopped();
    /// @}

    SUMOVehicleClass getVehicleClass() const {
        return myType->getVehicleClass();
    }
    bool knowsEdgeTest(MSEdge &edge) const;

    class Information
    {
    public:
        Information(SUMOReal neededTime_, SUMOTime time_)
                : neededTime(neededTime_), time(time_) { }
        SUMOReal neededTime; // how long needed the vehicle to travel on the edge
        SUMOTime time; // the Time, when the Info was saved
    };

    SUMOReal getEffort(const MSEdge * const e, SUMOReal t) const;


    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    const std::vector<MSDevice*> &getDevices() const {
        return myDevices;
    }

    void setWasVaporized(bool onDepart);

    /**
     * Compute distance that will be covered, if the vehicle moves to a given position on its route,
     * starting at its current position.
     * @param destPos:	position on the destination edge that shall be reached
     * @param destEdge: destination edge that shall be reached
     * @return			distance from the vehicles current position to the destination position,
     *					or a near infinite real value if the destination position is not contained
     *					within the vehicles route or the vehicle is not active
     */
    SUMOReal getDistanceToPosition(SUMOReal destPos, const MSEdge* destEdge);

#ifndef NO_TRACI
    /**
     * schedule a new stop for the vehicle; each time a stop is reached, the vehicle
     * will wait for the given duration before continuing on its route
     * @param lane		lane on wich to stop
     * @param pos		position on the given lane at wich to stop
     * @param radius	the vehicle will stop if it is within the range [pos-radius, pos+radius]
     * @duration		after waiting for the time period duration, the vehicle will
     *					continue until the stop is reached again
     */
    bool addTraciStop(MSLane* lane, SUMOReal pos, SUMOReal radius, SUMOTime duration);

    void checkReroute(SUMOTime t);

    /**
     * Used by TraCIServer to change the weight of an edge locally for a specific vehicle
     * @param edgeID: ID of the edge to change
     * @param travelTime: the new time to be set for the edge
     * @param currentTime: the current simulation time
     */
    bool changeEdgeWeightLocally(std::string edgeID, SUMOReal travelTime, SUMOTime currentTime);

    /**
     * Used by TraCIServer to restore the weight of an edge that was changed previously
     * via "changeRoute" command
     * @param edgeID: ID of the edge to restore
     */
    bool restoreEdgeWeightLocally(std::string edgeID, SUMOTime currentTime);

    /**
     * The vehicle will slow down to newSpeed within the time interval duration.
     * This is done by limiting the maximum speed every time a simulation step
     * is performed by TraCI. Speed reduction is linear.
     * @param newSpeed speed to reduce to
     * @param duration time intervall for the speed adaption
     * @param currentTime current simulation time
     */
    bool startSpeedAdaption(float newSpeed, SUMOTime duration, SUMOTime currentTime);

    /**
     * called each simulation step to slow down if necessary (see above)
     */
    void adaptSpeed();

    /**
     * Checks if the sticky time for a "changelane" command has passed already
     * @param time current simulation time
     */
    void checkLaneChangeConstraint(SUMOTime time);

    /**
     * After each changed lane, check if the lane requested by TraCI command "changeLane"
     * is reached and request for more changes if necessary.
     */
    void checkForLaneChanges();

    /**
     * Initiate a lane change requested by TraCI command "changeLane".
     * @param lane	the lane index within the current edge, that is the destination of the change
     * @param stickyTime	duration for wich the constraint takes effect
     */
    void startLaneChange(unsigned lane, SUMOTime stickyTime);

    /**
     * Forces the vehicle to change the given number of lanes to the right side
     * @param numLanes number of lanes that shall be passed
     * @param stickyTime duration for wich the lane change constraint takes effect
     */
    //void forceLaneChangeRight(int numLanes, SUMOTime stickyTime);

    /**
     * Forces the vehicle to change the given number of lanes to the left side
     * @param numLanes number of lanes that shall be passed
     * @param stickyTime duration for wich the lane change constraint takes effect
     */
    //void forceLaneChangeLeft(int numLanes, SUMOTime stickyTime);

    /**
     * takes all action necessary during a simulation step to process any active command sent by TraCI
     * @param time the current simulation time
     */
    void processTraCICommands(SUMOTime time);
#endif

protected:
    /** @brief Processes stops, returns the velocity needed to reach the stop
     * @return The velocity in dependance to the next/current stop
     * @todo Describe more detailed
     * @see Stop
     * @see MSBusStop
     */
    SUMOReal processNextStop(SUMOReal currentVelocity) throw();



    /// @name Interaction with move reminders
    ///@{

    /** @brief "Activates" all current move reminder
     *
     * For all move reminder stored in "myMoveReminders", their method
     *  "MSMoveReminder::isActivatedByEmitOrLaneChange" is called. The reminder
     *  removed if the call returns false.
     *
     * @param[in] isEmit true means emit, false: lane change
     * @see MSMoveReminder
     * @see MSMoveReminder::isActivatedByEmitOrLaneChange
     */
    void activateRemindersByEmitOrLaneChange(bool isEmit) throw();


    /** @brief Adapts the vehicle's entering of a new lane
     *
     * All offsets already stored in "myOldLaneMoveReminderOffsets" are increased by the
     *  length that has been left. All still active move reminders from "myMoveReminders"
     *  are put into "myOldLaneMoveReminders" and the offset to the last lane is added to
     *  "myOldLaneMoveReminderOffsets" for each of these.
     *
     * Move reminder from the given lane are set into "myMoveReminders".
     *
     * "myLane" must still be the left lane!
     *
     * @param[in] enteredLane
     * @see MSMoveReminder
     * @see MSLane::getMoveReminder
     */
    void adaptLaneEntering2MoveReminder(const MSLane &enteredLane) throw();
    ///@}



    void rebuildContinuationsFor(LaneQ &q, MSLane *l, MSRouteIterator ce, int seen) const;
    virtual void setBlinkerInformation() { }


    /// Use this constructor only.
    MSVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
              const MSVehicleType* type, int vehicleIndex);

    /// information how long ago the vehicle has performed a lane-change
    unsigned int myLastLaneChangeOffset;

    /// the lane, the vehicle will be within the next time step
    MSLane *myTarget;

    /** @brief The time the vehicle waits
        This is the number of simulation steps the vehicle was not faster than 0.1m/s
        It's a size_t, now, as we assume a vehicle will not wait for
        longer than about 50 hours which still fits into a size_t when the simulation
        runs in ms */
    unsigned int myWaitingTime;

    // The time the vehicle waits, may mean the same like myWaitingTime
    int timeSinceStop;

#ifdef _MESSAGES
    /// The message emitters
    MSMessageEmitter *myLCMsgEmitter;
    MSMessageEmitter *myBMsgEmitter;
#endif

    /// Vehicle's parameter.
    const SUMOVehicleParameter* myParameter;

    /// Vehicle's route.
    const MSRoute* myRoute;

    /// Vehicles driving state. here: pos and speed
    State myState;

    /// An individual speed for an vehicle that is used (iff set) instead of
    /// the maximal speed of the vehicle class.
    /// NOTE: This is just a little workaround for allowing an external
    ///       influence on the actual speed
    SUMOReal myIndividualMaxSpeed;

    /// is true if there has an individual speed been set
    bool myIsIndividualMaxSpeedSet;

    /// The lane the vehicle is on
    MSLane* myLane;

    MSAbstractLaneChangeModel *myLaneChangeModel;

    const MSVehicleType * const myType;

    mutable const MSEdge *myLastBestLanesEdge;
    mutable std::vector<std::vector<LaneQ> > myBestLanes;
    mutable std::vector<LaneQ>::iterator myCurrentLaneInBestLanes;
    static std::vector<MSLane*> myEmptyLaneVector;

    std::map<MSCORN::Pointer, void*> myPointerCORNMap;
    std::map<MSCORN::Function, int> myIntCORNMap;

    /// @brief The vehicle's list of stops
    std::list<Stop> myStops;

    /// the position on the destination lane where the vehicle stops
    SUMOReal myArrivalPos;

    /// @brief The current acceleration
    SUMOReal myAcceleration;

private:
    std::vector<MSDevice*> myDevices;

    /** Iterator to current route-edge.  */
    MSRouteIterator myCurrEdge;

    /** The vehicle's allowed lanes on it'S current edge to drive
        according to it's route. */
    NextAllowedLanes myAllowedLanes;


    /// Default constructor.
    MSVehicle();

    /// Copy constructor.
    MSVehicle(const MSVehicle&);

    /// Assignment operator.
    MSVehicle& operator=(const MSVehicle&);

    struct DriveProcessItem {
        MSLink *myLink;
        SUMOReal myVLinkPass;
        SUMOReal myVLinkWait;
        DriveProcessItem(MSLink *link, SUMOReal vPass, SUMOReal vWait) :
                myLink(link), myVLinkPass(vPass), myVLinkWait(vWait) { };
    };

    typedef std::vector< DriveProcessItem > DriveItemVector;

    /// Container for used Links/visited Lanes during lookForward.
    DriveItemVector myLFLinkLanes;


    /// @name Move reminder structures
    /// @{

    /// @brief Definition of a move reminder container
    typedef std::vector< MSMoveReminder* > MoveReminderCont;

    /// @brief Current lane's move reminder
    MoveReminderCont myMoveReminders;

    /// @brief Prior lanes' move reminder
    MoveReminderCont myOldLaneMoveReminders;

    /// @brief Definition of a vector of offset to prior move reminder
    typedef std::vector<SUMOReal> OffsetVector;

    /// @brief Offsets for prior move reminder
    OffsetVector myOldLaneMoveReminderOffsets;
    /// @}



    typedef std::vector<MSVehicleQuitReminded*> QuitRemindedVector;
    QuitRemindedVector myQuitReminded;



    /**
     * @class RouteReplaceInfo
     * @brief Information about a replaced route
     *
     * Generated optionally and stored in a vector within the Pointer-CORN-map
     *  this structure contains information about a replaced route: the edge
     *  the route was replaced at by a new one, the time this was done, and
     *  the previous route.
     */
    class RouteReplaceInfo
    {
    public:
        /// Constructor
        RouteReplaceInfo(const MSEdge * const edge_, SUMOTime time_, MSRoute *route_)
                : edge(edge_), time(time_), route(route_) {}

        /// Destructor
        ~RouteReplaceInfo() { }

        /// The edge the vehicle was on when the route was replaced
        const MSEdge *edge;

        /// The time the route was replaced
        SUMOTime time;

        /// The prior route
        MSRoute *route;

    };

    /// Definition of the vector which stores information about replaced routes
    typedef std::vector<RouteReplaceInfo> ReplacedRoutesVector;


    struct DepartArrivalInformation {
        SUMOTime time;
        MSLane *lane;
        SUMOReal pos;
        SUMOReal speed;
    };

#ifndef NO_TRACI
    bool myHaveRouteInfo;
    typedef std::map<const MSEdge * const, Information *> InfoCont;
    InfoCont infoCont;

    /**
     * if true, indicates that a TraCI message "changeRoute" was sent to this vehicle,
     * thus it checks for a new route when the next simulation step is performed by TraCI
     */
    bool myWeightChangedViaTraci;

    /**
     * all edges in this list are marked as "changed by TracI", this means infoCont data
     * of this edge must not be changed except by a TraCI "changeRoute" message.
     * The corresponding Information value of the map holds the Information data
     * related to an edge before it was changed by TraCI
     */
    InfoCont edgesChangedByTraci;

    /* indicates whether the vehicle is adapting its speed caused by the TraCI command slowDown*/
    bool adaptingSpeed;
    bool isLastAdaption;

    /* speed of the vehicle before any speed adaption began */
    SUMOReal speedBeforeAdaption;

    /* the amount by wich the speed shall be reduced */
    SUMOReal speedReduction;

    /* simulation time, when the last speed adaption started */
    SUMOTime timeBeforeAdaption;

    /* duration of the last speed adaption */
    SUMOTime adaptDuration;

    /* simulation time when the last lane change was forced */
    SUMOTime timeBeforeLaneChange;

    /* duration for which the last lane change will be in effect */
    SUMOTime laneChangeStickyTime;

    /* lane index of the destination road map position for an active lane change*/
    unsigned myDestinationLane;

    /* true if any forced lane change is in effect*/
    bool laneChangeConstraintActive;

#endif

};


#endif

/****************************************************************************/

