/****************************************************************************/
/// @file    RODUAEdgeBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Jan 2004
/// @version $Id$
///
// Interface for building instances of duarouter-edges
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "RODUAEdgeBuilder.h"
#include <router/ROEdge.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
RODUAEdgeBuilder::RODUAEdgeBuilder(bool useBoundariesOnOverride, bool interpolate) throw()
        : myUseBoundariesOnOverride(useBoundariesOnOverride), myInterpolate(interpolate) {}


RODUAEdgeBuilder::~RODUAEdgeBuilder() throw() {}


ROEdge *
RODUAEdgeBuilder::buildEdge(const std::string &name, RONode *from, RONode *to) throw() {
    return new ROEdge(name, from, to, getNextIndex(), myUseBoundariesOnOverride, myInterpolate);
}


/****************************************************************************/

