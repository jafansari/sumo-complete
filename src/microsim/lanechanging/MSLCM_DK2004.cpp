//---------------------------------------------------------------------------//
//                        MSLCM_DK2004.cpp -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 29.05.2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.11  2006/04/05 05:27:34  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.10  2005/10/07 11:37:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.9  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.8  2005/09/15 11:07:36  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.7  2005/07/15 07:18:39  dkrajzew
// code style applied
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include "MSLCM_DK2004.h"
#include <iostream>

#ifdef ABS_DEBUG
#include "../MSDebugHelper.h"
#endif


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * some definitions (debugging only)
 * ======================================================================= */
#define DEBUG_OUT cout


/* =========================================================================
 * variable definitions
 * ======================================================================= */
std::string id_leader("367719");
std::string id_vehicle("366338");
std::string id_follower("4536");
std::string id_pfollower("1131");
size_t searchedtime = 21900;

#ifdef GUI_DEBUG
#include <utils/gui/div/GUIGlobalSelection.h>
#include <guisim/GUIVehicle.h>
#endif

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG

// 80km/h will be the swell for dividing between long/short foresight
#define LOOK_FORWARD_SPEED_DIVIDER 12.22

#define LOOK_FORWARD_FAR 15



#define LONG_FORESIGHT


/* =========================================================================
 * member method definitions
 * ======================================================================= */
MSLCM_DK2004::MSLCM_DK2004(MSVehicle &v)
    : MSAbstractLaneChangeModel(v),
    myChangeProbability(0),
    myVSafe(0)
{
}

MSLCM_DK2004::~MSLCM_DK2004()
{
    changed();
}

