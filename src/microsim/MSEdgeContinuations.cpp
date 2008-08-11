/****************************************************************************/
/// @file    MSEdgeContinuations.cpp
/// @author  Daniel Krajzewicz
/// @date    2005-11-09
/// @version $Id$
///
// Stores predeccessor-successor-relations of MSEdges.
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <map>
#include <vector>
#include "MSEdgeContinuations.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
MSEdgeContinuations::MSEdgeContinuations() throw()
{}


MSEdgeContinuations::~MSEdgeContinuations() throw()
{}


void
MSEdgeContinuations::add(const MSEdge *const to, const MSEdge *const from) throw()
{
    if (myContinuations.find(to)==myContinuations.end()) {
        myContinuations[to] = vector<const MSEdge *const>();
    }
    myContinuations[to].push_back(from);
}


const std::vector<const MSEdge *const> &
MSEdgeContinuations::getInFrontOfEdge(const MSEdge &toEdge) const throw()
{
    return myContinuations.find(static_cast<MSEdge*>(& ((MSEdge&) toEdge)))->second;
}


bool
MSEdgeContinuations::hasFurther(const MSEdge &toEdge) const throw()
{
    return myContinuations.find(static_cast<MSEdge*>(& ((MSEdge&) toEdge)))!=myContinuations.end();
}



/****************************************************************************/

