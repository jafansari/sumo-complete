/****************************************************************************/
/// @file    DFRORouteDesc.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// A route within the DFROUTER
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
#ifndef DFRORouteDesc_h
#define DFRORouteDesc_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <string>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RODFDetector;


// ===========================================================================
// struct definitions
// ===========================================================================
/**
 * @struct DFRORouteDesc
 * @brief A route within the DFROUTER
 */
struct DFRORouteDesc {
    /// @brief The edges the route is made of
    std::vector<ROEdge*> edges2Pass;
    /// @brief The name of the route
    std::string routename;
    SUMOReal duration_2;
    SUMOReal distance;
    int passedNo;
    const ROEdge *endDetectorEdge;
    const ROEdge *lastDetectorEdge;
    SUMOReal distance2Last;
    SUMOTime duration2Last;

    SUMOReal overallProb;
    SUMOReal factor;

};


#endif

/****************************************************************************/

