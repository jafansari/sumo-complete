/****************************************************************************/
/// @file    MSPerson.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// The class for modelling person-movements
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
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

#include <string>
#include <vector>
#include "MSNet.h"
#include "MSEdge.h"
#include "MSLane.h"
#include "MSPerson.h"
#include "MSPersonControl.h"
#include "MSEdgeControl.h"
#include "MSEmitControl.h"
#include "MSVehicle.h"
#include "MSVehicleType.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage::MSPersonStage(const MSEdge &destination)
        : myDestination(destination) {}


MSPerson::MSPersonStage::~MSPersonStage() {}


const MSEdge &
MSPerson::MSPersonStage::getDestination() const {
    return myDestination;
}


/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Walking - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Walking::MSPersonStage_Walking(MSEdgeVector route, SUMOTime walkingTime, SUMOReal speed)
        : MSPersonStage(*route.back()), myWalkingTime(walkingTime) {
    if (speed > 0) {
        SUMOReal time = 0;
        for (MSEdgeVector::const_iterator it = route.begin(); it != route.end(); ++it) {
            time += ((*it)->getLanes())[0]->getLength() * speed;
        }
        myWalkingTime = MAX2(walkingTime, time);
    }
}


MSPerson::MSPersonStage_Walking::~MSPersonStage_Walking() {}


void
MSPerson::MSPersonStage_Walking::proceed(MSNet* net,
        MSPerson* person, SUMOTime now,
        const MSEdge & /*previousEdge*/) {
    net->getPersonControl().setArrival(MAX2(now, now + myWalkingTime), person);
}



/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_PrivateVehicle - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Driving::MSPersonStage_Driving(const MSEdge &destination,
                                                       const std::vector<std::string> &lines)
        : MSPersonStage(destination), myLines(lines) {}


MSPerson::MSPersonStage_Driving::~MSPersonStage_Driving() {}


void
MSPerson::MSPersonStage_Driving::proceed(MSNet* net,
        MSPerson* person, SUMOTime now,
        const MSEdge &previousEdge) {
    MSVehicle *v = MSNet::getInstance()->getVehicleControl().getWaitingVehicle(&previousEdge, myLines);
    if (v != 0) {
        v->addPerson(person);
        if (v->getDesiredDepart() == -1) {
            MSNet::getInstance()->getEmitControl().add(v);
            MSNet::getInstance()->getVehicleControl().removeWaiting(&previousEdge, v);
        }
    }
}


/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Waiting - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Waiting::MSPersonStage_Waiting(const MSEdge &destination,
                                                       SUMOTime duration, SUMOTime until)
        : MSPersonStage(destination), myWaitingDuration(duration), myWaitingUntil(until) {}


MSPerson::MSPersonStage_Waiting::~MSPersonStage_Waiting() {}


void
MSPerson::MSPersonStage_Waiting::proceed(MSNet* net,
        MSPerson* person, SUMOTime now,
        const MSEdge & /*previousEdge*/) {
    const SUMOReal until = MAX3(now, now + myWaitingDuration, myWaitingUntil);
    net->getPersonControl().setArrival(until, person);
}


/* -------------------------------------------------------------------------
 * MSPerson - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPerson(const SUMOVehicleParameter* pars, MSPersonPlan *plan)
        : myParameter(pars), myPlan(plan) {
    myStep = myPlan->begin();
}


MSPerson::~MSPerson() {
    delete myPlan;
}


const std::string&
MSPerson::getID() const throw() {
    return myParameter->id;
}


void
MSPerson::proceed(MSNet* net, SUMOTime time) {
    const MSEdge &arrivedAt = (*myStep)->getDestination();
    myStep++;
    if (myStep != myPlan->end()) {
        (*myStep)->proceed(net, this, time, arrivedAt);
    }
}


bool
MSPerson::endReached() const {
    return myStep == myPlan->end();
}


SUMOTime
MSPerson::getDesiredDepart() const throw() {
    return myParameter->depart;
}


const MSEdge &
MSPerson::getDestination() const {
    return (*myStep)->getDestination();
}


/****************************************************************************/

