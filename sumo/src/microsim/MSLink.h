/****************************************************************************/
/// @file    MSLink.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A connnection between lanes
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
#ifndef MSLink_h
#define MSLink_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSLogicJunction.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSTrafficLightLogic;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLinks
 * @brief A connnection between lanes
 *
 * Each link lies within a junction.
 *
 * A link is basically a connection between two lanes, stored within the
 *  originating (the one that is being left) lane and pointing to the
 *  approached lane. When using inner-junction simulation, additionally
 *  a "via-lane" is stored, the one that is used to cross the junction
 *  and which represents the links shape.
 *
 * Because right-of-way rules are controlled by the junctions, the link
 *  stores the information about where to write information about approaching
 *  vehicles (the "request") and where to get the information whether the
 *  vehicle really may drive (the "respond").
 *
 * Because a link is a connection over a junction, it basically also has a
 *  length. This length is needed to assure that vehicles have the correct halting
 *  distance before approaching the link.
 *
 * @todo Check whether links with via-lanes should be subclassed
 */
class MSLink
{
public:
    /**
     * @enum LinkState
     * @brief The right-of-way state of this link
     *
     * This enumerations holds the possible right-of-way rules a link
     *  may have. Beyond the righ-of-way rules, this enumeration also
     *  holds the possible traffic light states.
     */
    enum LinkState {
        /// The link has green light
        LINKSTATE_TL_GREEN,
        /// The link has red light
        LINKSTATE_TL_RED,
        /// The link has yellow light
        LINKSTATE_TL_YELLOW,
        /// The link is controlled by a tls which is off and blinks
        LINKSTATE_TL_OFF_BLINKING,
        /// The link is controlled by a tls which is off, not blinking
        LINKSTATE_TL_OFF_NOSIGNAL,
        /// This is an uncontrolled, major link
        LINKSTATE_MAJOR,
        /// This is an uncontrolled, minor link
        LINKSTATE_MINOR,
        /// This is an uncontrolled, right-before-left link
        LINKSTATE_EQUAL,
        /// This is a dead end link
        LINKSTATE_DEADEND
    };


    /**
     * @enum LinkDirection
     * @brief The different directions a link may take.
     */
    enum LinkDirection {
        /// The link is a straight direction
        LINKDIR_STRAIGHT = 0,
        /// The link is a 180 degree turn
        LINKDIR_TURN,
        /// The link is a (hard) left direction
        LINKDIR_LEFT,
        /// The link is a (hard) right direction
        LINKDIR_RIGHT,
        /// The link is a partial left direction
        LINKDIR_PARTLEFT,
        /// The link is a partial right direction
        LINKDIR_PARTRIGHT,
        /// The link has no direction (is a dead end link)
        LINKDIR_NODIR
    };


#ifndef HAVE_INTERNAL_LANES
    /** @brief Constructor for simulation not using internal lanes
     *
     * @param[in] succLane The lane approached by this link
     * @param[in] yield Information whether vehicles have to decelerate in front of this link
     * @param[in] dir The direction of this link
     * @param[in] state The state of this link
     */
    MSLink(MSLane* succLane,
           bool yield, LinkDirection dir, LinkState state, SUMOReal length) throw();
#else
    /** @brief Constructor for simulation which uses internal lanes
    *
    * @param[in] succLane The lane approached by this link
    * @param[in] via The lane to use within the junction
    * @param[in] yield Information whether vehicles have to decelerate in front of this link
    * @param[in] dir The direction of this link
    * @param[in] state The state of this link
    * @param[in] internalEnd Information whether this link is followed by a second junction-internal link
    */
    MSLink(MSLane* succLane, MSLane *via,
           bool yield, LinkDirection dir, LinkState state, bool internalEnd,
           SUMOReal length) throw();
#endif

    /// @brief Destructor
    ~MSLink() throw();

