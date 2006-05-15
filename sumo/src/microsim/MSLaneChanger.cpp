/***************************************************************************
                          MSLaneChanger.cpp  -  Handles lane-changes within
                          the edge's lanes.
                             -------------------
    begin                : Fri, 01 Feb 2002
    copyright            : (C) 2002 by Christian Roessel
    email                : roessel@zpr.uni-koeln.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.27  2006/05/15 05:50:40  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.27  2006/05/08 11:08:33  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.26  2006/04/05 05:27:34  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.25  2006/03/17 09:01:12  dkrajzew
// .icc-files removed
//
// Revision 1.24  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.23  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.22  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.21  2005/05/04 08:26:48  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; debugging
//
// Revision 1.20  2005/02/17 10:33:37  dkrajzew
// code beautifying;
// Linux building patched;
// warnings removed;
// new configuration usage within guisim
//
// Revision 1.19  2004/12/16 12:25:26  dkrajzew
// started a better vss handling
//
// Revision 1.18  2004/11/23 10:20:10  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.17  2004/08/02 12:07:01  dkrajzew
// first steps towards a lane-changing model API
//
// Revision 1.16  2004/03/19 13:09:40  dkrajzew
// debugging
//
// Revision 1.15  2004/02/18 05:29:11  dkrajzew
// false resetting of values after swap patched
//
// Revision 1.14  2003/12/12 12:55:59  dkrajzew
// looking back implemented
//
// Revision 1.13  2003/11/11 08:36:21  dkrajzew
// removed some debug-variables
//
// Revision 1.12  2003/10/16 08:33:49  dkrajzew
// new lane changing rules implemented
//
// Revision 1.11  2003/10/15 11:40:59  dkrajzew
// false rules removed; initial state for further tests
//
// Revision 1.10  2003/07/18 12:35:04  dkrajzew
// removed some warnings
//
// Revision 1.9  2003/07/16 15:29:51  dkrajzew
// myFirstState renamed to myLastState in MSLane
//
// Revision 1.8  2003/06/18 11:30:26  dkrajzew
// debug outputs now use a DEBUG_OUT macro instead of cout;
//  this shall ease the search for further couts which must be redirected to
//  the messaaging subsystem
//
// Revision 1.7  2003/05/20 09:31:46  dkrajzew
// emission debugged; movement model reimplemented (seems ok); detector output
//  debugged; setting and retrieval of some parameter added
//
// Revision 1.6  2003/03/20 16:21:12  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.5  2003/03/03 14:56:21  dkrajzew
// some debugging; new detector types added; actuated traffic lights added
//
// Revision 1.4  2003/02/07 10:41:50  dkrajzew
// updated
//
// Revision 1.3  2002/10/29 10:42:50  dkrajzew
// problems accured due to the deletion of a vehicle that reached his
//  destination debugged
//
// Revision 1.2  2002/10/28 12:56:52  dkrajzew
// forgot reinitialisation of hopped vehicle added
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.13  2002/09/25 18:14:09  roessel
// Bugfix: advan2left(): a succLink was implicitly converted to
// bool. Thanks to g++-3.2
//
// Revision 1.12  2002/09/25 17:14:42  roessel
// MeanData calculation and output implemented.
//
// Revision 1.11  2002/07/31 17:33:00  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.12  2002/07/26 11:13:57  dkrajzew
// safeChange debugged
//
// Revision 1.11  2002/07/03 15:50:56  croessel
// safeChange uses now MSVehicle::isSafeChange instead of safeChange.
//
// Revision 1.10  2002/05/29 17:06:03  croessel
// Inlined some methods. See the .icc files.
//
// Revision 1.9  2002/05/22 16:39:35  croessel
// advan2left(): First vehicles going to use a prioritized link will not
// be allowed to change to the left because stayState calculation assumes
// deceleration.
//
// Revision 1.8  2002/05/17 12:36:37  croessel
// advan2left/right: Check if vehicle brakes too much because of laneChangers
// disability to look beyond a lane. This caused many crashes.
//
// Revision 1.7  2002/05/08 09:29:56  croessel
// Last change, i.e. test if vrh(target)!=0, moved from change2left/right
// to congested().
//
// Revision 1.6  2002/05/02 13:46:57  croessel
// change2left/right(): Check if veh(target) exists before passing target as
// argument.
//
// Revision 1.5  2002/04/24 13:41:57  croessel
// Changed the condition-order in change2left(). target-validity-ckeck
// must be the first one.
//
// Revision 1.4  2002/04/18 12:18:39  croessel
// Bug-fix: Problem was that a right and a left vehicle could change to a
// middle lane, even though they were overlapping. Solution: Introduction
// of hoppedVeh in ChangeElem and method overlapWithHopped().
//
// Revision 1.3  2002/04/18 11:37:33  croessel
// In updateLanes(): changed swap() against assignment.
//
// Revision 1.2  2002/04/11 15:25:56  croessel
// Changed SUMOReal to SUMOReal.
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.2  2002/03/21 14:00:06  croessel
// Bug-fix in change(): Vehicles that keep to their lane must also be
// inserted in the tmpVehicles-container.
// Error-prone assert() in candiOnAllowed removed.
//
// Revision 2.1  2002/03/21 11:31:53  croessel
// Changed onAllowed to candiOnAllowed. Now the changeCandidate checks
// if a given lane is suitable for him. In change2right/left it is
// checked, that the target is a valid lane.
//
// Revision 2.0  2002/02/14 14:43:17  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.2  2002/02/13 10:31:46  croessel
// Modified the lane-changing conditions by using
// MSVehicleType::maxLength() and MSVehicleType::minDecel() in cases,
// where the change-candidate has no successor or predecessor.
//
// Revision 1.1  2002/02/05 11:53:02  croessel
// Initial commit.
//
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

#include "MSLaneChanger.h"
#include "MSVehicle.h"
#include "MSVehicleType.h"
#include "MSVehicleTransfer.h"
#include "MSGlobals.h"
#include <cassert>
#include <iterator>
#include <cstdlib>
#include <cmath>
#include <microsim/MSAbstractLaneChangeModel.h>
#include <utils/common/MsgHandler.h>

#ifdef ABS_DEBUG
#include "MSDebugHelper.h"
#endif

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * some definitions (debugging only)
 * ======================================================================= */
