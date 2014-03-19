/****************************************************************************/
/// @file    MSSwarmTrafficLightLogic.cpp
/// @author  Gianfilippo Slager
/// @date    Mar 2010
/// @version $Id: MSSwarmTrafficLightLogic.h 1 2010-03-06 12:40:00Z gslager $
///
// The class for Swarm-based logics
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

#include "MSSwarmTrafficLightLogic.h"

MSSwarmTrafficLightLogic::MSSwarmTrafficLightLogic(MSTLLogicControl &tlcontrol,
		const std::string &id, const std::string &subid, const Phases &phases,
		unsigned int step, SUMOTime delay,
		const std::map<std::string, std::string>& parameters) :
		MSSOTLHiLevelTrafficLightLogic(tlcontrol, id, subid, phases, step,
				delay, parameters) {

	addPolicy(new MSSOTLPlatoonPolicy(new MSSOTLPolicyStimulus(parameters),parameters));
	addPolicy(new MSSOTLPhasePolicy(new MSSOTLPolicyStimulus(parameters),parameters));
	addPolicy(new MSSOTLMarchingPolicy(new MSSOTLPolicyStimulus(parameters),parameters));
	addPolicy(new MSSOTLCongestionPolicy(new MSSOTLPolicyStimulus(parameters),parameters));

	mustChange = false;

	DBG(
		std::ostringstream d_str;
		d_str << getMaxCongestionDuration();
		vector<MSSOTLPolicy*> policies = getPolicies();

		MsgHandler::getMessageInstance()->inform(
				"getMaxCongestionDuration " + d_str.str());
		for (unsigned int i = 0; i < policies.size(); i++) {
			MSSOTLPolicy* policy = policies[i];
			MSSOTLPolicyStimulus* stim =
					(MSSOTLPolicyStimulus*) policy->getDesirabilityAlgorithm();
			std::ostringstream _str;
			_str << policy->getName() << " stimCox " << stim->getStimCox()
					<< " StimOffsetIn " << stim->getStimOffsetIn()
					<< " StimOffsetOut " << stim->getStimOffsetOut()
					<< " StimDivisorIn " << stim->getStimDivisorIn()
					<< " StimDivisorOut " << stim->getStimDivisorOut()
					<< " StimCoxExpIn " << stim->getStimCoxExpIn()
					<< " StimCoxExpOut " << stim->getStimCoxExpOut()
					<< " getThetaSensitivity " << policy->getThetaSensitivity()
					<< " .";
			WRITE_MESSAGE(_str.str());
		)
	}
	congestion_steps = 0;

}

MSSwarmTrafficLightLogic::~MSSwarmTrafficLightLogic() {
	if (logData && swarmLogFile.is_open()) {
		swarmLogFile.close();
	}
}

void MSSwarmTrafficLightLogic::init(NLDetectorBuilder &nb) throw (ProcessError) {
	MSSOTLHiLevelTrafficLightLogic::init(nb);
	//Setting the startup policy
	choosePolicy(0, 0);
	//Initializing the random number generator to a time-dependent seed
	srand((unsigned int) time(NULL));
	//Initializing pheromone maps according to input lanes
	//For each lane insert a pair into maps
	MSLane *currentLane = NULL;
	for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector =
			myLanes.begin(); laneVector != myLanes.end(); laneVector++) {
		for (MSTrafficLightLogic::LaneVector::const_iterator lane =
				laneVector->begin(); lane != laneVector->end(); lane++) {
			currentLane = (*lane);
			if (pheromoneInputLanes.find(currentLane->getID())
					== pheromoneInputLanes.end()) {
				pheromoneInputLanes.insert(
						MSLaneId_Pheromone(currentLane->getID(), 0.0));
				DBG(
						WRITE_MESSAGE("*** Intersection " + getID() + " pheromoneInputLanes adding " +currentLane->getID() );)
			}
		}
	}

	LinkVectorVector myLinks = getLinks();

	for (unsigned int i = 0; i < myLinks.size(); i++) {
		LinkVector oneLink = getLinksAt(i);

		for (unsigned int j = 0; j < oneLink.size(); j++) {

			currentLane = oneLink[j]->getLane();

			if (pheromoneOutputLanes.find(currentLane->getID())
					== pheromoneOutputLanes.end()) {
				pheromoneOutputLanes.insert(
						MSLaneId_Pheromone(currentLane->getID(), 0.0));
				DBG(
						WRITE_MESSAGE("*** Intersection " + getID() + " pheromoneOutputLanes adding " +currentLane->getID() );)
			}
		}
	}

	//Initializing thresholds for theta evaluations
	lastThetaSensitivityUpdate = MSNet::getInstance()->getCurrentTimeStep();

	WRITE_MESSAGE(
			"*** Intersection " + getID()
					+ " will run using MSSwarmTrafficLightLogic ***");
	string logFileName = getParameter("SWARMLOG", "");
	logData = logFileName.compare("") != 0;
	if (logData) {
		swarmLogFile.open(logFileName.c_str(), ios::out | ios::binary);
	}

}

