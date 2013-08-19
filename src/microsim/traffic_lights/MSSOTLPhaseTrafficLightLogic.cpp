/****************************************************************************/
/// @file    MSSOTLPhaseTrafficLightLogic.cpp
/// @author  Gianfilippo Slager
/// @date    Feb 2010
/// @version $Id: MSSOTLPhaseTrafficLightLogic.cpp 0 2010-02-18 12:40:00Z gslager $
///
// The class for SOTL Phase logics
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

#include "MSSOTLPhaseTrafficLightLogic.h"

MSSOTLPhaseTrafficLightLogic::MSSOTLPhaseTrafficLightLogic(MSTLLogicControl &tlcontrol,
                              const std::string &id, const std::string &subid,
							  const Phases &phases, unsigned int step, SUMOTime delay) throw() : MSSOTLTrafficLightLogic(tlcontrol, id, subid, phases, step, delay) {
								  MsgHandler::getMessageInstance()->inform("*** Intersection " + id + " will run using MSSOTLPhaseTrafficLightLogic ***");
}

MSSOTLPhaseTrafficLightLogic::MSSOTLPhaseTrafficLightLogic(MSTLLogicControl &tlcontrol,
                              const std::string &id, const std::string &subid,
							  const Phases &phases, unsigned int step, SUMOTime delay, MSSOTLSensors *sensors) throw() : MSSOTLTrafficLightLogic(tlcontrol, id, subid, phases, step, delay, sensors){
}

bool
MSSOTLPhaseTrafficLightLogic::canRelease() throw() {
	if (getCurrentPhaseElapsed() >= getCurrentPhaseDef().minDuration) {
		return isThresholdPassed();
	}
	return false;
}