int
MSLCM_DK2004::wantsChangeToRight(MSAbstractLaneChangeModel::MSLCMessager &msgPass,
                                 int blocked,
                                 const std::pair<MSVehicle*, SUMOReal> &leader,
                                 const std::pair<MSVehicle*, SUMOReal> &neighLead,
                                 const std::pair<MSVehicle*, SUMOReal> &neighFollow,
                                 const MSLane &neighLane,
                                 int bestLaneOffset, SUMOReal bestDist,
                                 SUMOReal neighDist,
                                 SUMOReal currentDist,
                                 MSVehicle **lastBlocked)
{
    /*
    if(gSelected.isSelected(GLO_VEHICLE, static_cast<GUIVehicle&>(myVehicle).getGlID())) {
        int blb = 0;
    }
    */
#ifdef ABS_DEBUG
    if(debug_globaltime>=debug_searchedtime && (myVehicle.getID()==debug_searched1||myVehicle.getID()==debug_searched2)) {
        DEBUG_OUT << "bla" << endl;
    }
#endif
    // keep information about being a leader/follower
    int ret = (myState&0x00ffff00);

    if( leader.first!=0
        &&
        (myState&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0
        &&
        (leader.first->getLaneChangeModel().getState()&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0) {

        myState &= (0xffffffff-LCA_AMBLOCKINGFOLLOWER_DONTBRAKE);
        if(myVehicle.speed()>0.1) {
            myState |= LCA_AMBACKBLOCKER;
        } else {
            ret |= LCA_AMBACKBLOCKER;
            myVSafe = 0;
        }
    }

    if((*lastBlocked)!=0) {
        SUMOReal gap = (*lastBlocked)->pos()-(*lastBlocked)->length()-myVehicle.pos();
/*        if(gap>=1.0) {
            SUMOReal decelGap = gap
                + SPEED2DIST(myVehicle.speed()) * 2.0
                - MAX2((*lastBlocked)->speed() - (*lastBlocked)->decelAbility() * 2.0, 0);
            if(decelGap>0&&(*lastBlocked)->isSafeChange_WithDistance(decelGap, myVehicle, &(*lastBlocked)->getLane())) {
                ret |= LCA_AMBACKBLOCKER;
                SUMOReal vSafe = myVehicle.vsafe(myVehicle.speed(),
                    myVehicle.decelAbility(), gap-1.0, (*lastBlocked)->speed());
                if(myVSafe>=0) {
                    myVSafe = MIN2(myVSafe, vSafe);
                } else {
                    myVSafe = vSafe;
                }
                (*lastBlocked) = 0;
            }
        } else */
        if(gap>0.1) {
            if(myVehicle.speed()<myVehicle.decelAbility()) {
                if((*lastBlocked)->speed()<0.1) {
                    ret |= LCA_AMBACKBLOCKER_STANDING;
                    myVSafe = myVehicle.vsafe(myVehicle.speed(),
                        myVehicle.decelAbility(), (SUMOReal) (gap-0.1), (*lastBlocked)->speed());
                    (*lastBlocked) = 0;
                } else {
                    ret |= LCA_AMBACKBLOCKER;
                    myVSafe = myVehicle.vsafe(myVehicle.speed(),
                        myVehicle.decelAbility(), (SUMOReal) (gap-0.1), (*lastBlocked)->speed());
                    (*lastBlocked) = 0;
                }
            }
        }
    }

    // -------- forced changing
    SUMOReal rv = /*neighLead.first!=0&&myVehicle.speed()>myVehicle.accelAbility()
        ? neighLead.first->speed()+20.0
        :*/
        //myVehicle.getLane().maxSpeed() * 5;
        myVehicle.getLane().maxSpeed() > LOOK_FORWARD_SPEED_DIVIDER
        ? myVehicle.getLane().maxSpeed() * LOOK_FORWARD_FAR
        : myVehicle.getLane().maxSpeed() * 5;

    if( bestLaneOffset<0&&currentDistDisallows(currentDist, bestLaneOffset, rv)) {
        informBlocker(msgPass, blocked, LCA_MRIGHT, neighLead, neighFollow);
        if(neighLead.second>0&&neighLead.second>leader.second) {
            myVSafe = myVehicle.vsafe(myVehicle.speed(),
                myVehicle.decelAbility(), neighLead.second, neighLead.first->speed())
                - (SUMOReal) 0.5;
        }
        return ret|LCA_RIGHT|LCA_URGENT|blocked;
    }
    // left lanes are better...
    if( bestLaneOffset>0&&currentDistDisallows(currentDist, bestLaneOffset, rv)/*&&currentDist!=neighDist*/) {
        return ret;
    }
    // the current lane is better
    if(neighDist<rv&&bestLaneOffset==0) {
        return ret;
    }
    // --------

    // -------- make place on current lane if blocking follower
    if( amBlockingFollowerPlusNB()
        &&
        (currentDistAllows(neighDist, bestLaneOffset, rv)||neighDist>=currentDist) ) {

        return ret|LCA_RIGHT|LCA_URGENT|LCA_KEEP1;
    }
    // --------


    // -------- security checks for krauss
    //  (vsafe fails when gap<0)
    if((blocked&(LCA_BLOCKEDBY_FOLLOWER|LCA_BLOCKEDBY_LEADER))!=0) {
        return ret;
    }
    // --------

    // -------- higher speed
    if(congested( neighLead.first )||predInteraction(leader.first)) {
        return ret;
    }
    SUMOReal neighLaneVSafe, thisLaneVSafe;
    if ( neighLead.first == 0 ) {
        neighLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                neighLane.length() - myVehicle.pos(), 0);
    } else {
        assert(neighLead.second>=0);
        neighLaneVSafe =
            myVehicle.vsafe(myVehicle.speed(), myVehicle.decelAbility(),
                neighLead.second, neighLead.first->speed());
    }
    if(leader.first==0) {
        thisLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                myVehicle.getLane().length() - myVehicle.pos(), 0);
    } else {
        assert(leader.second>=0);
        thisLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                leader.second, leader.first->speed());
    }

    thisLaneVSafe =
        MIN3(thisLaneVSafe, myVehicle.getLane().maxSpeed(), myVehicle.getVehicleType().maxSpeed());
    neighLaneVSafe =
        MIN3(neighLaneVSafe, neighLane.maxSpeed(), myVehicle.getVehicleType().maxSpeed());
    if(thisLaneVSafe>neighLaneVSafe) {
        // this lane is better
        if(myChangeProbability<0) {
            myChangeProbability /= 2.0;
        }
    } else {
        // right lane is better
        myChangeProbability -= (SUMOReal)
            ((neighLaneVSafe-thisLaneVSafe) / (myVehicle.getLane().maxSpeed()));
    }
    if(myChangeProbability<-.2) {
        return ret | LCA_RIGHT|LCA_SPEEDGAIN;
    }
    // --------
    return ret;
}


