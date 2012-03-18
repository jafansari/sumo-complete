/****************************************************************************/
/// @file    RORouteDef_OrigDest.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id$
///
// A route where only the origin and the destination edges are known
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
    RORouteDef_OrigDest(const std::string& id, const RGBColor* const color,
                        const ROEdge* from, const ROEdge* to, bool removeFirst = false) ;

    /// Destructor
    virtual ~RORouteDef_OrigDest() ;

    /// Builds the current route from the given information (perform routing, here)
    void preComputeCurrentRoute(SUMOAbstractRouter<ROEdge, ROVehicle> &router, SUMOTime begin,
                               const ROVehicle& veh) const;

    /** @brief Adds the build route to the container
    *
     * Here, the currently new route is added */
    void addAlternative(SUMOAbstractRouter<ROEdge, ROVehicle> &router,
                        const ROVehicle* const, RORoute* current, SUMOTime begin);

    /** @brief Returns a copy of the route definition */
    RORouteDef* copy(const std::string& id) const;

private:
    /// @brief Invalidated copy constructor
    RORouteDef_OrigDest(const RORouteDef_OrigDest& src);

    /// @brief Invalidated assignment operator
    RORouteDef_OrigDest& operator=(const RORouteDef_OrigDest& src);

};


#endif

/****************************************************************************/