void MSSwarmTrafficLightLogic::resetPheromone() {
	//input
	for (MSLaneId_PheromoneMap::iterator laneIterator =
			pheromoneInputLanes.begin();
			laneIterator != pheromoneInputLanes.end(); laneIterator++) {
		string laneId = laneIterator->first;
		pheromoneInputLanes[laneId] = 0;
	}
	//output
	for (MSLaneId_PheromoneMap::iterator laneIterator =
			pheromoneOutputLanes.begin();
			laneIterator != pheromoneOutputLanes.end(); laneIterator++) {
		string laneId = laneIterator->first;
		pheromoneOutputLanes[laneId] = 0;
	}
}

size_t MSSwarmTrafficLightLogic::decideNextPhase() {

	DBG(
			MsgHandler::getMessageInstance()->inform("\n" +time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic decideNextPhase()"); std::ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " getCurrentPhaseDef().getState()=" << getCurrentPhaseDef().getState() << " is commit?" << getCurrentPhaseDef().isCommit(); MsgHandler::getMessageInstance()->inform(dnp.str());)
	// if we're congested, it should be wise to reset and recalculate the pheromone levels after X steps
	if (getCurrentPolicy()->getName().compare("Congestion") == 0
			&& getCurrentPhaseDef().isCommit()) {
		congestion_steps += STEPS2TIME(getCurrentPhaseDef().duration);
		DBG(
				WRITE_MESSAGE("\n" +time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic decideNextPhase()"); std:ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " congestion_steps=" << congestion_steps; WRITE_MESSAGE(dnp.str());)
		if (congestion_steps >= getMaxCongestionDuration()) {
			resetPheromone();
			congestion_steps = 0;
			mustChange = true;
			DBG(
					WRITE_MESSAGE("\n" +time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic decideNextPhase()"); std::ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " max congestion reached, congestion_steps=" << congestion_steps; WRITE_MESSAGE(dnp.str());)
		}
	}

	//Update pheromone levels
	updatePheromoneLevels();

	//Decide the current policy according to pheromone levels. this should be done only at the end of a chain, before selecting the new one
	if (getCurrentPhaseDef().isCommit()) {
		decidePolicy();
	}

	//Update learning and forgetting thresholds
	updateSensitivities();

	DBG(
			std::ostringstream str; str << "tlsID=" << getID() << " currentPolicyname="+getCurrentPolicy()->getName(); WRITE_MESSAGE(str.str());)

	//Execute current policy. congestion "policy" must maintain the commit phase, and that must be an all-red one
	return getCurrentPolicy()->decideNextPhase(getCurrentPhaseElapsed(),
			&getCurrentPhaseDef(), getCurrentPhaseIndex(),
			getPhaseIndexWithMaxCTS(), isThresholdPassed(),
			countVehicles(getCurrentPhaseDef()));
}

void MSSwarmTrafficLightLogic::updatePheromoneLevels() {
	//Updating input lanes pheromone: all input lanes without distinction
	DBG(
			std::ostringstream _str; _str << " pheromoneInputLanes " << pheromoneInputLanes.size()<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::updatePheromoneLevels::"+_str.str());)

	//BETA_NO, GAMMA_NO
	for (MSLaneId_PheromoneMap::iterator laneIterator =
			pheromoneInputLanes.begin();
			laneIterator != pheromoneInputLanes.end(); laneIterator++) {
		string laneId = laneIterator->first;
		double oldPheroIn = laneIterator->second;
		double maxSpeed = getSensors()->meanVehiclesSpeed(laneId);
		bool updatePheromoneOut = (maxSpeed > -1);

		unsigned int vehicles = //getSensors()->countVehicles(laneId);
				13.89 - maxSpeed;
		double pheroIn = getBetaNo() * oldPheroIn + // Evaporation
				getGammaNo() * vehicles * updatePheromoneOut;
		;

		pheroIn = max(pheroIn, 0.0);
		pheroIn = min(pheroIn, getPheroMaxVal());
		pheromoneInputLanes[laneId] = pheroIn;
		DBG(
				std::ostringstream i_str; i_str << " meanVehiclesSpeed " << maxSpeed<<" pheromoneInputLanes "<<pheromoneInputLanes[laneId] << " lane "<< laneId<<" ID "<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::updatePheromoneLevels:: PheroIn"+i_str.str());)
	}

	//BETA_SP, GAMMA_SP
	//Updating output lanes pheromone: only input lanes currently having green light. Pheromone for non green lanes is "freezed"
	if (/*getCurrentPhaseDef(). isDecisional()*/true) {

		for (MSLaneId_PheromoneMap::iterator laneIterator =
				pheromoneOutputLanes.begin();
				laneIterator != pheromoneOutputLanes.end(); laneIterator++) {
			string currentLane = laneIterator->first;
			double oldPheroOut = laneIterator->second;

			// Load on the output lane is calculated with the difference between maximum and avg speed
			double maxSpeed = 13.89;//getSensors()->getMaxSpeed(currentLane);
			double meanVehiclesSpeed = getSensors()->meanVehiclesSpeed(
					currentLane);
			double pheroAdd = maxSpeed - meanVehiclesSpeed;
			// se è -1 non ci sono veicoli, con 0 ci sono ma sono fermi
			bool updatePheromoneOut = (meanVehiclesSpeed > -1);
			double pheroOut = getBetaSp() * oldPheroOut
					+ getGammaSp() * (pheroAdd) * updatePheromoneOut;
			//          pheroOut*=3;
			pheroOut = max(pheroOut, 0.0);	//sometimes pheroOut goes below 0
			pheroOut = min(pheroOut, getPheroMaxVal());
			pheromoneOutputLanes[currentLane] = pheroOut;

			DBG(
					std::ostringstream phero_str; phero_str << " oldPheroOut " << oldPheroOut << " ,pheroAdd  " << pheroAdd << " currentLane " << currentLane <<" ,meanVehiclesSpeed " << meanVehiclesSpeed <<" ,pheroOut " << pheroOut<<" ,ID  " << getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::updatePheromoneLevels::"+phero_str.str());)
		}
	}

}
void MSSwarmTrafficLightLogic::updateSensitivities() {
	SUMOTime elapsedTime = STEPS2TIME(
			MSNet::getInstance()->getCurrentTimeStep()
					- lastThetaSensitivityUpdate);
	lastThetaSensitivityUpdate = MSNet::getInstance()->getCurrentTimeStep();
	MSSOTLPolicy* currentPolicy = getCurrentPolicy();
	vector<MSSOTLPolicy*> policies = getPolicies();

	for (unsigned int i = 0; i < policies.size(); i++) {
		MSSOTLPolicy* policy = policies[i];
		double newSensitivity;

		if (policy == currentPolicy) { 			//Learning
			newSensitivity = policy->getThetaSensitivity()
					- getLearningCox() * elapsedTime;

		} else { //Forgetting

			newSensitivity = policy->getThetaSensitivity()
					+ getForgettingCox() * elapsedTime;
		}
		DBG(
				std::ostringstream phero_str; phero_str << " policy " << policy->getName() << " newSensitivity " << newSensitivity << " ,pol.Sensitivity  " << policy->getThetaSensitivity() << " ,LearningCox " << getLearningCox() <<" ,elapsedTime " << elapsedTime <<" ,LCox*Time " << getLearningCox() * elapsedTime;

				newSensitivity = max(min(newSensitivity, getThetaMax()), getThetaMin());

				phero_str << " NEWERSensitivity= " << newSensitivity<< " ID "<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::updateSensitivities::"+phero_str.str());)
		policy->setThetaSensitivity(newSensitivity);
	}
}

double MSSwarmTrafficLightLogic::getPheromoneForInputLanes() {
	if (pheromoneInputLanes.size() == 0) {
		return 0;
	}
	double pheroIn = 0;
	for (MSLaneId_PheromoneMap::const_iterator iterator =
			pheromoneInputLanes.begin(); iterator != pheromoneInputLanes.end();
			iterator++) {
		string laneId = iterator->first;
		double lanePhero = iterator->second;
		pheroIn += iterator->second;
		DBG(
				std::ostringstream phero_str; phero_str << " lane " << iterator->first << " pheromoneIN  " << iterator->second<<" id "<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::getPheromoneForInputLanes::"+phero_str.str());)
	}

	DBG(
			std::ostringstream o_str; o_str << " TOTpheromoneIN  " << pheroIn<< " return  " << pheroIn/pheromoneInputLanes.size()<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::getPheromoneForInputLanes::"+o_str.str());)
	return pheroIn / pheromoneInputLanes.size();
}

double MSSwarmTrafficLightLogic::getPheromoneForOutputLanes() {
	if (pheromoneOutputLanes.size() == 0) {
		return 0;
	}
	double pheroOut = 0;
	for (MSLaneId_PheromoneMap::const_iterator iterator =
			pheromoneOutputLanes.begin();
			iterator != pheromoneOutputLanes.end(); iterator++) {
		DBG(
				std::ostringstream phero_str; phero_str << " lane " << iterator->first << " pheromoneOUT  " << iterator->second<<" id "<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::getPheromoneForOutputLanes::"+phero_str.str());)
		pheroOut += iterator->second;
	}
	DBG(
			std::ostringstream o_str; o_str << " TOTpheromoneOUT  " << pheroOut<< " return  " << pheroOut/pheromoneOutputLanes.size()<<" id "<< getID() <<" ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::getPheromoneForOutputLanes::"+o_str.str());)
	return pheroOut / pheromoneOutputLanes.size();
}

void MSSwarmTrafficLightLogic::decidePolicy() {
	MSSOTLPolicy* currentPolicy = getCurrentPolicy();
	// Decide if it is the case to check for another plan
	double sampled = (double) rand();
	double changeProb = getChangePlanProbability();
	changeProb = changeProb * RAND_MAX;

	if (sampled <= changeProb || mustChange) { // Check for another plan

		double pheroIn = getPheromoneForInputLanes();
		double pheroOut = getPheromoneForOutputLanes();
		MSSOTLPolicy* oldPolicy = getCurrentPolicy();
		choosePolicy(pheroIn, pheroOut);
		MSSOTLPolicy* newPolicy = getCurrentPolicy();

		if (newPolicy != oldPolicy) {
			SUMOTime step = MSNet::getInstance()->getCurrentTimeStep();
			DBG(
					std::ostringstream phero_str; phero_str << " (pheroIn= " << pheroIn << " ,pheroOut= " << pheroOut << " )"; WRITE_MESSAGE("TL " +getID()+" time " +time2string(step)+" Policy: " +newPolicy->getName() +phero_str.str() +" OldPolicy: " + oldPolicy->getName()+" id "+getID()+" .");)
			if (oldPolicy->getName().compare("Congestion") == 0) {
				congestion_steps = 0;
			}
		} else //debug purpose only
		{
			DBG(
					std::ostringstream phero_str; phero_str << " (pheroIn= " << pheroIn << " ,pheroOut= " << pheroOut << " )"; SUMOTime step = MSNet::getInstance()->getCurrentTimeStep(); WRITE_MESSAGE("TL " +getID()+" time " +time2string(step)+" Policy: Nochanges" +phero_str.str()+" OldPolicy: " + oldPolicy->getName()+" id " +getID()+ " .");)
		}

		mustChange = false;

	}
}

void MSSwarmTrafficLightLogic::choosePolicy(double phero_in, double phero_out) {
	vector<double> thetaStimuli;
	double thetaSum = 0.0;
	// Compute simulus for each policy
	for (unsigned int i = 0; i < getPolicies().size(); i++) {
		double stimulus = getPolicies()[i]->computeDesirability(phero_in,
				phero_out);
		double thetaStimulus = pow(stimulus, 2)
				/ (pow(stimulus, 2)
						+ pow(getPolicies()[i]->getThetaSensitivity(), 2));

		thetaStimuli.push_back(thetaStimulus);
		thetaSum += thetaStimulus;

		DBG(
				ostringstream so_str; so_str << " policy " << getPolicies()[i]->getName() << " stimulus " << stimulus << " pow(stimulus,2) " << pow(stimulus, 2) << " pow(Threshold,2) " << pow(getPolicies()[i]->getThetaSensitivity(), 2) << " thetaStimulus " << thetaStimulus << " thetaSum " << thetaSum<<" ."; WRITE_MESSAGE("MSSwarmTrafficLightLogic::choosePolicy::"+so_str.str());)

	}

	// Compute a random value between 0 and the sum of the thetaVal
	double r = rand();
	r = r / RAND_MAX * thetaSum;

	double partialSum = 0;
	for (unsigned int i = 0; i < getPolicies().size(); i++) {
		partialSum += thetaStimuli[i];

		DBG(
				ostringstream aao_str; aao_str << " policy " << getPolicies()[i]->getName() << " partialSum " << partialSum << " thetaStimuls " << thetaStimuli[i] << " r " << r<<" ."; WRITE_MESSAGE("MSSwarmTrafficLightLogic::choosePolicy::"+aao_str.str());)

		if (partialSum >= r) {
			activate(getPolicies()[i]);
			break;
		}
	}
}

bool MSSwarmTrafficLightLogic::canRelease() {
	DBG(
			std::ostringstream phero_str; phero_str << "getCurrentPhaseElapsed()=" << time2string(getCurrentPhaseElapsed()) << " isThresholdPassed()=" << isThresholdPassed() << " currentPhase=" << (&getCurrentPhaseDef())->getState() << " countVehicles()=" << countVehicles(getCurrentPhaseDef()); WRITE_MESSAGE("\nMSSwamTrafficLightLogic::canRelease(): "+phero_str.str());)
	return getCurrentPolicy()->canRelease(getCurrentPhaseElapsed(),
			isThresholdPassed(), &getCurrentPhaseDef(),
			countVehicles(getCurrentPhaseDef()));
}