#define DEBUG_OUT cout


/* =========================================================================
 * member method definitions
 * ======================================================================= */
MSLaneChanger::~MSLaneChanger()
{

}

//-------------------------------------------------------------------------//

MSLaneChanger::MSLaneChanger( MSEdge::LaneCont* lanes )
{
    assert( lanes->size() > 1 );

    // Fill the changer with the lane-data.
    myChanger.reserve( lanes->size() );
    for ( MSEdge::LaneCont::iterator lane = lanes->begin();
          lane != lanes->end(); ++lane ) {
        ChangeElem ce;
        ce.follow    = 0;
        ce.lead      = 0;
        ce.lane      = *lane;
        ce.veh       = ( *lane )->myVehicles.rbegin();
        ce.hoppedVeh = 0;
        ce.lastBlocked = 0;
        myChanger.push_back( ce );
    }
}

//-------------------------------------------------------------------------//

void
MSLaneChanger::laneChange()
{
    // This is what happens in one timestep. After initialization of the
    // changer, each vehicle will try to change. After that the changer
    // nedds an update to prevent multiple changes of one vehicle.
    // Finally, the change-result has to be given back to the lanes.
    initChanger();
    while( vehInChanger() ) {

        bool haveChanged = change();
        updateChanger(haveChanged);
    }
    updateLanes();
}

//-------------------------------------------------------------------------//

void
MSLaneChanger::initChanger()
{
    // Prepare myChanger with a safe state.
    for ( ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce ) {
        ce->lead = 0;
        ce->hoppedVeh = 0;
        ce->lastBlocked = 0;

        MSLane::VehCont& vehicles = ce->lane->myVehicles;
        if ( vehicles.empty() ) {
            ce->veh  = vehicles.rend();
            ce->follow = 0;
            continue;
        }
        ce->veh  = vehicles.rbegin();
        if ( vehicles.size() == 1 ) {
            ce->follow = 0;
            continue;
        }
        ce->follow = *( vehicles.rbegin() + 1 );
    }
}

//-------------------------------------------------------------------------//
#ifdef GUI_DEBUG
#include <utils/gui/div/GUIGlobalSelection.h>
#include <guisim/GUIVehicle.h>
#endif