    /** @brief Sets the request information
     *
     * Because traffic lights and junction logics are loaded after links,
     *  we have to assign the information about the right-of-way
     *  requests and responses after the initialisation.
     *
     * @param[in] request The request of this link
     * @param[in] requestIdx This link's index within this request
     * @param[in] respond The respond of this link
     * @param[in] respondIdx This link's index within this respond
     * @todo Unsecure!
     */
    void setRequestInformation(
        MSLogicJunction::Request *request, size_t requestIdx,
        MSLogicJunction::Respond *respond, size_t respondIdx) throw();


    /** @brief Sets the information about an approaching vehicle
     *
     * Stores the approaching vehicle in myApproaching, sets the information
     *  that a vehicle is approaching in request.
     *
     * @param[in] approaching The approaching vehicle
     */
    void setApproaching(MSVehicle *approaching) throw();


    /** @brief Returns the approaching vehicle
     *
     * May be 0 if no vehicle is approaching.
     *
     * @return The approaching vehicle if any, 0 otherwise
     * @todo Unsecure!
     */
    MSVehicle *getApproaching() const throw() {
        return myApproaching;
    };


    /** @brief Resets this link priority (information whether it is yield)
     *
     * Some Junctions need to switch the priority.
     *
     * @param[in] prio The current priority of the link
     */
    void setPriority(bool prio) throw();


    /** @brief Returns the information whether the link may be passed
     *
     * Valid after the junctions have set their reponds
     *
     * @return Whether this link may be passed.
     */
    bool opened() const throw();


    /** @brief Removes the incoming vehicle's request
     *
     * Necessary to mask out vehicles having yellow or red.
     */
    void deleteRequest() throw();


    /** @brief Returns the current state of the link
     *
     * @return The current state of this link
     */
    LinkState getState() const throw();


    /** @brief Returns the direction the vehicle passing this link take
     *
     * @return The direction of this link
     */
    LinkDirection getDirection() const throw();


    /** @brief Sets the current tl-state
     *
     * @param[in] state The current state of the link
     */
    void setTLState(LinkState state) throw();


    /** @brief Returns the connected lane
     *
     * @return The lane approached by this link
     */
    MSLane *getLane() const throw();


    /** @brief Returns the respond index (for visualization)
     *
     * @return The respond index for this link
     */
    size_t getRespondIndex() const throw();


    /** @brief Returns whether this link is a major lin
     *
     * @return Whether the link has a large priority
     */
    bool havePriority() const throw();


    /** @brief Returns the length of this link
     *
     * @return The length of this link
     */
    SUMOReal getLength() const throw() {
        return myLength;
    }


#ifdef HAVE_INTERNAL_LANES
    /** @brief Returns the following inner lane
     *
     * @return The inner lane to use to cross the junction
     */
    MSLane * const getViaLane() const throw();


    /** @brief Checks whether the link may drive
     *
     * Use for inner-lane simulation. Has to be rechecked/described
     *
     * @todo Recheck/describe
     */
    void resetInternalPriority() throw();

#endif

private:
    /// @brief The lane approached by this link
    MSLane* myLane;

    /// @brief The right of way
    bool myPrio;

    /// @brief The approaching vehicle
    MSVehicle *myApproaching;

    /// @brief The request to set incoming request into
    MSLogicJunction::Request *myRequest;

    /// @brief The position of the link within this request
    size_t myRequestIdx;

    /// @brief The respond to read whether the car may drive from
    MSLogicJunction::Respond *myRespond;

    /// @brief The position within this respond
    size_t myRespondIdx;

    /// @brief The state of the link
    LinkState myState;

    /// @brief An abstract (hopefully human readable) definition of the link's direction
    LinkDirection myDirection;

    /// @brief The length of the link
    SUMOReal myLength;


#ifdef HAVE_INTERNAL_LANES
    /// @brief The following junction-internal lane if used
    MSLane * const myJunctionInlane;

    /// @brief Information whether this link is followed by a second junction-internal link
    bool myIsInternalEnd;
#endif


private:
    /// invalidated copy constructor
    MSLink(const MSLink &s);

    /// invalidated assignment operator
    MSLink &operator=(const MSLink &s);

};


#endif

/****************************************************************************/

