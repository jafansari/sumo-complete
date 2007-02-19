/****************************************************************************/
/// @file    GUIEvent_SimulationEnded.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 19 Jun 2003
/// @version $Id$
///
// Event sent when the the simulation is over
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
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <fx.h>
#include "GUIEvent_Message.h"
#include "GUIEvent_SimulationEnded.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUIEvent_SimulationEnded::GUIEvent_SimulationEnded(EndReason reason,
        size_t step)
        : GUIEvent(EVENT_SIMULATION_ENDED),
        myReason(reason), myStep(step)
{}


GUIEvent_SimulationEnded::~GUIEvent_SimulationEnded()
{}


size_t
GUIEvent_SimulationEnded::getTimeStep() const
{
    return myStep;
}


GUIEvent_SimulationEnded::EndReason
GUIEvent_SimulationEnded::getReason() const
{
    return myReason;
}



/****************************************************************************/