bool
MSLaneChanger::change()
{
    // Find change-candidate. If it is on an allowed lane, try to change
    // to the right (there is a rule in Germany that you have to change
    // to the right, unless you are overtaking). If change to the right
    // isn't possible, check if there is a possibility to overtake (on the
    // left.
    // If candidate isn't on an allowed lane, changing to an allowed has
    // priority.
    myCandi = findCandidate();
    MSVehicle* vehicle = veh( myCandi );
    /*
    if(gSelected.isSelected(GLO_VEHICLE, static_cast<GUIVehicle*>(vehicle)->getGlID())) {
        int blb = 0;
    }
    */

#ifdef ABS_DEBUG
    if(debug_globaltime>=debug_searchedtime && (vehicle->getID()==debug_searched1||vehicle->getID()==debug_searched2)) {
        DEBUG_OUT << "change:" << vehicle->getID() << ": " << vehicle->pos() << ", " << vehicle->speed() << endl;
    }
#endif
    pair<int, SUMOReal> changePreference = getChangePreference();
    SUMOReal currentLaneDist =
        vehicle->allowedContinuationsLength((*myCandi).lane, 0);
    if(changePreference.second==currentLaneDist) {
        changePreference.first = 0;
    }
    vehicle->getLaneChangeModel().prepareStep();
    // check whether the vehicle wants and is able to change to right lane
    std::pair<MSVehicle*, SUMOReal> rLead = getRealRightLeader();
    std::pair<MSVehicle*, SUMOReal> lLead = getRealLeftLeader();
    std::pair<MSVehicle*, SUMOReal> rFollow = getRealRightFollower();
    std::pair<MSVehicle*, SUMOReal> lFollow = getRealLeftFollower();
    std::pair<MSVehicle*, SUMOReal> leader = getRealThisLeader(myCandi);
    int state1 =
        change2right(leader, rLead, rFollow,
            changePreference.first, changePreference.second, currentLaneDist);
    bool changingAllowed =
        (state1&(LCA_BLOCKEDBY_LEADER|LCA_BLOCKEDBY_FOLLOWER))==0
        /*||
        (state1&LCA_OVERLAPPING)==0*//*&&currentLaneDist<100.0&&changePreference.second>100.0*/;
    if((state1&LCA_URGENT)!=0||(state1&LCA_SPEEDGAIN)!=0) {
        state1 |= LCA_RIGHT;
    }
    // change if the vehicle wants to and is allowed to change
    if ( (state1&LCA_RIGHT)!=0&&changingAllowed) {
        ( myCandi - 1 )->hoppedVeh = veh( myCandi );
        ( myCandi - 1 )->lane->myTmpVehicles.push_front( veh ( myCandi ) );
        vehicle->leaveLaneAtLaneChange();
        vehicle->enterLaneAtLaneChange( ( myCandi - 1 )->lane );
        vehicle->myLastLaneChangeOffset = 0;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (vehicle->getID()==debug_searched1||vehicle->getID()==debug_searched2)) {
        DEBUG_OUT << "changed2right" << endl;
    }
#endif
        vehicle->getLaneChangeModel().changed();
        return true;
    }
    if ( (state1&LCA_RIGHT)!=0&&(state1&LCA_URGENT)!=0) {
        if( myCandi->veh==myCandi->lane->myVehicles.rbegin()
            &&
            !vehicle->onAllowed()
            &&
            vehicle->speed()<MSGlobals::gMaxVehV4FalseLaneTeleport
            &&
            myCandi->lane->maxSpeed()>MSGlobals::gMinLaneVMax4FalseLaneTeleport
            &&
            currentLaneDist<MSGlobals::gMinVehDist4FalseLaneTeleport ) {

            WRITE_WARNING("Teleporting '" + veh(myCandi)->getID()+ "' due to failed lane changing.");
            MSVehicleTransfer::getInstance()->addVeh(veh(myCandi));
            return true;
        }
        ( myCandi - 1 )->lastBlocked = vehicle;
    }

    // check whether the vehicle wants and is able to change to left lane
    int state2 =
        change2left(leader, lLead, lFollow,
            changePreference.first, changePreference.second, currentLaneDist);
    if((state2&LCA_URGENT)!=0||(state2&LCA_SPEEDGAIN)!=0) {
        state2 |= LCA_LEFT;
    }
    changingAllowed =
        (state2&(LCA_BLOCKEDBY_LEADER|LCA_BLOCKEDBY_FOLLOWER))==0
        /*||
        (state2&LCA_OVERLAPPING)==0*//*&&currentLaneDist<100.0&&changePreference.second>100.0*/;
    vehicle->getLaneChangeModel().setState(state2|state1);
    // change if the vehicle wants to and is allowed to change
    if ( (state2&LCA_LEFT)!=0&&changingAllowed) {
        ( myCandi + 1 )->hoppedVeh = veh( myCandi );
        ( myCandi + 1 )->lane->myTmpVehicles.push_front( veh ( myCandi ) );
        vehicle->leaveLaneAtLaneChange();
        vehicle->enterLaneAtLaneChange( ( myCandi + 1 )->lane );
        vehicle->myLastLaneChangeOffset = 0;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime-5 && (vehicle->getID()==debug_searched1||vehicle->getID()==debug_searched2)) {
        DEBUG_OUT << "changed2left" << endl;
    }