int
MSLCM_DK2004::wantsChangeToLeft(MSAbstractLaneChangeModel::MSLCMessager &msgPass,
                                int blocked,
                                const std::pair<MSVehicle*, SUMOReal> &leader,
                                const std::pair<MSVehicle*, SUMOReal> &neighLead,
                                const std::pair<MSVehicle*, SUMOReal> &neighFollow,
                                const MSLane &neighLane,
                                int bestLaneOffset, SUMOReal bestDist,
                                SUMOReal neighDist,
                                SUMOReal currentDist,
                                MSVehicle **lastBlocked)
{
    /*
    if(gSelected.isSelected(GLO_VEHICLE, static_cast<GUIVehicle&>(myVehicle).getGlID())) {
        int blb = 0;
    }
    */
#ifdef ABS_DEBUG
    if(debug_globaltime>=debug_searchedtime && (myVehicle.getID()==debug_searched1||myVehicle.getID()==debug_searched2)) {
        DEBUG_OUT << "bla" << endl;
    }
#endif
    // keep information about being a leader/follower
    int ret = (myState&0x00ffff00);

    if( leader.first!=0
        &&
        (myState&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0
        &&
        (leader.first->getLaneChangeModel().getState()&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0) {

        myState &= (0xffffffff-LCA_AMBLOCKINGFOLLOWER_DONTBRAKE);
        if(myVehicle.speed()>0.1) {
            myState |= LCA_AMBACKBLOCKER;
        } else {
            ret |= LCA_AMBACKBLOCKER;
            myVSafe = 0;
        }
    }

    if((*lastBlocked)!=0) {
        SUMOReal gap = (*lastBlocked)->pos()-(*lastBlocked)->length()-myVehicle.pos();
        /*
        if(gap>=1.0) {
            SUMOReal decelGap = gap
                + SPEED2DIST(myVehicle.speed()) * 2.0
                - MAX2((*lastBlocked)->speed() - (*lastBlocked)->decelAbility() * 2.0, 0);
            if(decelGap>0&&(*lastBlocked)->isSafeChange_WithDistance(decelGap, myVehicle, &(*lastBlocked)->getLane())) {
                ret |= LCA_AMBACKBLOCKER;
                SUMOReal vSafe = myVehicle.vsafe(myVehicle.speed(),
                    myVehicle.decelAbility(),
                   gap-1.0, (*lastBlocked)->speed());
                myVSafe = MIN2(myVSafe, vSafe);
                (*lastBlocked) = 0;
                if(myVSafe>=0) {
                    myVSafe = MIN2(myVSafe, vSafe);
                } else {
                    myVSafe = vSafe;
                }
            }
        } else if(gap>0&&myVehicle.speed()<myVehicle.decelAbility()) {
            ret |= LCA_AMBACKBLOCKER;
            (*lastBlocked) = 0;
            myVSafe = 0;
        }*/
        if(gap>0.1) {
            if(myVehicle.speed()<myVehicle.decelAbility()) {
                if((*lastBlocked)->speed()<0.1) {
                    ret |= LCA_AMBACKBLOCKER_STANDING;
                    myVSafe = myVehicle.vsafe(myVehicle.speed(),
                        myVehicle.decelAbility(), (SUMOReal) (gap-0.1), (*lastBlocked)->speed());
                    (*lastBlocked) = 0;
                } else {
                    ret |= LCA_AMBACKBLOCKER;
                    myVSafe = myVehicle.vsafe(myVehicle.speed(),
                        myVehicle.decelAbility(), (SUMOReal) (gap-0.1), (*lastBlocked)->speed());
                    (*lastBlocked) = 0;
                }
            }
        }
    }

    // -------- forced changing
    SUMOReal lv = /*neighLead.first!=0&&myVehicle.speed()>myVehicle.accelAbility()
        ? neighLead.first->speed()+20.0
        : */
        //myVehicle.getLane().maxSpeed() * 5;
        myVehicle.getLane().maxSpeed() > LOOK_FORWARD_SPEED_DIVIDER
        ? myVehicle.getLane().maxSpeed() * LOOK_FORWARD_FAR
        : myVehicle.getLane().maxSpeed() * 5;
    if( bestLaneOffset>0
        &&
        currentDistDisallows(currentDist, bestLaneOffset, lv)) {

        informBlocker(msgPass, blocked, LCA_MLEFT, neighLead, neighFollow);
        if(neighLead.second>0&&neighLead.second>leader.second) {
            myVSafe = myVehicle.vsafe(myVehicle.speed(),
                myVehicle.decelAbility(), neighLead.second, neighLead.first->speed())
                - (SUMOReal) 0.5;
        }
        return ret|LCA_LEFT|LCA_URGENT|blocked;
    }
    // left lanes are better...
    if( bestLaneOffset<0&&currentDistDisallows(currentDist, bestLaneOffset, lv)/*&&currentDist!=neighDist*/) {
        return ret;
    }
    // the current lane is better
    if( neighDist<lv&&bestLaneOffset==0) {
        return ret;
    }
    // --------

    // -------- make place on current lane if blocking follower
    if( amBlockingFollowerPlusNB()
        &&
        (currentDistAllows(neighDist, bestLaneOffset, lv)||neighDist>=currentDist) ) {

        return ret|LCA_LEFT|LCA_URGENT|LCA_KEEP1;
    }
    // --------

    // -------- security checks for krauss
    //  (vsafe fails when gap<0)
    if((blocked&(LCA_BLOCKEDBY_FOLLOWER|LCA_BLOCKEDBY_LEADER))!=0) {
        return ret;
    }

    // -------- higher speed
    if(congested( neighLead.first )) {
        return ret;
    }
    SUMOReal neighLaneVSafe, thisLaneVSafe;
    if ( neighLead.first == 0 ) {
        neighLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                neighLane.length() - myVehicle.pos(), 0);
    } else {
        assert(neighLead.second>=0);
        neighLaneVSafe =
            myVehicle.vsafe(myVehicle.speed(), myVehicle.decelAbility(),
                neighLead.second, neighLead.first->speed());
    }
    if(leader.first==0) {
        thisLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                myVehicle.getLane().length() - myVehicle.pos(), 0);
    } else {
        assert(leader.second>=0);
        thisLaneVSafe =
            myVehicle.vsafe(
                myVehicle.speed(), myVehicle.decelAbility(),
                leader.second, leader.first->speed());
    }
    thisLaneVSafe =
        MIN3(thisLaneVSafe, myVehicle.getLane().maxSpeed(), myVehicle.getVehicleType().maxSpeed());
    neighLaneVSafe =
        MIN3(neighLaneVSafe, neighLane.maxSpeed(), myVehicle.getVehicleType().maxSpeed());
    if(thisLaneVSafe>neighLaneVSafe) {
        // this lane is better
        if(myChangeProbability>0) {
            myChangeProbability /= 2.0;
        }
    } else {
        // right lane is better
        myChangeProbability += (SUMOReal)
            ((neighLaneVSafe-thisLaneVSafe) / (myVehicle.getLane().maxSpeed()));
    }
    if(myChangeProbability>.2) {
        return ret | LCA_LEFT|LCA_SPEEDGAIN|LCA_URGENT;
    }
    // --------
    return ret;
}


