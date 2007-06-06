/****************************************************************************/
/// @file    GUIEvent_SimulationEnded.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 19 Jun 2003
/// @version $Id$
///
// Event send when the the simulation is over
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
#ifndef GUIEvent_SimulationEnded_h
#define GUIEvent_SimulationEnded_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "GUIEvent.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * GUIEvent_SimulationEnded
 * Throw from GUIRunThread to GUIApplicationWindow and then further to all
 * displays after a step has been performed
 */
class GUIEvent_SimulationEnded : public GUIEvent
{
public:
    enum EndReason {
        /// the simulation has ended as all vehicles have left it
        ER_NO_VEHICLES,

        /// The simulation has ended as the end time step was reached
        ER_END_STEP_REACHED,

        /// The simulation has ended due to an error
        ER_ERROR_IN_SIM,

        /// Someone else has forced the simulation to quit
        ER_FORCED,

        /// The simulation got too slow
        ER_TOO_SLOW

    };

    /// constructor
    GUIEvent_SimulationEnded(EndReason reason, size_t step);

    /// destructor
    ~GUIEvent_SimulationEnded();

    /// Returns the time step the simulation has ended at
    size_t getTimeStep() const;

    /// Returns the reason the simulation has ended due
    EndReason getReason() const;

protected:
    /// The reason the simulation has ended
    EndReason myReason;

    /// The time step the simulation has ended at
    size_t myStep;

};


#endif

/****************************************************************************/

