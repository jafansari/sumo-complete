/****************************************************************************/
/// @file    RODFEdge.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// An edge within the DFROUTER
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
#ifndef RODFEdge_h
#define RODFEdge_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
#include <router/ROEdge.h>
#include <utils/geom/Position2D.h>
#include "RODFDetectorFlow.h"


// ===========================================================================
// class declarations
// ===========================================================================
class ROLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFEdge
 */
class RODFEdge : public ROEdge
{
public:
    /// Constructor
    RODFEdge(const std::string &id, int index);

    /// Desturctor
    ~RODFEdge();

    void setFlows(const std::vector<FlowDef> &flows);

    const std::vector<FlowDef> &getFlows() const;

    void setFromPosition(const Position2D &p);
    void setToPosition(const Position2D &p);

    const Position2D &getFromPosition() const;
    const Position2D &getToPosition() const;

private:
    std::vector<FlowDef> myFlows;

    Position2D myFromPosition, myToPosition;

private:
    /// we made the copy constructor invalid
    RODFEdge(const RODFEdge &src);

    /// we made the assignment operator invalid
    RODFEdge &operator=(const RODFEdge &src);

};


#endif

/****************************************************************************/