SUMOReal
MSLCM_DK2004::patchSpeed(SUMOReal min, SUMOReal wanted, SUMOReal max, SUMOReal vsafe)
{
    /*
    if(gSelected.isSelected(GLO_VEHICLE, static_cast<GUIVehicle&>(myVehicle).getGlID())) {
        int blb= 0;
    }
    */
    SUMOReal vSafe = myVSafe;
    int state = myState;
    myState = 0;
    myVSafe = -1;
    // just to make sure to be notified about lane chaning end
    if(myVehicle.getLane().edge().nLanes()==1) {
        // remove chaning information if on a road with a single lane
        changed();
    }

    // check whether the vehicle is blocked
    if((state&LCA_URGENT)!=0) {
        if(vSafe>0) {
            return MIN2(max, MAX2(min, vSafe));
        }
        // check whether the vehicle maybe has to be swapped with one of
        //  the blocking vehicles
        if( (state&LCA_BLOCKEDBY_LEADER)!=0
            &&
            (state&LCA_BLOCKEDBY_FOLLOWER)!=0) {

            return (min+wanted)/(SUMOReal) 2.0;//wanted;
        } else {
            if((state&LCA_BLOCKEDBY_LEADER)!=0) {
                // if interacting with leader and not too slow
                return (min+wanted)/(SUMOReal) 2.0;
            }
            if((state&LCA_BLOCKEDBY_FOLLOWER)!=0) {
                return (max+wanted)/(SUMOReal) 2.0;
            }
        }
    }


    // decelerate if being a blocking follower
    //  (and does not have to change lanes)
    if((state&LCA_AMBLOCKINGFOLLOWER)!=0) {
        if(max==myVehicle.accelAbility()&&min==0) {
            return 0;
        }
        if(myVSafe<=0) {
            return (min+wanted)/(SUMOReal) 2.0;
        }
        return min;//MAX2(min, MIN2(vSafe, wanted));
    }
    if((state&LCA_AMBACKBLOCKER)!=0) {
        if(max<=myVehicle.accelAbility()&&min==0) {
            return MAX2(min, MIN2(vSafe, wanted));
        }
    }
    if((state&LCA_AMBACKBLOCKER_STANDING)!=0) {
        return MAX2(min, MIN2(vSafe, wanted));
    }
    // accelerate if being a blocking leader or blocking follower not able to brake
    //  (and does not have to change lanes)
    if((state&LCA_AMBLOCKINGLEADER)!=0) {
        return (max+wanted)/(SUMOReal) 2.0;
    }
    /*
    if((state&LCA_AMBACKBLOCKER)!=0) {
        if(max<=myVehicle.accelAbility()&&min==0) {
            return MAX2(min, MIN2(vSafe, wanted));
        }
        if(myVSafe>(min+wanted)/2.0) {
            return MIN2(vSafe, wanted);
        }
        return (min+wanted)/2.0;
    }
    */
    if((state&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0) {
        if(max<=myVehicle.accelAbility()&&min==0) {
            return wanted;
        }
        if(myVSafe>(min+wanted)/(SUMOReal) 2.0) {
            return MIN2(vSafe, wanted);
        }
        return (min+wanted)/(SUMOReal) 2.0;
    }
    return wanted;
}


void *
MSLCM_DK2004::inform(void *info, MSVehicle *sender)
{
    Info *pinfo = (Info*) info;
    if(pinfo->second==LCA_UNBLOCK) {
        myState &= 0xffff00ff;
        return (void*) true;
    }
    myState &= 0xffffffff;
    myState |= pinfo->second;
    if(pinfo->first>=0) {
        myVSafe = MIN2(myVSafe, pinfo->first);
    }
    delete pinfo;
    return (void*) true;
}


void
MSLCM_DK2004::changed()
{
    myChangeProbability = 0;
    myState = 0;
}


void
MSLCM_DK2004::informBlocker(MSAbstractLaneChangeModel::MSLCMessager &msgPass,
                            int &blocked,
                            int dir,
                            const std::pair<MSVehicle*, SUMOReal> &neighLead,
                            const std::pair<MSVehicle*, SUMOReal> &neighFollow)
{
    /*
    if(gSelected.isSelected(GLO_VEHICLE, static_cast<GUIVehicle&>(myVehicle).getGlID())) {
        int blb = 0;
    }
    */
    if((blocked&LCA_BLOCKEDBY_FOLLOWER)!=0) {
        assert(neighFollow.first!=0);
        MSVehicle *nv = neighFollow.first;
        SUMOReal decelGap =
            neighFollow.second
            + SPEED2DIST(myVehicle.speed()) * (SUMOReal) 2.0
            - MAX2(nv->speed() - nv->decelAbility() * (SUMOReal) 2.0, (SUMOReal) 0);
        if(neighFollow.second>0&&decelGap>0&&nv->isSafeChange_WithDistance(decelGap, myVehicle, &nv->getLane())) {
            SUMOReal vsafe = myVehicle.vsafe(myVehicle.speed(), myVehicle.decelAbility(),
                neighFollow.second, neighFollow.first->speed());
            msgPass.informNeighFollower(
                (void*) new Info(vsafe, dir|LCA_AMBLOCKINGFOLLOWER), &myVehicle);
        } else {
            SUMOReal vsafe = neighFollow.second<=0
                ? 0
                : myVehicle.vsafe(myVehicle.speed(), myVehicle.decelAbility(),
                    neighFollow.second, neighFollow.first->speed());
            msgPass.informNeighFollower(
                (void*) new Info(vsafe, dir|LCA_AMBLOCKINGFOLLOWER_DONTBRAKE), &myVehicle);
        }
    }
    if((blocked&LCA_BLOCKEDBY_LEADER)!=0) {
        if(neighLead.first!=0&&neighLead.second>0) {
            msgPass.informNeighLeader(
                (void*) new Info(0, dir|LCA_AMBLOCKINGLEADER), &myVehicle);
        }
    }
}


void
MSLCM_DK2004::prepareStep()
{
}


SUMOReal
MSLCM_DK2004::getProb() const
{
    return myChangeProbability;
}
/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

