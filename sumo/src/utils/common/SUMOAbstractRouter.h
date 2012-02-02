/****************************************************************************/
/// @file    SUMOAbstractRouter.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    25.Jan 2006
/// @version $Id$
///
// The dijkstra-router
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
#ifndef SUMOAbstractRouter_h
#define SUMOAbstractRouter_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMOAbstractRouter
 * The interface for routing the vehicles over the network.
 */
template<class E, class V>
class SUMOAbstractRouter {
public:
    /// Constructor
    SUMOAbstractRouter() { }

    /// Destructor
    virtual ~SUMOAbstractRouter() { }

    /** @brief Builds the route between the given edges using the minimum effort at the given time
        The definition of the effort depends on the wished routing scheme */
    virtual void compute(const E* from, const E* to, const V* const vehicle,
                         SUMOTime msTime, std::vector<const E*> &into) = 0;

    virtual SUMOReal recomputeCosts(const std::vector<const E*> &edges,
                                    const V* const v, SUMOTime msTime) const = 0;

    // interface extension for BulkStarRouter
    virtual void prepare(const E* edge, const V* vehicle) {
        assert(false);
    }
};


template<class E, class V>
struct prohibited_withRestrictions {
public:
    inline bool operator()(const E* edge, const V* vehicle) const {
        if (std::find(myProhibited.begin(), myProhibited.end(), edge) != myProhibited.end()) {
            return true;
        }
        return edge->prohibits(vehicle);
    }

    void prohibit(const std::vector<E*> &toProhibit) {
        myProhibited = toProhibit;
    }

protected:
    std::vector<E*> myProhibited;

};

template<class E, class V>
struct prohibited_noRestrictions {
public:
    inline bool operator()(const E*, const V*) const {
        return false;
    }
};




#endif

/****************************************************************************/

