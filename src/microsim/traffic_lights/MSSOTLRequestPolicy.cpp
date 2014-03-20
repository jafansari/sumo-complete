/****************************************************************************/
/// @file    MSSOTLRequestPolicy.cpp
/// @author  Gianfilippo Slager
/// @author  Anna Chiara Bellini 
/// @date    Apr 2013
/// @version $Id: MSSOTLRequestPolicy.cpp 2 2013-04-05 15:00:00Z acbellini $
///
// The class for SOTL Request logics
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

#include "MSSOTLRequestPolicy.h"

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
		const std::map<std::string, std::string>& parameters) :
		MSSOTLPolicy("Request", parameters) {
}

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
		MSSOTLPolicyDesirability *desirabilityAlgorithm) :
		MSSOTLPolicy("Request", desirabilityAlgorithm) {
	getDesirabilityAlgorithm()->setKeyPrefix("REQUEST");
}

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
		MSSOTLPolicyDesirability *desirabilityAlgorithm,
		const std::map<std::string, std::string>& parameters) :
		MSSOTLPolicy("Request", desirabilityAlgorithm, parameters) {
	getDesirabilityAlgorithm()->setKeyPrefix("REQUEST");

}

bool MSSOTLRequestPolicy::canRelease(int elapsed, bool thresholdPassed,
		const MSPhaseDefinition* stage, int vehicleCount) {
	if (elapsed >= getMinDecisionalPhaseDuration()) {
		return thresholdPassed;
	}
	return false;
}
