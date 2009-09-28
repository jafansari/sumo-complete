/****************************************************************************/
/// @file    MSCFModel_Krauss.cpp
/// @author  Tobias Mayer
/// @date    Mon, 04 Aug 2009
/// @version $Id$
///
// The Krauss car-following model and parameter
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSVehicle.h"
#include "MSLane.h"
#include "MSCFModel_Krauss.h"
#include "MSAbstractLaneChangeModel.h"
#include <utils/common/RandHelper.h>


// ===========================================================================
// used namespaces
// ===========================================================================
//using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_Krauss::MSCFModel_Krauss(const MSVehicleType* vtype, SUMOReal dawdle, SUMOReal tau) throw()
        : MSCFModel(vtype), myDawdle(dawdle), myTau(tau) {

    myInverseTwoDecel = SUMOReal(1) / (SUMOReal(2) * vtype->getMaxDecel());
    myTauDecel = vtype->getMaxDecel() * myTau;
}

MSCFModel_Krauss::~MSCFModel_Krauss() throw() {}



SUMOReal 
MSCFModel_Krauss::move(MSVehicle * const veh, const MSLane * const lane, const MSVehicle * const pred, const MSVehicle * const neigh) const throw() {
    // save old v for optional acceleration computation
    SUMOReal oldV = veh->getSpeed();
    // compute gap to use
    SUMOReal gap = veh->gap2pred(*pred);
    // security check for too low gaps
    if (gap<0.1) {
        gap = 0;
    }

    SUMOReal vSafe  = ffeV(veh, gap, pred->getSpeed());
    leftVehicleVsafe(veh, neigh, vSafe);
    SUMOReal vNext = moveHelper(veh, lane, vSafe);
    SUMOReal predDec = pred->getSpeedAfterMaxDecel(pred->getSpeed()); //!!!!q//-decelAbility() /* !!! decelAbility of leader! */);
    if (brakeGap(vNext)+vNext*myTau > brakeGap(predDec) + gap) {
        vNext = MIN2(vNext, (SUMOReal) DIST2SPEED(gap));
    }
    vNext = MAX3((SUMOReal) 0, vNext, myType->getSpeedAfterMaxDecel(oldV));
    return vNext;
}


SUMOReal 
MSCFModel_Krauss::moveHelper(MSVehicle * const veh, const MSLane * const lane, SUMOReal vPos) const throw() {
    // save old v for optional acceleration computation
    SUMOReal oldV = veh->getSpeed();
    SUMOReal vSafe = MIN2(vPos, veh->processNextStop(vPos));
    // we need the acceleration for emission computation;
    //  in this case, we neglect dawdling, nonetheless, using
    //  vSafe does not incorporate speed reduction due to interaction
    //  on lane changing
    veh->setPreDawdleAcceleration(SPEED2ACCEL(vSafe-oldV));
    SUMOReal vNext = dawdle(MIN3(lane->maxSpeed(), maxNextSpeed(oldV), vSafe));
    vNext =
        veh->getLaneChangeModel().patchSpeed(
            MAX2((SUMOReal) 0, oldV-(SUMOReal)ACCEL2SPEED(myType->getMaxDecel())), //!!! reverify
            vNext,
            MIN3(vSafe, veh->getLane().maxSpeed(), maxNextSpeed(oldV)),//vaccel(myState.mySpeed, myLane->maxSpeed())),
            vSafe);
    vNext = MIN4(vNext, vSafe, veh->getLane().maxSpeed(), maxNextSpeed(oldV));
    return vNext;
}


void 
MSCFModel_Krauss::leftVehicleVsafe(const MSVehicle * const ego, const MSVehicle * const neigh, SUMOReal &vSafe) const throw()
{
    if (neigh!=0&&neigh->getSpeed()>60./3.6) {
        SUMOReal mgap = MAX2((SUMOReal) 0, neigh->getPositionOnLane()-neigh->getVehicleType().getLength()-ego->getPositionOnLane());
        SUMOReal nVSafe = ffeV(ego, mgap, neigh->getSpeed());
        if (mgap-neigh->getSpeed()>=0) {
            vSafe = MIN2(vSafe, nVSafe);
        }
    }
}


SUMOReal
MSCFModel_Krauss::ffeV(const MSVehicle * const veh, SUMOReal speed, SUMOReal gap2pred, SUMOReal predSpeed) const throw() {
    return MIN2(_vsafe(gap2pred, predSpeed), maxNextSpeed(speed));
}


SUMOReal 
MSCFModel_Krauss::ffeV(const MSVehicle * const veh, SUMOReal gap2pred, SUMOReal predSpeed) const throw() {
    return MIN2(_vsafe(gap2pred, predSpeed), maxNextSpeed(veh->getSpeed()));
}


