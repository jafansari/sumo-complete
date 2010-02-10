/****************************************************************************/
/// @file    RORouteDef_OrigDest.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A route where only the origin and the destination edges are known
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef RORouteDef_OrigDest_h
#define RORouteDef_OrigDest_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include "RORouteDef.h"
#include <utils/common/RGBColor.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RORoute;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORouteDef_OrigDest
 * A route definition where only the begin and the end edge are given.
 */
class RORouteDef_OrigDest
            : public RORouteDef {
public:
    /// Constructor
    RORouteDef_OrigDest(const std::string &id, const RGBColor * const color,
                        const ROEdge *from, const ROEdge *to, bool removeFirst=false) throw();

    /// Destructor
    virtual ~RORouteDef_OrigDest() throw();

    /// Builds the current route from the given information (perform routing, here)
    RORoute *buildCurrentRoute(SUMOAbstractRouter<ROEdge,ROVehicle> &router, SUMOTime begin,
                               const ROVehicle &veh) const;

    /** @brief Adds the build route to the container
    *
     * Here, the currently new route is added */
    void addAlternative(SUMOAbstractRouter<ROEdge,ROVehicle> &router,
                        const ROVehicle *const, RORoute *current, SUMOTime begin);

    /** @brief Returns a copy of the route definition */
    RORouteDef *copy(const std::string &id) const;

    virtual OutputDevice &writeXMLDefinition(SUMOAbstractRouter<ROEdge,ROVehicle> &router,
            OutputDevice &dev, const ROVehicle * const veh, bool asAlternatives, bool withExitTimes) const;

protected:
    /// The origin and the destination edge of the route
    const ROEdge *myFrom, *myTo;

    /// The complete route (after building)
    RORoute *myCurrent;

    /// The begin of the trip
    SUMOTime myStartTime;

    /** @brief Information whether the first edge shall be removed
        */
    bool myRemoveFirst;


private:
    /// @brief Invalidated copy constructor
    RORouteDef_OrigDest(const RORouteDef_OrigDest &src);

    /// @brief Invalidated assignment operator
    RORouteDef_OrigDest &operator=(const RORouteDef_OrigDest &src);

};


#endif

/****************************************************************************/