#endif
        vehicle->getLaneChangeModel().changed();
        return true;
    }
    if ( (state2&LCA_LEFT)!=0&&(state2&LCA_URGENT)!=0) {
        if( myCandi->veh==myCandi->lane->myVehicles.rbegin()
            &&
            !vehicle->onAllowed()
            &&
            vehicle->speed()<MSGlobals::gMaxVehV4FalseLaneTeleport
            &&
            myCandi->lane->maxSpeed()>MSGlobals::gMinLaneVMax4FalseLaneTeleport
            &&
            currentLaneDist<MSGlobals::gMinVehDist4FalseLaneTeleport ) {

            WRITE_WARNING("Teleporting '" + veh(myCandi)->getID()+ "' due to failed lane changing.");
            MSVehicleTransfer::getInstance()->addVeh(veh(myCandi));
            return true;
        }
        ( myCandi + 1 )->lastBlocked = vehicle;
    }

    if((state1&(LCA_URGENT))!=0&&(state2&(LCA_URGENT))!=0) {
        // ... wants to go to the left AND to the right
        // just let them go to the right lane...
        state2 = 0;
        vehicle->getLaneChangeModel().setState(state1);
    }
    // check whether the vehicles should be swapped
    if((state1&(LCA_URGENT))!=0||(state2&(LCA_URGENT))!=0) {
        // get the direction ...
        ChangerIt target;
        if((state1&(LCA_URGENT))!=0) {
            // ... wants to go right
            target = myCandi - 1;
        }
        if((state2&(LCA_URGENT))!=0) {
            // ... wants to go left
            target = myCandi + 1;
        }
        MSVehicle *prohibitor = target->lead;//veh(target);//->follow;
        if(target->hoppedVeh!=0) {
            SUMOReal hoppedPos = target->hoppedVeh->pos();
            if( prohibitor==0||(
                hoppedPos>vehicle->pos() && prohibitor->pos()>hoppedPos) ) {

                prohibitor = 0;// !!! vehicles should not jump over more than one lanetarget->hoppedVeh;
            }
        }
        if( prohibitor!=0
            &&
            ((prohibitor->getLaneChangeModel().getState()&(LCA_URGENT/*|LCA_SPEEDGAIN*/))!=0
             &&
             (prohibitor->getLaneChangeModel().getState()&(LCA_LEFT|LCA_RIGHT))
                !=
                (vehicle->getLaneChangeModel().getState()&(LCA_LEFT|LCA_RIGHT))
            )
          ) {

            // check for position and speed
            if( /*prohibitor->speed()<0.1&&vehicle->speed()<0.1
                &&
                fabs(prohibitor->pos()-vehicle->pos())<0.1
                &&*/
                prohibitor->length()-vehicle->length()==0
                ) {

                // ok, may be swapped
                    // remove vehicle to swap with
				MSLane::VehCont::iterator i =
					find(
						target->lane->myTmpVehicles.begin(),
						target->lane->myTmpVehicles.end(),
						prohibitor);
				if(i!=target->lane->myTmpVehicles.end()) {
	                MSVehicle *bla = *i;
		            assert(bla==prohibitor);
	                target->lane->myTmpVehicles.erase(i);
                    // set this vehicle
                target->hoppedVeh = vehicle;
                target->lane->myTmpVehicles.push_front( vehicle );
                myCandi->hoppedVeh = prohibitor;
                myCandi->lane->myTmpVehicles.push_front( prohibitor );

                // leave lane and detectors
                vehicle->leaveLaneAtLaneChange();
                prohibitor->leaveLaneAtLaneChange();
                // patch position and speed
                SUMOReal p1 = vehicle->pos();
                vehicle->myState.myPos = prohibitor->myState.myPos;
                prohibitor->myState.myPos = p1;
                p1 = vehicle->speed();
                vehicle->myState.mySpeed = prohibitor->myState.mySpeed;
                prohibitor->myState.mySpeed = p1;
                // enter lane and detectors
                vehicle->enterLaneAtLaneChange( target->lane );
                prohibitor->enterLaneAtLaneChange( myCandi->lane );
                // mark lane change
                vehicle->getLaneChangeModel().changed();
                vehicle->myLastLaneChangeOffset = 0;
                prohibitor->getLaneChangeModel().changed();
                prohibitor->myLastLaneChangeOffset = 0;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime-5 && (vehicle->getID()==debug_searched1||vehicle->getID()==debug_searched2)) {
        DEBUG_OUT << "swapped:"
            << vehicle->getID() << ": at" << vehicle->getLane().getID() << ", " << vehicle->pos() << ", " << vehicle->speed()
            << " with:"
            << prohibitor->getID() << ": at" << prohibitor->getLane().getID() << ", " << prohibitor->pos() << ", " << vehicle->speed()
            << endl;
    }
#endif
                return true;
				}
            }
        }
    }
    // Candidate didn't change lane.
    myCandi->lane->myTmpVehicles.push_front( veh ( myCandi ) );
    vehicle->myLastLaneChangeOffset++;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime-5 && (vehicle->getID()==debug_searched1||vehicle->getID()==debug_searched2)) {
        DEBUG_OUT << "kept" << endl;
    }
