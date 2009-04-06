/****************************************************************************/
/// @file    MSOffTrafficLightLogic.h
/// @author  Daniel Krajzewicz
/// @date    08.05.2007
/// @version $Id$
///
// A traffic lights logic which represents a tls in an off-mode
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef MSOffTrafficLightLogic_h
#define MSOffTrafficLightLogic_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


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
 * @class MSOffTrafficLightLogic
 * @brief A traffic lights logic which represents a tls in an off-mode
 */
class MSOffTrafficLightLogic : public MSTrafficLightLogic {
public:
    /// definition of a list of phases, being the junction logic
    typedef std::vector<MSPhaseDefinition*> Phases;

public:
    /// constructor
    MSOffTrafficLightLogic(MSTLLogicControl &tlcontrol,
                           const std::string &id);

    /// destructor
    ~MSOffTrafficLightLogic();

    /** @brief Switches to the next phase
        Returns the time of the next switch */
    SUMOTime trySwitch(bool isActive);

    /// Returns the priorities for all lanes for the current phase
    const std::bitset<64> &linkPriorities() const;

    /// Returns a bitset where all links having yellow are set
    const std::bitset<64> &yellowMask() const;

    const std::bitset<64> &allowed() const;

    /// Returns the number of phases
    unsigned int getPhaseNumber() const;

    /// returns the current step
    size_t getCurrentPhaseIndex() const;

    /// returns the cycletime
    size_t getCycleTime() ;

    /// returns the position of the logic at the actual step of the simulation
    size_t getPosition(SUMOTime simStep);

    /// returns the step (the phasenumber) of a given position of the cycle
    unsigned int getStepFromPos(unsigned int position);

    /// returns the position (start of a phase during a cycle) from of a given step
    size_t getPosFromStep(size_t step);

    /// Returns the phases of this tls
    const Phases &getPhases() const;

    /// Returns the phase of a given step
    const MSPhaseDefinition &getPhaseFromStep(size_t givenstep) const;

    void setLinkPriorities() const;
    bool maskRedLinks() const;
    bool maskYellowLinks() const;
    MSPhaseDefinition getCurrentPhaseDef() const;
    void changeStepAndDuration(MSTLLogicControl &tlcontrol, SUMOTime simStep, unsigned int step, SUMOTime stepDuration) {
    }
    std::string buildStateList() const;
};


#endif

/****************************************************************************/

