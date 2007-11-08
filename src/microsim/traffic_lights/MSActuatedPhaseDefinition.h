/****************************************************************************/
/// @file    MSActuatedPhaseDefinition.h
/// @author  Daniel Krajzewicz
/// @date    Jan 2004
/// @version $Id$
///
// The definition of a single phase of an extended tls logic
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
#ifndef MSActuatedPhaseDefinition_h
#define MSActuatedPhaseDefinition_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSPhaseDefinition.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSActuatedPhaseDefinition
 * The definition of a single phase of an extended tls logic.
 * We moved it out of the main class to allow later template
 * parametrisation.
 */
class MSActuatedPhaseDefinition : public MSPhaseDefinition
{
public:
    /// The minimum duration of the phase
    SUMOTime minDuration;

    /// The maximum duration of the phase
    SUMOTime maxDuration;

    /// constructor
    MSActuatedPhaseDefinition(size_t durationArg,
                              const std::bitset<64> &driveMaskArg, const std::bitset<64> &breakMaskArg,
                              const std::bitset<64> &yellowMaskArg,
                              int minDurationArg, int maxDurationArg)
            : MSPhaseDefinition(durationArg, driveMaskArg,
                                breakMaskArg, yellowMaskArg),
            minDuration(minDurationArg), maxDuration(maxDurationArg) {
        // defines minDuration
        size_t minDurationDefault = 10;
        if (minDurationArg < 0) {
            if (durationArg < minDurationDefault) {
                minDuration = durationArg;
            } else {
                minDuration = minDurationDefault;
            }
        } else {
            minDuration = minDurationArg;
        }
        // defines maxDuration (maxDuration is only used in MSActuatedTraffifLight Logic)
        size_t maxDurationDefault = 30;
        if (maxDurationArg < 0) {
            if (durationArg > maxDurationDefault) {
                maxDuration = durationArg;
            } else {
                maxDuration = maxDurationDefault;
            }
        } else {
            maxDuration = maxDurationArg;
        }
    }

    /// destructor
    ~MSActuatedPhaseDefinition() { }

};


#endif

/****************************************************************************/

