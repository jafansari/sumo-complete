/****************************************************************************/
/// @file    MSSOTLPlatoonPolicy.h
/// @author  Gianfilippo Slager
/// @date    Feb 2010
/// @version $Id: MSSOTLPlatoonPolicy.h 0 2010-02-18 12:40:00Z gslager $
///
// The class for SOTL Platoon logics
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
#ifndef MSTLPlatoonPolicy_h
#define MSTLPlatoonPolicy_h

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include "MSSOTLPolicy.h"
/**
 * @class MSSOTLPlatoonPolicy
 * @brief Class for low-level platoon policy.
 *
 */
class MSSOTLPlatoonPolicy: public MSSOTLPolicy {

public:
	MSSOTLPlatoonPolicy(const std::map<std::string, std::string>& parameters);
	MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability *desirabilityAlgorithm);

	MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability *desirabilityAlgorithm,
			const std::map<std::string, std::string>& parameters);

	bool canRelease(int elapsed, bool thresholdPassed,
			const MSPhaseDefinition* stage, int vehicleCount);


};

#endif
/****************************************************************************/
