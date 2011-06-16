/****************************************************************************/
/// @file    MSCFModel_KraussOrig1.cpp
/// @author  Tobias Mayer
/// @date    Mon, 04 Aug 2009
/// @version $Id$
///
// The original Krauss (1998) car-following model and parameter
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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

#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_KraussOrig1.h"
#include <microsim/MSAbstractLaneChangeModel.h>
#include <utils/common/RandHelper.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_KraussOrig1::MSCFModel_KraussOrig1(const MSVehicleType* vtype,  SUMOReal accel, SUMOReal decel,
        SUMOReal dawdle, SUMOReal tau) throw()
        : MSCFModel(vtype, decel), myAccel(accel), myDawdle(dawdle), myTau(tau) {

    myTauDecel = decel * myTau;
}


MSCFModel_KraussOrig1::~MSCFModel_KraussOrig1() throw() {}


SUMOReal
MSCFModel_KraussOrig1::moveHelper(MSVehicle * const veh, SUMOReal vPos) const throw() {
    const SUMOReal oldV = veh->getSpeed(); // save old v for optional acceleration computation
    const SUMOReal vSafe = MIN2(vPos, veh->processNextStop(vPos)); // process stops
    // we need the acceleration for emission computation;
    //  in this case, we neglect dawdling, nonetheless, using
    //  vSafe does not incorporate speed reduction due to interaction
    //  on lane changing
    veh->setPreDawdleAcceleration(SPEED2ACCEL(vSafe-oldV));
    const SUMOReal vMin = MAX2((SUMOReal) 0, oldV - ACCEL2SPEED(myDecel));
    const SUMOReal vMax = MIN3(veh->getLane()->getMaxSpeed(), maxNextSpeed(oldV), vSafe);
    assert(vMin<=vMax);
    return veh->getLaneChangeModel().patchSpeed(vMin, MAX2(vMin, dawdle(vMax)), vMax, *this);
}


SUMOReal
MSCFModel_KraussOrig1::ffeV(const MSVehicle * const /*veh*/, SUMOReal speed, SUMOReal gap, SUMOReal predSpeed) const throw() {
    return MIN2(_vsafe(gap, predSpeed), maxNextSpeed(speed));
}


SUMOReal
MSCFModel_KraussOrig1::ffeV(const MSVehicle * const veh, SUMOReal gap, SUMOReal predSpeed) const throw() {
    return MIN2(_vsafe(gap, predSpeed), maxNextSpeed(veh->getSpeed()));
}


SUMOReal
MSCFModel_KraussOrig1::ffeV(const MSVehicle * const veh, const MSVehicle *pred) const throw() {
    return MIN2(_vsafe(veh->gap2pred(*pred), pred->getSpeed()), maxNextSpeed(veh->getSpeed()));
}


SUMOReal
MSCFModel_KraussOrig1::ffeS(const MSVehicle * const veh, SUMOReal gap) const throw() {
    return MIN2(_vsafe(gap, 0), maxNextSpeed(veh->getSpeed()));
}


SUMOReal
MSCFModel_KraussOrig1::interactionGap(const MSVehicle * const veh, SUMOReal vL) const throw() {
    // Resolve the vsafe equation to gap. Assume predecessor has
    // speed != 0 and that vsafe will be the current speed plus acceleration,
    // i.e that with this gap there will be no interaction.
    SUMOReal vNext = MIN2(maxNextSpeed(veh->getSpeed()), veh->getLane()->getMaxSpeed());
    SUMOReal gap = (vNext - vL) *
                   ((veh->getSpeed() + vL) * myInverseTwoDecel + myTau) +
                   vL * myTau;

    // Don't allow timeHeadWay < deltaT situations.
    return MAX2(gap, SPEED2DIST(vNext));
}


SUMOReal
MSCFModel_KraussOrig1::dawdle(SUMOReal speed) const throw() {
    return MAX2(SUMOReal(0), speed - ACCEL2SPEED(myDawdle * myAccel * RandHelper::rand()));
}


/** Returns the SK-vsafe. */
SUMOReal MSCFModel_KraussOrig1::_vsafe(SUMOReal gap, SUMOReal predSpeed) const throw() {
    if (predSpeed==0&&gap<0.01) {
        return 0;
    }
    SUMOReal vsafe = (SUMOReal)(-1. * myTauDecel
                                + sqrt(
                                    myTauDecel*myTauDecel
                                    + (predSpeed*predSpeed)
                                    + (2. * myDecel * gap)
                                ));
    assert(vsafe >= 0);
    return vsafe;
}


MSCFModel *
MSCFModel_KraussOrig1::duplicate(const MSVehicleType *vtype) const throw() {
    return new MSCFModel_KraussOrig1(vtype, myAccel, myDecel, myDawdle, myTau);
}


//void MSCFModel::saveState(std::ostream &os) {}

