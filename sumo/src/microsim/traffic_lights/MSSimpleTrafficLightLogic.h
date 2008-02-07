/****************************************************************************/
/// @file    MSSimpleTrafficLightLogic.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The basic traffic light logic
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
#ifndef MSSimpleTrafficLightLogic_h
#define MSSimpleTrafficLightLogic_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utility>
#include <vector>
#include <bitset>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include "MSTrafficLightLogic.h"
#include "MSPhaseDefinition.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSSimpleTrafficLightLogic
 * The implementation of a simple traffic light which only switches between
 * it's phases and sets the lights to red in between.
 * Some functions are called with an information about the current step. This
 * is needed as a single logic may be used by many junctions and so the current
 * step is stored within them, not within the logic.
 */
class MSSimpleTrafficLightLogic : public MSTrafficLightLogic
{
public:
    /// definition of a list of phases, being the junction logic
    typedef std::vector<MSPhaseDefinition*> Phases;

public:
    /// constructor
    MSSimpleTrafficLightLogic(MSNet &net, MSTLLogicControl &tlcontrol,
                              const std::string &id, const std::string &subid,
                              const Phases &phases, size_t step, size_t delay);

    /// destructor
    ~MSSimpleTrafficLightLogic();

    /** @brief Switches to the next phase
        Returns the time of the next switch */
    SUMOTime trySwitch(bool isActive);

    /// returns the current step
    size_t getStepNo() const;

    void setLinkPriorities() const;
    bool maskRedLinks() const;
    bool maskYellowLinks() const;
    MSPhaseDefinition getCurrentPhaseDef() const;

    /// returns the cycletime
    size_t getCycleTime() ;

    /// returns the position of the logic at the actual step of the simulation
    size_t getPosition(SUMOTime simStep);

    /**
     * Returns the position of the logic at any given simulation time
     * @return: the position of the logic
     */
    //size_t getPositionFromSimTime(SUMOTime time);

    /// returns the step (the phasenumber) of a given position of the cycle
    size_t getStepFromPos(size_t position);

    /// returns the position (start of a phase during a cycle) from of a given step
    size_t getPosFromStep(size_t step);

    /// Returns the phases of this tls
    const Phases &getPhases() const;

    /// Returns the phase of a given step
    const MSPhaseDefinition &getPhaseFromStep(size_t givenstep) const;

    void changeStepAndDuration(MSTLLogicControl &tlcontrol, SUMOTime simStep,
                               int step, SUMOTime stepDuration);

    std::string buildStateList() const;

protected:
    /// the list of phases this logic uses
    Phases myPhases;

    /// The current step
    size_t myStep;

    /// the cycletime
    size_t myCycleTime;

};


#endif

/****************************************************************************/

