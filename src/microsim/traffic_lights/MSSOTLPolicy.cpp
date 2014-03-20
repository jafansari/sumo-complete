/****************************************************************************/
/// @file    MSSOTLPolicy.cpp
/// @author  Alessio Bonfietti
/// @author  Anna Chiara Bellini
/// @date    Jun 2013
/// @version $Id: MSSOTLPolicy.cpp 0  $
///
// The class for low-level policy
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2013 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/

#include "MSSOTLPolicy.h"

MSSOTLPolicy::MSSOTLPolicy(string name,
		const std::map<std::string, std::string>& parameters) :
		Parameterised(parameters), myName(name) {
	theta_sensitivity = 0;

}

MSSOTLPolicy::MSSOTLPolicy(string name,
		MSSOTLPolicyDesirability *desirabilityAlgorithm) :
		Parameterised(), myName(name), myDesirabilityAlgorithm(
				desirabilityAlgorithm) {
	theta_sensitivity = 0;

}

MSSOTLPolicy::MSSOTLPolicy(string name,
		MSSOTLPolicyDesirability *desirabilityAlgorithm,
		const std::map<std::string, std::string>& parameters) :
		Parameterised(parameters), myName(name), myDesirabilityAlgorithm(
				desirabilityAlgorithm) {

	std::ostringstream key;
	key << "THETA_INIT";
	std::ostringstream def;
	def << "0.5";
	theta_sensitivity = s2f(getParameter(key.str(), def.str()));

}

MSSOTLPolicy::~MSSOTLPolicy(void) {
	int r = 0;
}

double MSSOTLPolicy::computeDesirability(double vehInMeasure,
		double vehOutMeasure) {

	DBG(
			std::ostringstream str; str << "\nMSSOTLPolicy::computeStimulus\n" << getName(); WRITE_MESSAGE(str.str());)

	return myDesirabilityAlgorithm->computeDesirability(vehInMeasure,
			vehOutMeasure);

}

size_t MSSOTLPolicy::decideNextPhase(int elapsed,
		const MSPhaseDefinition *stage, size_t currentPhaseIndex,
		size_t phaseMaxCTS, bool thresholdPassed, int vehicleCount) {
	//If the junction was in a commit step
	//=> go to the target step that gives green to the set with the current highest CTS
	//   and return computeReturnTime()
	if (stage->isCommit()) {
		// decide which chain to activate. Gotta work on this
		return phaseMaxCTS;
	}
	if (stage->isTransient()) {
		//If the junction was in a transient step
		//=> go to the next step and return computeReturnTime()
		return currentPhaseIndex + 1;
	}

	if (stage->isDecisional()) {

		if (canRelease(elapsed, thresholdPassed, stage, vehicleCount)) {
			return currentPhaseIndex + 1;
		}
	}

	return currentPhaseIndex;
}

/*
 bool MSSOTLPolicy::canRelease(int elapsed, bool thresholdPassed, const MSPhaseDefinition* stage, int vehicleCount) {
 if (getName().compare("request") == 0) {
 return elapsed > 3000 && thresholdPassed;
 } else if (getName().compare("phase") == 0) {
 return thresholdPassed && elapsed >= stage->minDuration;
 } else if (getName().compare("platoon") == 0) {
 return thresholdPassed && (vehicleCount == 0 || elapsed >= stage->maxDuration);
 } else if (getName().compare("marching") == 0) {
 return elapsed >= stage->duration;
 } else if (getName().compare("congestion") == 0) {
 return elapsed >= stage->minDuration;
 }
 return true; //

 }
 */

