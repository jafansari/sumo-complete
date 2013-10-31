/****************************************************************************/
/// @file    MSLCM_DK2013.h
/// @author  Daniel Krajzewicz
/// @author  Friedemann Wesner
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 08.10.2013
/// @version $Id$
///
// A lane change model developed by D. Krajzewicz et al. between 2004 and 2013
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo-sim.org/
// Copyright (C) 2001-2013 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
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

#include <iostream>
#include <utils/common/RandHelper.h>
#include "MSEdge.h"
#include "MSLane.h"
#include "MSNet.h"
#include "MSLCM_DK2013.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS

//#define DEBUG_VEHICLE_GUI_SELECTION 1
#ifdef DEBUG_VEHICLE_GUI_SELECTION
#include <utils/gui/div/GUIGlobalSelection.h>
#include <guisim/GUIVehicle.h>
#include <guisim/GUILane.h>
#endif



// ===========================================================================
// variable definitions
// ===========================================================================
// 80km/h will be the threshold for dividing between long/short foresight
#define LOOK_FORWARD_SPEED_DIVIDER 14.

#define LOOK_FORWARD_FAR  15.
#define LOOK_FORWARD_NEAR 5.


#define JAM_FACTOR 1.


// ===========================================================================
// member method definitions
// ===========================================================================
MSLCM_DK2013::MSLCM_DK2013(MSVehicle& v) : 
    MSAbstractLaneChangeModel(v),
    mySpeedGainProbability(0),
    myLeadingBlockerLength(0), 
    myLeftSpace(0),
    myLastAccel(0)
{}

MSLCM_DK2013::~MSLCM_DK2013() {
    changed();
}


int
MSLCM_DK2013::wantsChange(
        int laneOffset,
        MSAbstractLaneChangeModel::MSLCMessager& msgPass,
        int blocked,
        const std::pair<MSVehicle*, SUMOReal>& leader,
        const std::pair<MSVehicle*, SUMOReal>& neighLead,
        const std::pair<MSVehicle*, SUMOReal>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked) 
{
    UNUSED_PARAMETER(firstBlocked);
    const int result = _wantsChange(laneOffset, msgPass, blocked, leader, neighLead, neighFollow, neighLane, preb, lastBlocked);
    return result;
}


SUMOReal
MSLCM_DK2013::patchSpeed(const SUMOReal min, const SUMOReal wanted, const SUMOReal max, const MSCFModel& cfModel) {
    const SUMOReal newSpeed = _patchSpeed(min, wanted, max, cfModel);
    myLastAccel = SPEED2ACCEL(newSpeed - myVehicle.getSpeed());
    return newSpeed;
}


