/****************************************************************************/
/// @file    SUMOVehicle.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 17 Feb 2009
/// @version $Id$
///
// Abstract base class for vehicle representations
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef SUMOVehicle_h
#define SUMOVehicle_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOAbstractRouter.h>
#include <utils/common/SUMOVehicleParameter.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicleType;
class MSRoute;
class MSEdge;
class MSLane;
class MSDevice;
class MSPerson;

typedef std::vector<const MSEdge*> MSEdgeVector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMOVehicle
 * @brief Representation of a vehicle
 */
class SUMOVehicle {
public:
    /// @brief Destructor
    virtual ~SUMOVehicle() {}

    /** @brief Get the vehicle's ID
     * @return The the ID of the vehicle
     */
    virtual const std::string& getID() const = 0;

    /** @brief Get the vehicle's position along the lane
     * @return The position of the vehicle (in m from the lane's begin)
     */
    virtual SUMOReal getPositionOnLane() const = 0;

    /** @brief Returns the vehicle's maximum speed
     * @return The vehicle's maximum speed
     */
    virtual SUMOReal getMaxSpeed() const = 0;

    /** @brief Returns the vehicle's current speed
     * @return The vehicle's speed
     */
    virtual SUMOReal getSpeed() const = 0;

    /** @brief Returns the vehicle's type
     * @return The vehicle's type
     */
    virtual const MSVehicleType& getVehicleType() const = 0;

    /// Returns the current route
    virtual const MSRoute& getRoute() const = 0;

    /** @brief Returns the nSuccs'th successor of edge the vehicle is currently at
     *
     * If the rest of the route (counted from the current edge) than nSuccs,
     *  0 is returned.
     * @param[in] nSuccs The number of edge to look forward
     * @return The nSuccs'th following edge in the vehicle's route
     */
    virtual const MSEdge* succEdge(unsigned int nSuccs) const = 0;

    /// Replaces the current route by the given edges
    virtual bool replaceRouteEdges(const MSEdgeVector& edges, bool onInit = false) = 0;

    /// Replaces the current route by the given one
    virtual bool replaceRoute(const MSRoute* route, bool onInit = false) = 0;

    /** @brief Performs a rerouting using the given router
     *
     * Tries to find a new route between the current edge and the destination edge, first.
     * Tries to replace the current route by the new one using replaceRoute.
     *
     * @param[in] t The time for which the route is computed
     * @param[in] router The router to use
     * @see replaceRoute
     */
    virtual void reroute(SUMOTime t, SUMOAbstractRouter<MSEdge, SUMOVehicle> &router, bool withTaz = false) = 0;

    /** @brief Returns the vehicle's acceleration
     * @return The acceleration
     */
    virtual SUMOReal getAcceleration() const = 0;

    /** @brief Returns the edge the vehicle is currently at
     *
     * @return The current edge in the vehicle's route
     */
    virtual const MSEdge* getEdge() const = 0;

    /** @brief Returns the vehicle's parameter (including departure definition)
     *
     * @return The vehicle's parameter
     */
    virtual const SUMOVehicleParameter& getParameter() const = 0;

    /** @brief Called when the vehicle is inserted into the network
     *
     * Sets optional information about departure time, informs the vehicle
     *  control about a further running vehicle.
     */
    virtual void onDepart() = 0;

    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    virtual bool isOnRoad() const = 0;

    /** @brief Returns this vehicle's real departure time
     * @return This vehicle's real departure time
     */
    virtual SUMOTime getDeparture() const = 0;

    /** @brief Returns whether this vehicle has departed
     */
    virtual bool hasDeparted() const = 0;

    /** @brief Returns the number of new routes this vehicle got
     * @return the number of new routes this vehicle got
     */
    virtual unsigned int getNumberReroutes() const = 0;

    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    virtual const std::vector<MSDevice*> &getDevices() const = 0;

    /** @brief Adds a person to this vehicle
     *
     * May do nothing since persons are not supported by default
     *
     * @param[in] person The person to add
     */
    virtual void addPerson(MSPerson* person) = 0;

    /** @brief Adds a stop
     *
     * The stop is put into the sorted list.
     * @param[in] stop The stop to add
     * @return Whether the stop could be added
     */
    virtual bool addStop(const SUMOVehicleParameter::Stop& stopPar, SUMOTime untilOffset=0) = 0;

    /** @brief Returns whether the vehicle is at a stop
     * @return Whether the has stopped
     */
    virtual bool isStopped() const = 0;


    virtual SUMOReal getChosenSpeedFactor() const = 0;
};


#endif

/****************************************************************************/