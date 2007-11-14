/****************************************************************************/
/// @file    RORouteDef_Complete.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A complete route definition (with all passed edges being known)
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
#ifndef RORouteDef_Complete_h
#define RORouteDef_Complete_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include "ROEdgeVector.h"
#include "RORouteDef.h"


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RORoute;
class ROVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORouteDef_Complete
 * @brief A complete route description containing all edges the driver will pass.
 */
class RORouteDef_Complete :
            public RORouteDef
{
public:
    /// Constructor
	RORouteDef_Complete(const std::string &id, const std::string &color,
                        const ROEdgeVector &edges);

    /// Destructor
    virtual ~RORouteDef_Complete();

    /// returns the begin of the trip
    const ROEdge * const getFrom() const;

    /// Returns the end of the trip
    const ROEdge * const getTo() const;

    /// Builds the route
    RORoute *buildCurrentRoute(ROAbstractRouter &router, SUMOTime begin,
                               const ROVehicle &veh) const;

    /** @brief Adds an route alternative (see further comments)
    *
     * Here, as in most cases, the alternative is the route that was build
     * as last and will stay the only route known */
    void addAlternative(const ROVehicle *const, RORoute *current, SUMOTime begin);

    /** @brief Returns a copy of the route definition */
    RORouteDef *copy(const std::string &id) const;

    const ROEdgeVector &getCurrentEdgeVector() const;

protected:
    /// The list of edges the driver passes
    ROEdgeVector myEdges;

    /// The begin of the route
    SUMOTime myStartTime;

};


#endif

/****************************************************************************/