SUMOReal
MSLCM_DK2013::_patchSpeed(const SUMOReal min, const SUMOReal wanted, const SUMOReal max, const MSCFModel& cfModel) {

    int state = myOwnState;

    // letting vehicles merge in at the end of the lane in case of counter-lane change, step#2
    SUMOReal MAGIC_offset = 1.;
    //   if we want to change and have a blocking leader and there is enough room for him in front of us
    if (myLeadingBlockerLength != 0) {
        SUMOReal space = myLeftSpace - myLeadingBlockerLength - MAGIC_offset - myVehicle.getVehicleType().getMinGap();
        if (space > 0) {
            // compute speed for decelerating towards a place which allows the blocking leader to merge in in front
            SUMOReal safe = cfModel.stopSpeed(&myVehicle, myVehicle.getSpeed(), space);
            // if we are approaching this place
            if (safe < wanted) {
                // return this speed as the speed to use
                return MAX2(min, safe);
            }
        }
    }

    // just to make sure to be notified about lane chaning end
    if (myVehicle.getLane()->getEdge().getLanes().size() == 1 || myVehicle.getLane()->getEdge().getPurpose() == MSEdge::EDGEFUNCTION_INTERNAL) {
        // remove chaning information if on a road with a single lane
        changed();
        return wanted;
    }

    SUMOReal nVSafe = wanted;
    bool gotOne = false;
    for (std::vector<SUMOReal>::const_iterator i = myVSafes.begin(); i != myVSafes.end(); ++i) {
        SUMOReal v = (*i);
        if (v >= min && v <= max) {
            nVSafe = MIN2(v, nVSafe);
            gotOne = true;
        }
    }

    // check whether the vehicle is blocked
    if ((state & LCA_WANTS_LANECHANGE) != 0) {
        if (gotOne && !myDontBrake) {
            return nVSafe;
        }
        if ((state & LCA_BLOCKED) != 0) {
            // adapt speed if the laneChange is done for ourselves (and not just to help)
            if ((state & LCA_CHANGE_TO_HELP) == 0) {
                if ((state & LCA_BLOCKED_BY_LEADER) != 0) {
                    // if interacting with leader and not too slow
                    return (min + wanted) / (SUMOReal) 2.0;
                }
                if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
                    return (max + wanted) / (SUMOReal) 2.0;
                }
            }
        }
    }

    // decelerate if being a blocking follower
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGFOLLOWER) != 0) {
        if (fabs(max - myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle)) < 0.001 && min == 0) { // !!! was standing
            return 0;
        }
        
        return (min + wanted) / (SUMOReal) 2.0; 
    }
    if ((state & LCA_AMBACKBLOCKER) != 0) {
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            return nVSafe;
        }
    }
    if ((state & LCA_AMBACKBLOCKER_STANDING) != 0) {
        return nVSafe;
    }
    // accelerate if being a blocking leader or blocking follower not able to brake
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGLEADER) != 0) {
        return (max + wanted) / (SUMOReal) 2.0;
    }

    return wanted;
}


void*
MSLCM_DK2013::inform(void* info, MSVehicle* /*sender*/) {
    /// XXX use info.first
    Info* pinfo = (Info*) info;
    //myOwnState &= 0xffffffff; // reset all bits of MyLCAEnum but only those
    myOwnState |= pinfo->second;
    delete pinfo;
    return (void*) true;
}


void
MSLCM_DK2013::changed() {
    myOwnState = 0;
    myLastLaneChangeOffset = 0;
    mySpeedGainProbability = 0;
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myVSafes.clear();
    myDontBrake = false;
}


