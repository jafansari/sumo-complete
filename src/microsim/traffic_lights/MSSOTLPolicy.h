/****************************************************************************/
/// @file    MSSOTLPolicy.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @author  Anna Chiara Bellini
/// @date    Jun 2013
/// @version $Id: MSSOTLPolicy.h 0  $
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

#ifndef MSTLPolicy_h
#define MSTLPolicy_h

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include <sstream>
#include <math.h>
#include <utility>
#include <vector>
#include <utils/common/Parameterised.h>
#include "MSPhaseDefinition.h"
#include "MSSOTLPolicyDesirability.h"

using namespace std;

/**
 * @class MSSOTLPolicy
 * @brief Class for a low-level policy.
 * 
 */
class MSSOTLPolicy: public Parameterised {
private:

	/**
	 * \brief The sensitivity of this policy
	 */
	double theta_sensitivity;
	/**
	 * \brief The name of the policy
	 */
	string myName;
	/**
	 * \brief A pointer to the policy desirability object.\nIt's an optional component related to the computeDesirability() method and it's necessary
	 * only when the policy is used in combination with an high level policy.
	 */
	MSSOTLPolicyDesirability *myDesirabilityAlgorithm;

protected:
	double s2f(string str) {
		istringstream buffer(str);
		double temp;
		buffer >> temp;
		return temp;
	}

public:
	/** @brief Simple constructor
	 * @param[in] name The name of the policy
	 * @param[in] parameters Parameters defined for the policy
	 */
	MSSOTLPolicy(string name,
			const std::map<std::string, std::string>& parameters);
	/** @brief Constructor when the policy is a low-level policy used by an high level policy
	 * @param[in] name The name of the policy
	 * @param[in] desirabilityAlgorithm The desirability algorithm to be used for this policy
	 */
	MSSOTLPolicy(string name, MSSOTLPolicyDesirability *desirabilityAlgorithm);
	/** @brief Constructor when the policy is a low-level policy used by an high level policy
	 * @param[in] name The name of the policy
	 * @param[in] desirabilityAlgorithm The desirability algorithm to be used for this policy
	 * @param[in] parameters Parameters defined for the policy
	 */
	MSSOTLPolicy(string name, MSSOTLPolicyDesirability *desirabilityAlgorithm,
			const std::map<std::string, std::string>& parameters);
	virtual ~MSSOTLPolicy();

	virtual bool canRelease(int elapsed, bool thresholdPassed,
			const MSPhaseDefinition* stage, int vehicleCount) = 0;
	virtual size_t decideNextPhase(int elapsed, const MSPhaseDefinition* stage,
			size_t currentPhaseIndex, size_t phaseMaxCTS, bool thresholdPassed,
			int vehicleCount);

	virtual double getThetaSensitivity() {
		return theta_sensitivity;
	}
	virtual void setThetaSensitivity(double val) {
		theta_sensitivity = val;
	}
	string getName() {
		return myName;
	}
	MSSOTLPolicyDesirability* getDesirabilityAlgorithm() {
		return myDesirabilityAlgorithm;
	}
	/**
	 * \brief Computes the desirability of this policy, necessary when used in combination with an high level policy.
	 */
	double computeDesirability(double vehInMeasure, double vehOutMeasure);

};

#endif