#endif
    return false;
}

//-------------------------------------------------------------------------//
std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealThisLeader(const ChangerIt &target)
{
    // get the leading vehicle on the lane to change to
    MSVehicle* leader = target->lead;
    if( leader==0 ) {
        MSLane* targetLane = target->lane;
        MSLinkCont::iterator link =
            targetLane->succLinkSec( *veh(myCandi), 1, *targetLane );
        if(targetLane->isLinkEnd(link)) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        MSLane *nextLane = (*link)->getLane();
        if(nextLane==0) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        leader = nextLane->getLastVehicle(*this);
        if(leader==0) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        SUMOReal gap =
            leader->pos()-leader->length()
            +
            (myCandi->lane->length()-veh(myCandi)->pos());
        return std::pair<MSVehicle *, SUMOReal>(leader, MAX2((SUMOReal) 0, gap));
    } else {
        MSVehicle *candi = veh(myCandi);
        SUMOReal gap = leader->pos()-leader->length()-candi->pos();
        return std::pair<MSVehicle *, SUMOReal>(leader, MAX2((SUMOReal) 0, gap));
    }
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealLeader(const ChangerIt &target)
{
    // get the leading vehicle on the lane to change to
    MSVehicle* neighLead = target->lead;
    // check whether the hopped vehicle got the leader
    if(target->hoppedVeh!=0) {
        SUMOReal hoppedPos = target->hoppedVeh->pos();
        if( hoppedPos>veh(myCandi)->pos() &&
            (neighLead==0 || neighLead->pos()>hoppedPos) ) {

            neighLead = target->hoppedVeh;
        }
    }

    if( neighLead==0 ) {
        MSLane* targetLane = target->lane;
        MSLinkCont::iterator link =
            targetLane->succLinkSec( *veh(myCandi), 1, *targetLane );
        if(targetLane->isLinkEnd(link)) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        MSLane *nextLane = (*link)->getLane();
        if(nextLane==0) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        neighLead = nextLane->getLastVehicle(*this);
        if(neighLead==0) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        return std::pair<MSVehicle *, SUMOReal>(neighLead,
            neighLead->pos()-neighLead->length()
            +
            (myCandi->lane->length()-veh(myCandi)->pos()));
    } else {
        MSVehicle *candi = veh(myCandi);
        return std::pair<MSVehicle *, SUMOReal>(neighLead,
            neighLead->pos()-neighLead->length()-candi->pos());
    }
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealRightLeader()
{
    // there is no right lane
    if ( myCandi == myChanger.begin() ) {
        return std::pair<MSVehicle *, SUMOReal>(0, -1);
    }
    ChangerIt target = myCandi - 1;
    return getRealLeader(target);
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealLeftLeader()
{
    // there is no left lane
    if ( (myCandi+1) == myChanger.end() ) {
        return std::pair<MSVehicle *, SUMOReal>(0, -1);
    }
    ChangerIt target = myCandi + 1;
    return getRealLeader(target);
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealFollower(const ChangerIt &target)
{
    MSVehicle* neighFollow = veh(target);//->follow;
    // check whether the hopped vehicle got the follower
    if(target->hoppedVeh!=0) {
        SUMOReal hoppedPos = target->hoppedVeh->pos();
        if( hoppedPos<=veh(myCandi)->pos() &&
            (neighFollow==0 || neighFollow->pos()>hoppedPos) ) {

            neighFollow = target->hoppedVeh;
        }
    }
    if(neighFollow==0) {
        neighFollow = target->lane->myApproaching;
        if(neighFollow==0) {
            return std::pair<MSVehicle *, SUMOReal>(0, -1);
        }
        MSVehicle *candi = veh(myCandi);
        SUMOReal gap = candi->pos()-candi->length()
            + target->lane->myBackDistance;
        return std::pair<MSVehicle *, SUMOReal>(neighFollow, gap);
            //(neighFollow->getLane().length()-neighFollow->pos()));
    } else {
        MSVehicle *candi = veh(myCandi);
        return std::pair<MSVehicle *, SUMOReal>(neighFollow,
            candi->pos()-candi->length()-neighFollow->pos());
    }
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealRightFollower()
{
    // there is no right lane
    if ( myCandi == myChanger.begin() ) {
        return std::pair<MSVehicle *, SUMOReal>(0, -1);
    }
    ChangerIt target = myCandi - 1;
    return getRealFollower(target);
}


std::pair<MSVehicle *, SUMOReal>
MSLaneChanger::getRealLeftFollower()
{
    // there is no left lane
    if ( (myCandi+1) == myChanger.end() ) {
        return std::pair<MSVehicle *, SUMOReal>(0, -1);
    }
    ChangerIt target = myCandi + 1;
    return getRealFollower(target);
}



void
MSLaneChanger::updateChanger(bool vehHasChanged)
{
    assert(  myCandi->veh != myCandi->lane->myVehicles.rend() );

    // "Push" the vehicles to the back, i.e. follower becomes vehicle,
    // vehicle becomes leader, and leader becomes predecessor of vehicle,
    // if it exists.
    if(!vehHasChanged) {
        myCandi->lead = veh( myCandi );
    }
    myCandi->veh    = myCandi->veh + 1;

    if ( veh( myCandi ) == 0 ) {
        assert( myCandi->follow == 0 );
        // leader already 0.
        return;
    }
    if ( myCandi->veh + 1 == myCandi->lane->myVehicles.rend() ) {
        myCandi->follow = 0;
    }
    else {
        myCandi->follow = *( myCandi->veh + 1 ) ;
    }
    return;
}

//-------------------------------------------------------------------------//

void
MSLaneChanger::updateLanes()
{

    // Update the lane's vehicle-container.
    // First: it is bad style to change other classes members, but for
    // this release, other attempts were too time-consuming. In a next
    // release we will change from this lane-centered design to a vehicle-
    // centered. This will solve many problems.
    // Second: this swap would be faster if vehicle-containers would have
    // been pointers, but then I had to change too much of the MSLane code.
    for ( ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce ) {

        ce->lane->swapAfterLaneChange();
    }
}

//-------------------------------------------------------------------------//

MSLaneChanger::ChangerIt
MSLaneChanger::findCandidate()
{
    // Find the vehicle in myChanger with the smallest position. If there
    // is no vehicle in myChanger (shouldn't happen) , return
    // myChanger.end().
    ChangerIt max = myChanger.end();
    for ( ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce ) {
        if ( veh( ce ) == 0 ) {
            continue;
        }
        if ( max == myChanger.end() ) {
            max = ce;
            continue;
        }
        assert( veh( ce )  != 0 );
        assert( veh( max ) != 0 );
        if ( veh( max )->pos() < veh( ce )->pos() ) {
            max = ce;
        }
    }
    assert( max != myChanger.end() );
    assert( veh( max ) != 0 );
    return max;
}

//-------------------------------------------------------------------------//

int
MSLaneChanger::change2right(const std::pair<MSVehicle*, SUMOReal> &leader,
                            const std::pair<MSVehicle*, SUMOReal> &rLead,
                            const std::pair<MSVehicle*, SUMOReal> &rFollow,
                            int bestLaneOffset, SUMOReal bestDist,
                            SUMOReal currentDist)
{
    // Try to change to the right-lane if there is one. If this lane isn't
    // an allowed one, cancel the try. Otherwise, check some conditions. If
    // they are simultaniously fulfilled, a change is possible.

    // no right lane -> exit
    if ( myCandi == myChanger.begin() ) {
        return 0;
    }
    ChangerIt target = myCandi - 1;
    int blocked = overlapWithHopped( target )
        ? target->hoppedVeh->pos()<veh(myCandi)->pos()
            ? LCA_BLOCKEDBY_FOLLOWER
            : LCA_BLOCKEDBY_LEADER
        : 0;
    setOverlap(rLead, rFollow, /*target, */blocked);
    setIsSafeChange(rLead, rFollow, target, blocked);
    return blocked
        |
        advan2right(leader, rLead, rFollow,
            blocked, bestLaneOffset, bestDist, currentDist);
}

//-------------------------------------------------------------------------//

int
MSLaneChanger::change2left(const std::pair<MSVehicle*, SUMOReal> &leader,
                           const std::pair<MSVehicle*, SUMOReal> &rLead,
                           const std::pair<MSVehicle*, SUMOReal> &rFollow,
                           int bestLaneOffset, SUMOReal bestDist,
                           SUMOReal currentDist)
{
    // Try to change to the left-lane, if there is one. If this lane isn't
    // an allowed one, cancel the try. Otherwise, check some conditions.
    // If they are simultaniously fulfilled, a change is possible.

    // no left lane, overlapping or left lane not allowed -> exit
    ChangerIt target = myCandi + 1;
    if ( target == myChanger.end() ) {
        return 0;
    }
    int blocked = overlapWithHopped( target )
        ? target->hoppedVeh->pos()<veh(myCandi)->pos()
            ? LCA_BLOCKEDBY_FOLLOWER
            : LCA_BLOCKEDBY_LEADER
        : 0;
    setOverlap(rLead, rFollow, /*target, */blocked );
    setIsSafeChange(rLead, rFollow, target, blocked);
    return blocked
        |
        advan2left(leader, rLead, rFollow,
            blocked, bestLaneOffset, bestDist, currentDist);
}

//-------------------------------------------------------------------------//

MSLaneChanger::ChangerIt
MSLaneChanger::findTarget()
{
    // If candidate is on a not allowed lane, it has to search for a
    // neighboured lane that will bring it closer to an allowed one.
    // Certainly there is a more elegant way than looping over all lanes.
    ChangerIt nearestTarget = myChanger.end();
    unsigned int minDist    = myChanger.size();

    for ( ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce ) {

        if ( ce == myCandi ) {

            continue;
        }
        if ( candiOnAllowed( ce ) ) {

            unsigned int dist = abs( distance( myCandi, ce ) );
            assert( dist > 0 );
            assert( dist < myChanger.size() );
            if ( dist < minDist ) {

                minDist       = dist;
                nearestTarget = ce;
            }
        }
    }
    assert( nearestTarget != myChanger.end() );

    MSVehicle *vehicle = veh(myCandi);
    if ( distance( myCandi, nearestTarget ) > 0 ) {

        assert( myCandi + 1 != myChanger.end() );
        return myCandi + 1;
    }
    assert( myCandi != myChanger.begin() );
    return myCandi - 1;
}

//-------------------------------------------------------------------------//

void
MSLaneChanger::setOverlap(const std::pair<MSVehicle*, SUMOReal> &rLead,
                          const std::pair<MSVehicle*, SUMOReal> &rFollow,
                          /*const ChangerIt &target, */int &blocked)
{
    // check the follower only if not already known that...
    if((blocked&LCA_BLOCKEDBY_FOLLOWER)==0) {
        if(rFollow.first!=0&&rFollow.second<0) {
            blocked |= (LCA_BLOCKEDBY_FOLLOWER|LCA_OVERLAPPING);
        }
    }
    // check the leader only if not already known that...
    if((blocked&LCA_BLOCKEDBY_LEADER)==0) {
        if(rLead.first!=0&&rLead.second<0) {
            blocked |= (LCA_BLOCKEDBY_LEADER|LCA_OVERLAPPING);
        }
    }
}

//-------------------------------------------------------------------------//

void
MSLaneChanger::setIsSafeChange(const std::pair<MSVehicle*, SUMOReal> &neighLead,
                               const std::pair<MSVehicle*, SUMOReal> &neighFollow,
                               const ChangerIt &target, int &blocked)
{
    // Check if candidate's change to target-lane will be safe, i.e. is there
    // enough back-gap to the neighFollow to drive collision-free (if there is
    // no neighFollow, keep a safe-gap to the beginning of the lane) and is
    // there enough gap for the candidate to neighLead to drive collision-
    // free (if there is no neighLead, be sure that candidate is able to slow-
    // down towards the lane end).
    MSVehicle* vehicle     = veh( myCandi );

    // check back gap
    if((blocked&LCA_BLOCKEDBY_FOLLOWER)==0) {
        if(neighFollow.first!=0) {
            MSLane* targetLane = target->lane;
            // !!! eigentlich: vsafe braucht die Max. Geschwindigkeit beider Spuren
            if(!neighFollow.first->hasSafeGap(neighFollow.first->speed(), neighFollow.second, vehicle->speed(), targetLane->maxSpeed())) {
//                neighFollow.second, *vehicle, targetLane)) {
                blocked |= LCA_BLOCKEDBY_FOLLOWER;
            }
        }
    }

    // check front gap
    if((blocked&LCA_BLOCKEDBY_LEADER)==0) {
        if(neighLead.first!=0) {
            MSLane* targetLane = target->lane;
            // !!! eigentlich: vsafe braucht die Max. Geschwindigkeit beider Spuren
            if(!vehicle->hasSafeGap(vehicle->speed(), neighLead.second, neighLead.first->speed(), targetLane->maxSpeed())) {
//                neighLead.second, *neighLead.first, targetLane)) {
                blocked |= LCA_BLOCKEDBY_LEADER;
            }
        }
    }
}

//-------------------------------------------------------------------------//

int
MSLaneChanger::advan2right(const std::pair<MSVehicle*, SUMOReal> &leader,
                           const std::pair<MSVehicle*, SUMOReal> &neighLead,
                           const std::pair<MSVehicle*, SUMOReal> &neighFollow,
                           int blocked,
                           int bestLaneOffset, SUMOReal bestDist,
                           SUMOReal currentDist)
{
    MSAbstractLaneChangeModel::MSLCMessager
        msg(leader.first, neighLead.first, neighFollow.first);
    return veh(myCandi)->getLaneChangeModel().wantsChangeToRight(
        msg, blocked,
        leader, neighLead, neighFollow, *(myCandi-1)->lane,
        bestLaneOffset, bestDist,
        veh(myCandi)->allowedContinuationsLength((myCandi-1)->lane, 0),
        currentDist,
        &(myCandi->lastBlocked));
}

//-------------------------------------------------------------------------//

int
MSLaneChanger::advan2left(const std::pair<MSVehicle*, SUMOReal> &leader,
                          const std::pair<MSVehicle*, SUMOReal> &neighLead,
                          const std::pair<MSVehicle*, SUMOReal> &neighFollow,
                          int blocked,
                          int bestLaneOffset, SUMOReal bestDist,
                          SUMOReal currentDist)
{
    MSAbstractLaneChangeModel::MSLCMessager
        msg(leader.first, neighLead.first, neighFollow.first);
    return veh(myCandi)->getLaneChangeModel().wantsChangeToLeft(
        msg, blocked,
        leader, neighLead, neighFollow, *(myCandi+1)->lane,
        bestLaneOffset, bestDist,
        veh(myCandi)->allowedContinuationsLength((myCandi+1)->lane, 0),
        currentDist,
        &(myCandi->lastBlocked));
}


std::pair<int, SUMOReal>
MSLaneChanger::getChangePreference()
{
    MSVehicle *vehicle = veh( myCandi );
    // compute the need to change lanes soon
    int maxNo = -1;
    int maxIdx = -1;
    int midx = distance(myChanger.begin(), myCandi);
    for(int i=0; i<(int) myChanger.size(); ++i) {
        MSLane *lane = (*(myChanger.begin()+i)).lane;
        int mmax = vehicle->countAllowedContinuations(lane, -(midx-i));
        if(maxNo==-1||mmax>maxNo) {
            maxNo = mmax;
            maxIdx = i;
        }
    }
    assert(maxIdx!=-1);
    return pair<int, SUMOReal>(
        maxIdx-distance(myChanger.begin(), myCandi),
        vehicle->allowedContinuationsLength((*(myChanger.begin()+maxIdx)).lane, -(midx-maxIdx))
        );
}


//-------------------------------------------------------------------------//


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