SUMOReal 
MSCFModel_Krauss::ffeV(const MSVehicle * const veh, const MSVehicle *pred) const throw() {
    return MIN2(_vsafe(veh->gap2pred(*pred), pred->getSpeed()), maxNextSpeed(veh->getSpeed()));
}


SUMOReal 
MSCFModel_Krauss::ffeS(const MSVehicle * const veh, SUMOReal gap2pred) const throw() {
    return MIN2(_vsafe(gap2pred, 0), maxNextSpeed(veh->getSpeed()));
}


SUMOReal 
MSCFModel_Krauss::maxNextSpeed(SUMOReal speed) const throw() {
    return MIN2(speed + (SUMOReal) ACCEL2SPEED(myType->getMaxAccel(speed)), myType->getMaxSpeed());
}


SUMOReal 
MSCFModel_Krauss::brakeGap(SUMOReal speed) const throw() {
    return speed * speed * myInverseTwoDecel + speed * myTau;
}


SUMOReal 
MSCFModel_Krauss::approachingBrakeGap(SUMOReal speed) const throw() {
    return speed * speed * myInverseTwoDecel;
}


SUMOReal MSCFModel_Krauss::interactionGap(const MSVehicle * const veh, SUMOReal vL) const throw() {
    // Resolve the vsafe equation to gap. Assume predecessor has
    // speed != 0 and that vsafe will be the current speed plus acceleration,
    // i.e that with this gap there will be no interaction.
    SUMOReal vNext = MIN2(maxNextSpeed(veh->getSpeed()), veh->getLane().maxSpeed());
    SUMOReal gap = (vNext - vL) *
                   ((veh->getSpeed() + vL) * myInverseTwoDecel + myTau) +
                   vL * myTau;

    // Don't allow timeHeadWay < deltaT situations.
    return MAX2(gap, timeHeadWayGap(vNext));
}


bool 
MSCFModel_Krauss::hasSafeGap(SUMOReal speed, SUMOReal gap, SUMOReal predSpeed, SUMOReal laneMaxSpeed) const throw() {
    if(gap<0) {
        return false;
    }
    SUMOReal vSafe = MIN2(_vsafe(gap, predSpeed), maxNextSpeed(speed));
    SUMOReal vNext = MIN3(maxNextSpeed(speed), laneMaxSpeed, vSafe);
    return (vNext>=myType->getSpeedAfterMaxDecel(speed)
            &&
            gap   >= timeHeadWayGap(speed));
}


SUMOReal 
MSCFModel_Krauss::safeEmitGap(SUMOReal speed) const throw() {
    SUMOReal vNextMin = myType->getSpeedAfterMaxDecel(speed); // ok, minimum next speed
    SUMOReal safeGap  = vNextMin * (speed * myInverseTwoDecel + myTau);
    return MAX2(safeGap, timeHeadWayGap(speed)) + ACCEL2DIST(myType->getMaxAccel(speed));
}


SUMOReal 
MSCFModel_Krauss::dawdle(SUMOReal speed) const throw() {
    // generate random number out of [0,1]
    SUMOReal random = RandHelper::rand();
    // Dawdle.
    if (speed < myType->getMaxAccel(0)) {
        // we should not prevent vehicles from driving just due to dawdling
        //  if someone is starting, he should definitely start
        // (but what about slow-to-start?)!!!
        speed -= ACCEL2SPEED(myDawdle * speed * random);
    } else {
        speed -= ACCEL2SPEED(myDawdle * myType->getMaxAccel(speed) * random);
    }
    return MAX2(SUMOReal(0), speed);
}


SUMOReal 
MSCFModel_Krauss::decelAbility() const throw() {
    return ACCEL2SPEED(myType->getMaxDecel());
}



/** Returns the SK-vsafe. */
SUMOReal MSCFModel_Krauss::_vsafe(SUMOReal gap2pred, SUMOReal predSpeed) const throw() {
    if (predSpeed==0&&gap2pred<0.01) {
        return 0;
    }
    assert(gap2pred  >= SUMOReal(0));
    assert(predSpeed >= SUMOReal(0));
    SUMOReal vsafe = (SUMOReal)(-1. * myTauDecel
                                + sqrt(
                                    myTauDecel*myTauDecel
                                    + (predSpeed*predSpeed)
                                    + (2. * myType->getMaxDecel() * gap2pred)
                                ));
    assert(vsafe >= 0);
    return vsafe;
}



//void MSCFModel::saveState(std::ostream &os) {}

