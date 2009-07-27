/****************************************************************************/
/// @file    ROHelper.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Some helping methods for router
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
#ifndef ROHelper_h
#define ROHelper_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <functional>
#include <vector>
#include "ROEdge.h"
#include "ROVehicle.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROVehicleByDepartureComperator
 * @brief A function for sorting vehicles by their departure time
 *
 * In the case two vehicles have the same departure time, they are sorted
 *  lexically.
 */
class ROVehicleByDepartureComperator : public std::less<ROVehicle*> {
public:
    /// @brief Constructor
    explicit ROVehicleByDepartureComperator() throw() { }

    /// @brief Destructor
    ~ROVehicleByDepartureComperator() throw() { }

    /** @brief Comparing operator
     *
     * Returns whether the first vehicles wants to leave later than the second.
     *  If both vehicles have the same departure time, a lexical comparison is
     *  done.
     *
     * @param[i] veh1 The first vehicle to compare
     * @param[i] veh2 The second vehicle to compare
     * @return Whether the first vehicle departs later than the second
     * @todo Check whether both vehicles can be const
     */
    bool operator()(ROVehicle *veh1, ROVehicle *veh2) const throw() {
        if (veh1->getDepartureTime()==veh2->getDepartureTime()) {
            return veh1->getID()>veh2->getID();
        }
        return veh1->getDepartureTime()>veh2->getDepartureTime();
    }
};


/**
 * @namespace ROHelper
 * @brief Some helping methods for router
 */
namespace ROHelper {
/** @brief Computes the costs (travel time) for the given list of edges
 *
 * @param[in] edges The edges to pass
 * @param[in] v The vehicle that passes the edges
 * @param[in] time The time at which the vehicle begins to pass the edges
 * @return The time needed by the vehicle to pass the edges (completely)
 * @todo Check whether vehicle's max speed is considered
 */
/*
SUMOReal recomputeCosts(SUMOAbstractRouter<ROEdge,ROVehicle> &router,
                    const std::vector<const ROEdge*> &edges,
                    const ROVehicle * const v, SUMOTime time) throw();
*/

/** @brief Checks whether the given edge list contains loops and removes them
 *
 * @param[in] edges The list of edges to remove loops from
 */
void recheckForLoops(std::vector<const ROEdge*> &edges) throw();

}


/// @brief Output operator for edge lists (edges will be written as their ids, divided by spaces)
std::ostream &operator<<(std::ostream &os, const std::vector<const ROEdge*> &ev);



#endif

/****************************************************************************/

