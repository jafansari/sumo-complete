/****************************************************************************/
/// @file    MSOffTrafficLightLogic.cpp
/// @author  Daniel Krajzewicz
/// @date    08.05.2007
/// @version $Id: MSOffTrafficLightLogic.cpp 3486 2007-02-19 13:38:00 +0100 (Mo, 19 Feb 2007) behrisch $
///
// A traffic lights logic which represents a tls in an off-mode
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

#include <cassert>
#include <utility>
#include <vector>
#include <bitset>
#include <sstream>
#include <microsim/MSEventControl.h>
#include "MSTrafficLightLogic.h"
#include "MSOffTrafficLightLogic.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
MSOffTrafficLightLogic::MSOffTrafficLightLogic(MSTLLogicControl &tlcontrol,
        const std::string &id)
        : MSTrafficLightLogic(tlcontrol, id, "off", 0)
{
}


MSOffTrafficLightLogic::~MSOffTrafficLightLogic()
{
}


SUMOTime
MSOffTrafficLightLogic::trySwitch(bool)
{
    return 0;
}


size_t
MSOffTrafficLightLogic::getStepNo() const
{
    return 0;
}

size_t
MSOffTrafficLightLogic::getCycleTime()
{
    return 0;
}

size_t
MSOffTrafficLightLogic::getPosition(SUMOTime simStep)
{
    return 0;
}

size_t
MSOffTrafficLightLogic::getStepFromPos(size_t position)
{
    return 0;
}

size_t
MSOffTrafficLightLogic::getPosFromStep(size_t step)
{
    return 0;
}


const MSOffTrafficLightLogic::Phases &
MSOffTrafficLightLogic::getPhases() const
{
    throw 1;
}

const MSPhaseDefinition &
MSOffTrafficLightLogic::getPhaseFromStep(size_t givenStep) const
{
    throw 1;
}


void
MSOffTrafficLightLogic::setLinkPriorities() const
{
}


bool
MSOffTrafficLightLogic::maskRedLinks() const
{
    return true;
}


bool
MSOffTrafficLightLogic::maskYellowLinks() const
{
    return true;
}


MSPhaseDefinition
MSOffTrafficLightLogic::getCurrentPhaseDef() const
{
    size_t no = getLinks().size();
    std::bitset<64> driveMaskArg;
    std::bitset<64> breakMaskArg;
    std::bitset<64> yellowMaskArg;
    for (size_t i=0; i<no; ++i) {
        driveMaskArg[i] = true;
        breakMaskArg[i] = false;
        yellowMaskArg[i] = false;
    }
    return MSPhaseDefinition(-1, driveMaskArg, breakMaskArg, yellowMaskArg);
}


std::string
MSOffTrafficLightLogic::buildStateList() const
{
    std::ostringstream strm;
    size_t no = getLinks().size();
    for (size_t i=0; i<no; ++i) {
        strm << 'O';
    }
    return strm.str();
}



/****************************************************************************/