void
MSLCM_DK2013::informBlocker(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                            int blocked,
                            int dir,
                            const std::pair<MSVehicle*, SUMOReal>& neighLead,
                            const std::pair<MSVehicle*, SUMOReal>& neighFollow) {
    if ((blocked & LCA_BLOCKED_BY_FOLLOWER) != 0) {
        assert(neighFollow.first != 0);
        MSVehicle* nv = neighFollow.first;
        // PARAMETERS
        // assume other vehicle will assume the equivalent of 1 second of
        // maximum deceleration to help use (will probably be spread over
        // multiple seconds)
        // -----------
        const SUMOReal helpDecel = nv->getCarFollowModel().getMaxDecel();

        // change in the gap between ego and blocker over 1 second (not STEP!)
        const SUMOReal egoNewSpeed = myVehicle.getSpeed() + myLastAccel;
        const SUMOReal neighNewSpeed = MAX2((SUMOReal)0, nv->getSpeed() - helpDecel);
        const SUMOReal deltaGap =  egoNewSpeed - neighNewSpeed; 
        // new gap between follower and self in case the follower does brake
        SUMOReal decelGap = neighFollow.second + deltaGap;
        if (deltaGap > 0 && decelGap > 0 
                && decelGap >= nv->getCarFollowModel().getSecureGap(neighNewSpeed, egoNewSpeed, myVehicle.getCarFollowModel().getMaxDecel())) {
            // if the blocking neighbor brakes it could actually help
            // how hard does it actually need to be?
            // XXX this part of the message is currently not evaluated
            SUMOReal vsafe = nv->getCarFollowModel().followSpeed(
                    &myVehicle, myVehicle.getSpeed(), neighFollow.second, neighFollow.first->getSpeed(), neighFollow.first->getCarFollowModel().getMaxDecel());
            msgPass.informNeighFollower(new Info(vsafe, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
        } else {
            SUMOReal vsafe = neighFollow.second <= 0 ? 0 : myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighFollow.second, neighFollow.first->getSpeed(), neighFollow.first->getCarFollowModel().getMaxDecel());
            msgPass.informNeighFollower(new Info(vsafe, dir | LCA_AMBLOCKINGFOLLOWER_DONTBRAKE), &myVehicle);
        }
    }
    if ((blocked & LCA_BLOCKED_BY_LEADER) != 0) {
        if (neighLead.first != 0 && neighLead.second > 0) {
            msgPass.informNeighLeader(new Info(0, dir | LCA_AMBLOCKINGLEADER), &myVehicle);
        }
    }
}


void
MSLCM_DK2013::prepareStep() {
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myVSafes.clear();
    myDontBrake = false;
    // truncate mySpeedGainProbability to work around numerical instability between different builds
    mySpeedGainProbability = ceil(mySpeedGainProbability * 100000.0) * 0.00001;
}


int
MSLCM_DK2013::_wantsChange(
        int laneOffset,
        MSAbstractLaneChangeModel::MSLCMessager& msgPass,
        int blocked,
        const std::pair<MSVehicle*, SUMOReal>& leader,
        const std::pair<MSVehicle*, SUMOReal>& neighLead,
        const std::pair<MSVehicle*, SUMOReal>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked) 
{
    assert(laneOffset == 1 || laneOffset == -1);
    // compute bestLaneOffset
    MSVehicle::LaneQ curr, best;
    int bestLaneOffset = 0;
    SUMOReal currentDist = 0;
    SUMOReal neighDist = 0;
    SUMOReal neighExtDist = 0;
    SUMOReal currExtDist = 0;
    int currIdx = 0;
    for (int p = 0; p < (int) preb.size(); ++p) {
        if (preb[p].lane == myVehicle.getLane()) {
            curr = preb[p];
            bestLaneOffset = curr.bestLaneOffset;
            currentDist = curr.length;
            currExtDist = curr.lane->getLength();
            neighDist = preb[p + laneOffset].length;
            neighExtDist = preb[p + laneOffset].lane->getLength();
            best = preb[p + bestLaneOffset];
            currIdx = p;
        }
    }
    // direction specific constants
    const bool right = (laneOffset == -1);
    const int lca = (right ? LCA_RIGHT : LCA_LEFT);
    const int myLca = (right ? LCA_MRIGHT : LCA_MLEFT);
    const int lcaCounter = (right ? LCA_LEFT : LCA_RIGHT);
    const bool changeToBest = (right && bestLaneOffset < 0) || (!right && bestLaneOffset > 0);
    // keep information about being a leader/follower
    int ret = (myOwnState & 0x00ffff00);

    // process information about the last blocked vehicle
    //  if this vehicle is blocking someone in front, we maybe decelerate to let him in
    if ((*lastBlocked) != 0) {
        SUMOReal gap = (*lastBlocked)->getPositionOnLane() - (*lastBlocked)->getVehicleType().getLength() - myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getMinGap();
        if (gap > 0.1) {
            if (myVehicle.getSpeed() < ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())) {
                if ((*lastBlocked)->getSpeed() < SUMO_const_haltingSpeed) {
                    ret |= LCA_AMBACKBLOCKER_STANDING;
                } else {
                    ret |= LCA_AMBACKBLOCKER;
                }
                myVSafes.push_back(myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), (SUMOReal)(gap - 0.1), (*lastBlocked)->getSpeed(), (*lastBlocked)->getCarFollowModel().getMaxDecel()));
                (*lastBlocked) = 0;
            }
            //return ret;
        }
    }

    // we try to estimate the distance which is necessary to get on a lane
    //  we have to get on in order to keep our route
    // we assume we need something that depends on our velocity
    // and compare this with the free space on our wished lane
    //
    // if the free space is somehow less than the space we need, we should
    //  definitely try to get to the desired lane
    //
    // this rule forces our vehicle to change the lane if a lane changing is necessary soon
    // lookAheadDistance:
    SUMOReal laDist = myVehicle.getSpeed() > LOOK_FORWARD_SPEED_DIVIDER
                  ? myVehicle.getSpeed() * (SUMOReal) LOOK_FORWARD_FAR
                  : myVehicle.getSpeed() * (SUMOReal) LOOK_FORWARD_NEAR;
    laDist += myVehicle.getVehicleType().getLengthWithGap() * (SUMOReal) 2.;
    // free space that is available for changing
    //const SUMOReal neighSpeed = (neighLead.first != 0 ? neighLead.first->getSpeed() :
    //        neighFollow.first != 0 ? neighFollow.first->getSpeed() :
    //        best.lane->getSpeedLimit());
    // @note: while this lets vehicles change earlier into the correct direction
    // it also makes the vehicles more "selfish" and prevents changes which are necessary to help others

    const SUMOReal usableDist = (currentDist - myVehicle.getPositionOnLane() - best.occupation * (SUMOReal) JAM_FACTOR);

    if (fabs(best.length - currentDist) > MIN2((SUMOReal) .1, best.lane->getLength()) 
            && changeToBest
            && currentDistDisallows(usableDist, bestLaneOffset, laDist)) {

        informBlocker(msgPass, blocked, myLca, neighLead, neighFollow);
        if (neighLead.second > 0 && neighLead.second > leader.second) {
            myVSafes.push_back(myCarFollowModel.followSpeed(
                        &myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()) - (SUMOReal) 0.5);
        }

        // letting vehicles merge in at the end of the lane in case of counter-lane change, step#1
        //   if there is a leader and he wants to change to the opposite direction
        if (neighLead.first != 0 && (neighLead.first->getLaneChangeModel().getOwnState() & lcaCounter) != 0) {
            // save at least his length in myLeadingBlockerLength
            myLeadingBlockerLength = MAX2(neighLead.first->getVehicleType().getLengthWithGap(), myLeadingBlockerLength);
            // save the left space
            myLeftSpace = currentDist - myVehicle.getPositionOnLane();
        }
        //
        return ret | lca | LCA_URGENT;
    }


    // the opposite lane-changing direction should be done than the one examined herein
    //  we'll check whether we assume we could change anyhow and get back in time...
    //
    // this rule prevents the vehicle from moving in opposite direction of the best lane
    //  unless the way till the end where the vehicle has to be on the best lane
    //  is long enough
    SUMOReal maxJam = MAX2(preb[currIdx + laneOffset].occupation, preb[currIdx].occupation);
    SUMOReal neighLeftPlace = MAX2((SUMOReal) 0, neighDist - myVehicle.getPositionOnLane() - maxJam);
    if (!changeToBest && (currentDistDisallows(neighLeftPlace, bestLaneOffset - (2 * laneOffset), laDist))) {
        // ...we will not change the lane if not
        return ret;
    }


    // if the current lane is the best and a lane-changing would cause a situation
    //  of which we assume we will not be able to return to the lane we have to be on...
    //
    // this rule prevents the vehicle from leaving the current, best lane when it is
    //  close to this lane's end
    if (currExtDist > neighExtDist && (neighLeftPlace * 2. < laDist)) {
        return ret;
    }

    // let's also regard the case where the vehicle is driving on a highway...
    //  in this case, we do not want to get to the dead-end of an on-ramp
    if (right) {
        if (bestLaneOffset == 0 && preb[currIdx + laneOffset].bestLaneOffset != 0 && myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle) > 80. / 3.6) {
            return ret;
        }
    }
    // --------

    // -------- make place on current lane if blocking follower
    if (amBlockingFollowerPlusNB()
            && (currentDistAllows(neighDist, bestLaneOffset, laDist) || neighDist >= currentDist)) {

        return ret | lca | LCA_URGENT ;
    }
    // --------


    // -------- security checks for krauss
    //  (vsafe fails when gap<0)
    if ((blocked & LCA_BLOCKED) != 0) {
        return ret;
    }
    // --------

    // -------- higher speed
    if ((congested(neighLead.first) && neighLead.second < 20) || predInteraction(leader.first)) { //!!!
        return ret;
    }
    SUMOReal thisLaneVSafe = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);
    SUMOReal neighLaneVSafe = neighLane.getVehicleMaxSpeed(&myVehicle);
    if (neighLead.first == 0) {
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighDist, 0, 0));
    } else {
        // @todo: what if leader is below safe gap?!!!
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(
                    &myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()));
    }
    if (leader.first == 0) {
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), currentDist, 0, 0));
    } else {
        // @todo: what if leader is below safe gap?!!!
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), leader.second, leader.first->getSpeed(), leader.first->getCarFollowModel().getMaxDecel()));
    }

    thisLaneVSafe = MIN2(thisLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());
    neighLaneVSafe = MIN2(neighLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());

    if (right) {
        // ONLY FOR CHANGING TO THE RIGHT
        if (thisLaneVSafe - neighLaneVSafe > 5. / 3.6) {
            // ok, the current lane is faster than the right one...
            if (mySpeedGainProbability < 0) {
                mySpeedGainProbability /= 2.0;
            }
        } else {
            // ok, the right lane is faster than the current
            mySpeedGainProbability -= (SUMOReal)((neighLaneVSafe - thisLaneVSafe) / (myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle)));
        }

        // let's recheck the "Rechtsfahrgebot"
        SUMOReal vmax = MIN2(myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle), myVehicle.getVehicleType().getMaxSpeed());
        vmax -= (SUMOReal)(5. / 2.6);
        if (neighLaneVSafe >= vmax) {
#ifndef NO_TRACI
            /* if there was a request by TraCI for changing to this lane
            and holding it, this rule is ignored */
            if (myChangeRequest != MSVehicle::REQUEST_HOLD) {
#endif
                mySpeedGainProbability -= (SUMOReal)((neighLaneVSafe - vmax) / (vmax));
#ifndef NO_TRACI
            }
#endif
        }

        if (mySpeedGainProbability < -2 && neighDist / MAX2((SUMOReal) .1, myVehicle.getSpeed()) > 20.) { //./MAX2((SUMOReal) .1, myVehicle.getSpeed())) { // -.1
            return ret | lca | LCA_SPEEDGAIN;
        }
    } else {
        // ONLY FOR CHANGING TO THE LEFT
        if (thisLaneVSafe > neighLaneVSafe) {
            // this lane is better
            if (mySpeedGainProbability > 0) {
                mySpeedGainProbability /= 2.0;
            }
        } else {
            // left lane is better
            mySpeedGainProbability += (SUMOReal)((neighLaneVSafe - thisLaneVSafe) / (myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle))); // !!! Fahrzeuggeschw.!
        }
        if (mySpeedGainProbability > .2 && neighDist / MAX2((SUMOReal) .1, myVehicle.getSpeed()) > 20.) { // .1
            return ret | lca | LCA_SPEEDGAIN;
        }
    }
    // --------

#ifndef NO_TRACI
    // If there is a request by TraCI, try to change the lane
    if ((right && myChangeRequest == MSVehicle::REQUEST_RIGHT) || 
            (!right && myChangeRequest == MSVehicle::REQUEST_LEFT)) {
        return ret | lca;
    }
#endif

    return ret;
}

/****************************************************************************/

