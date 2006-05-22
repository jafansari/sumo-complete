/***************************************************************************
                          MSVehicle.cpp  -  Base for all
                          micro-simulation Vehicles.
                             -------------------
    begin                : Mon, 05 Mar 2001
    copyright            : (C) 2001 by ZAIK http://www.zaik.uni-koeln.de/AFS
    author               : Christian Roessel
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
// Revision 1.80  2006/05/22 13:27:15  dkrajzew
// debugged phone device usage
//
// Revision 1.79  2006/05/15 05:50:40  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.79  2006/05/08 11:09:36  dkrajzew
// began with the extraction of the car-following-model from MSVehicle
//
// Revision 1.78  2006/04/18 08:13:52  dkrajzew
// debugging rerouting
//
// Revision 1.77  2006/04/07 05:28:07  dkrajzew
// debugging
//
// Revision 1.76  2006/04/05 05:27:34  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.75  2006/03/17 09:01:12  dkrajzew
// .icc-files removed
//
// Revision 1.74  2006/02/27 12:08:14  dkrajzew
// raknet-support added
//
// Revision 1.73  2006/02/23 11:31:09  dkrajzew
// TO SS2 output added
//
// Revision 1.72  2006/01/26 08:30:29  dkrajzew
// patched MSEdge in order to work with a generic router
//
// Revision 1.71  2006/01/23 10:25:29  dkrajzew
// post-release changes
//
// Revision 1.70  2006/01/09 11:53:00  dkrajzew
// bus stops implemented
//
// Revision 1.69  2005/12/01 07:37:35  dkrajzew
// introducing bus stops: eased building vehicles; vehicles may now have nested elements
//
// Revision 1.68  2005/11/09 06:39:38  dkrajzew
// usage of internal lanes is now optional at building
//
// Revision 1.67  2005/10/10 11:58:14  dkrajzew
// debugging
//
// Revision 1.66  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.65  2005/09/23 13:16:40  dkrajzew
// debugging the building process
//
// Revision 1.64  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.63  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.62  2005/07/12 12:06:11  dkrajzew
// first devices (mobile phones) added
//
// Revision 1.61  2005/05/04 08:34:20  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; new mead data functionality; lane-changing offset computation debugged; simulation speed-up by avoiding multiplication with 1
//
// Revision 1.60  2005/02/17 10:33:38  dkrajzew
// code beautifying;
// Linux building patched;
// warnings removed;
// new configuration usage within guisim
//
// Revision 1.59  2005/02/01 10:10:42  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.58  2004/12/16 12:24:45  dkrajzew
// debugging
//
// Revision 1.57  2004/11/23 10:20:10  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.56  2004/08/02 12:40:55  dkrajzew
// debugging; refactoring; lane-changing API
//
// Revision 1.55  2004/07/02 09:57:37  dkrajzew
// handling of routes added
//
// Revision 1.54  2004/04/02 11:36:28  dkrajzew
// "compute or not"-structure added;
//  added two further simulation-wide output
//  (emission-stats and single vehicle trip-infos)
//
// Revision 1.53  2004/04/01 16:39:03  roessel
// Bug fix: the value of the first parameter in the call to
// updateMeanData in the methods enterLaneAtEmit and
// enterLaneAtLaneChange could have been unreasonably high
// (unsigned(0)-1) because of wrong placement of parentheses. The example
// in data/examples/cross3ltl_meandata/ does perform without assertion
// now.
//
// Revision 1.52  2004/03/19 13:09:40  dkrajzew
// debugging
//
// Revision 1.51  2004/02/18 05:30:09  dkrajzew
// removal of all moveReminders on lane change added
//
// Revision 1.50  2004/02/16 15:21:58  dkrajzew
// movedDistance-retrival reworked; forgetting predecessors when driving over
//  more than one lane patched
//
// Revision 1.49  2004/02/05 16:37:51  dkrajzew
// e3-debugging: only e3-detectors have to remove killed vehicles; storage
//  for detectors to be informed added
//
// Revision 1.48  2004/01/26 15:55:55  dkrajzew
// the vehicle is now informed about being emitted (as we want to display
//  the information about the real departure time witin the gui - within
//  microsim, this information may be used for some other stuff)
//
// Revision 1.47  2004/01/26 07:51:44  dkrajzew
// the vehicle leaves his move reminders when leaving the simulation, now
//  (is still false)
//
// Revision 1.46  2004/01/12 15:03:40  dkrajzew
// removed some unneeded debug-variables
//
// Revision 1.45  2003/12/12 12:37:42  dkrajzew
// proper usage of lane states applied; scheduling of vehicles into the
//  beamer on push failures added
//
// Revision 1.44  2003/12/11 06:31:45  dkrajzew
// implemented MSVehicleControl as the instance responsible for vehicles
//
// Revision 1.43  2003/12/04 13:30:41  dkrajzew
// work on internal lanes
//
// Revision 1.42  2003/11/24 10:22:56  dkrajzew
// patched the false usage of oldLaneMoveReminders when more than one street
//  is within
//
// Revision 1.41  2003/11/20 14:59:17  dkrajzew
// detector usage patched
//
// Revision 1.40  2003/11/12 13:50:30  dkrajzew
// MSLink-members are now secured from the outer world
//
// Revision 1.39  2003/11/11 08:36:21  dkrajzew
// removed some debug-variables
//
// Revision 1.38  2003/10/31 08:04:18  dkrajzew
// an invalid assertion removed
//
// Revision 1.37  2003/10/28 08:37:01  dkrajzew
// retelportation avoidance added
//
// Revision 1.36  2003/10/24 16:48:37  roessel
// Added new method getMovedDistance and corresponding member.
//
// Revision 1.35  2003/10/22 07:06:04  dkrajzew
// patching of lane states on force vehicle removal added
//
// Revision 1.34  2003/10/20 07:59:43  dkrajzew
// grid lock dissolving by vehicle teleportation added
//
// Revision 1.33  2003/10/17 06:52:01  dkrajzew
// acceleration is now time-dependent
//
// Revision 1.32  2003/10/16 08:33:49  dkrajzew
// new lane changing rules implemented
//
// Revision 1.31  2003/10/15 11:43:50  dkrajzew
// false lane-changing rules removed; an (far too large information interface
//  between vehicle and lane-changer implemented
//
// Revision 1.30  2003/09/22 12:35:03  dkrajzew
// vehicle does not decelerate on yellow when halting is not possible
//
// Revision 1.29  2003/09/05 15:14:42  dkrajzew
// first steps for reading of internal lanes
//
// Revision 1.28  2003/08/20 11:44:11  dkrajzew
// min and max-functions moved to an own definition file
//
// Revision 1.27  2003/08/04 11:47:05  dkrajzew
// vehicle collision on yellow tl patched
//
// Revision 1.26  2003/07/18 12:35:04  dkrajzew
// removed some warnings
//
// Revision 1.25  2003/06/19 10:58:34  dkrajzew
// too conservative computation of the braking gap patched
//
// Revision 1.24  2003/06/18 11:30:26  dkrajzew
// debug outputs now use a DEBUG_OUT macro instead of cout; this shall ease
//  the search for further couts which must be redirected to the messaging
//  subsystem
//
// Revision 1.23  2003/06/05 10:19:44  roessel
// Added previous lane reminder-container and workOnMoveReminders().
//
// Revision 1.22  2003/05/25 16:15:10  roessel
// Rewrite of workOnMoveReminders and activateRemindersByEmitOrLaneChange.
//
// Revision 1.21  2003/05/22 12:48:12  roessel
// New method activateRemindersByEmitOrLaneChange. Exchanged for-loop by new
// method.
//
// Revision 1.20  2003/05/21 16:20:45  dkrajzew
// further work detectors
//
// Revision 1.19  2003/05/21 15:15:42  dkrajzew
// yellow lights implemented (vehicle movements debugged
//
// Revision 1.18  2003/05/20 09:31:46  dkrajzew
// emission debugged; movement model reimplemented (seems ok); detector output
// debugged; setting and retrieval of some parameter added
//
// Revision 1.17  2003/04/16 10:05:06  dkrajzew
// uah, debugging
//
// Revision 1.16  2003/04/14 08:33:02  dkrajzew
// some further bugs removed
//
// Revision 1.15  2003/04/10 15:43:43  dkrajzew
// emission on non-source lanes debugged
//
// Revision 1.14  2003/04/09 15:32:29  dkrajzew
// periodical vehicles must have a period over zero now to be reasserted
//
// Revision 1.13  2003/04/07 12:12:38  dkrajzew
// eps reduced for small segments (dawdle2)
//
// Revision 1.12  2003/03/20 17:31:42  dkrajzew
// StringUtils moved from utils/importio to utils/common
//
// Revision 1.11  2003/03/20 16:21:12  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.10  2003/03/12 16:50:56  dkrajzew
// lane retrival added for centering a vehicle on the display
//
// Revision 1.9  2003/03/03 14:56:26  dkrajzew
// some debugging; new detector types added; actuated traffic lights added
//
// Revision 1.8  2003/02/07 10:41:50  dkrajzew
// updated
//
// Revision 1.7  2002/10/29 10:42:51  dkrajzew
// problems accured due to the deletion of a vehicle that reached his
// destination debugged
//
// Revision 1.6  2002/10/28 12:59:38  dkrajzew
// vehicles are now deleted whe the tour is over
//
// Revision 1.5  2002/10/21 09:55:40  dkrajzew
// begin of the implementation of multireferenced, dynamically loadable routes
//
// Revision 1.4  2002/10/17 06:11:48  dkrajzew
// forgot setting of drive request when regarding a critical non-first vehicle
// added
//
// Revision 1.3  2002/10/16 17:33:10  dkrajzew
// error in moveCritical yielding in collisions removed
//
// Revision 1.2  2002/10/16 16:45:41  dkrajzew
// debugged
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.21  2002/09/25 17:14:42  roessel
// MeanData calculation and output implemented.
//
// Revision 1.20  2002/07/31 17:33:01  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.23  2002/07/31 14:42:34  croessel
// Use of new VehicleType methods.
//
// Revision 1.22  2002/07/24 16:33:49  croessel
// New methods isInsertTimeHeadWayCond() and isInsertBrakeCond() to check
// the Krauss conditions during emit/laneChange.
//
// Revision 1.21  2002/07/16 17:50:36  croessel
// Removed debug code.
//
// Revision 1.20  2002/07/03 15:57:11  croessel
// Redundant code removal.
// New methods isSafeChange, hasSafeGap, safeEmitGap instead of safeGap.
// Removed safeLaneChangeGap, not used.
// New method vNeighEqualPos for "don't overtake on the right".
// move() respects now neighbours taht shouldn't be overtaken.
//
// Revision 1.19  2002/06/25 10:53:36  croessel
// Little mistake in destructor.
//
// Revision 1.18  2002/06/25 10:41:17  croessel
// safeGap using fabs is not the answer to collisions. Removing it.
//
// Revision 1.17  2002/06/21 10:59:09  dkrajzew
// inclusion of .cpp-files in .cpp files removed
//
// Revision 1.16  2002/06/20 13:44:58  dkrajzew
// safeGap is now using fabs
//
// Revision 1.15  2002/06/20 08:00:59  dkrajzew
// template and .cpp inclusion inserted due to problems with MSVC++; should
// be revalidated and removed as soon as possible
//
// Revision 1.14  2002/06/19 15:09:12  croessel
// Changed *Gap methods to check for timeheadway < deltaT states.
//
// Revision 1.13  2002/06/11 19:38:22  croessel
// Bugfix: in safeGap(), vDecel should be max(...), not
// min(...). Otherwise gap is always < 0 and LaneChanger will almost
// always allow a change. This may lead to collisions in the next
// timesteps.
//
// Revision 1.12  2002/06/06 07:21:10  croessel
// Changed inclusion from .iC to .icc
//
// Revision 1.11  2002/05/29 17:06:03  croessel
// Inlined some methods. See the .icc files.
//
// Revision 1.10  2002/05/17 13:23:36  croessel
// Changed novehicles to MSNet::noVehicles
//
// Revision 1.9  2002/05/17 13:04:24  croessel
// Added _SPEEDCHECK code in all move-methos.
// Added upper bound (= vaacel) for vSafe in nextState().
// nextStateCompete():
// - Removed vMax and vLaneMax, they are already considered in vaccel().
// - Calculate nextPos with vNext instead of vSafe.
// New method laneChangeBrake2much() added.
//
// Revision 1.8  2002/05/08 13:24:21  croessel
// safeGap(): vDecel shouldn't be less than 0.
//
// Revision 1.7  2002/05/08 11:36:26  croessel
// destReached() changed to allow routes to include the destination-edge
// several times before reaching the destination.
//
// Revision 1.6  2002/04/24 15:22:08  croessel
// Bugfix: changed myType->decel() into myType->accel() in dadwle().
//
// Revision 1.5  2002/04/18 15:05:18  croessel
// In nextStateCompete(): Returns now correct position instead of 0.
//
// Revision 1.4  2002/04/18 14:30:24  croessel
// Bug in Revision 1.3 changes. Return State( pos, speed = 0 ) instead of
// myState.
//
// Revision 1.3  2002/04/17 10:58:24  croessel
// Introduced dontMoveGap to handle SUMORealing-point-inaccuracy. Vehicles
// will keep their state if gap2pred is smaller.
//
// Revision 1.2  2002/04/11 15:25:56  croessel
// Changed SUMOReal to SUMOReal.
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.4  2002/03/20 16:02:23  croessel
// Initialisation and update of allowedLanes-container fixed.
//
// Revision 2.3  2002/03/13 17:44:38  croessel
// Assert removed because it causes not intended crashes for
// YIELD_ON_SUCC.
//
// Revision 2.2  2002/03/13 17:42:19  croessel
// In nextState() we need to dawdle _before_ we set the new state.
//
// Revision 2.1  2002/03/13 16:56:35  croessel
// Changed the simpleOutput to XMLOutput by introducing nested classes
// XMLOut. Output is now indented.
//
// Revision 2.0  2002/02/14 14:43:19  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.14  2002/02/05 13:51:53  croessel
// GPL-Notice included.
// In *.cpp files also config.h included.
//
// Revision 1.13  2002/02/05 11:52:11  croessel
// Changed method vAccel() to accelState().
// Introduced new mezhods for lane-change:
// State::advantage()
// onAllowed()
// overlap()
// congested()
//
// Revision 1.12  2002/02/01 13:57:07  croessel
// Changed methods and members bmax/dmax/sigma to more meaningful names
// accel/decel/dawdle.
//
// Revision 1.11  2002/01/31 13:53:08  croessel
// Assertion in move() added.
//
// Revision 1.10  2002/01/24 18:29:43  croessel
// New and modified gap-methods: interactionDist renamed to brakeGap,
// interactionGap introduced and calculation of gaps fixed.
//
// Revision 1.9  2002/01/16 10:07:50  croessel
// Modified some speed calculations to the use of MSNet::deltaT() instead
// of myTau.
//
// Revision 1.8  2001/12/20 14:50:07  croessel
// new method: id() (moved from .h)
//
// Revision 1.7  2001/12/13 14:11:58  croessel
// Introduction of MSVehicle::State methods.
//
// Revision 1.6  2001/12/06 13:16:41  traffic
// speed computation added (use -D _SPEEDCHECK)
//
// Revision 1.5  2001/11/21 15:25:13  croessel
// Numerous changes during debug session.
//
// Revision 1.4  2001/11/15 17:17:36  croessel
// Outcommented the inclusion of the inline *.iC files. Currently not needed.
// Vehicle-State introduced. Simulation is now independant of vehicle's
// speed. Still, the position is fundamental, also a gap between
// vehicles. But that's it.
//
// Revision 1.3  2001/11/14 15:47:34  croessel
// Merged the diffs between the .C and .cpp versions. Numerous changes
// in MSLane, MSVehicle and MSJunction.
//
// Revision 1.2  2001/11/14 10:49:07  croessel
// CR-line-end removed.
//
// Revision 1.1  2001/10/24 07:17:13  traffic
// new extension
//
// Revision 1.6  2001/10/23 09:31:41  traffic
// parser bugs removed
//
// Revision 1.4  2001/09/06 15:35:50  croessel
// Added operator<< to class MSVehicle for simple text output and minor
// changes.
//
// Revision 1.3  2001/07/25 12:17:59  traffic
// CC problems with make_pair repaired
//
// Revision 1.2  2001/07/16 12:55:47  croessel
// Changed id type from unsigned int to string. Added string-pointer
// dictionaries and dictionary methods.
//
// Revision 1.1.1.1  2001/07/11 15:51:13  traffic
// new start
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

#include "MSLane.h"
#include "MSVehicle.h"
#include "MSEdge.h"
#include "MSVehicleType.h"
#include "MSNet.h"
#include "MSRoute.h"
#include "MSLinkCont.h"
#include "MSVehicleQuitReminded.h"
#include "MSDebugHelper.h"
#include <utils/common/StringUtils.h>
#include <utils/common/StdDefs.h>
#include <utils/gfx/RGBColor.h>
#include <microsim/MSVehicleControl.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "MSMoveReminder.h"
#include <utils/options/OptionsSubSys.h>
#include <utils/options/OptionsCont.h>
#include "lanechanging/MSLCM_Krauss.h"
#include "lanechanging/MSLCM_DK2004.h"
#include <utils/common/ToString.h>
#include <utils/common/FileHelpers.h>
#include <utils/bindevice/BinaryInputDevice.h>
#include "trigger/MSBusStop.h"


#include "devices/MSDevice_CPhone.h"

#ifdef ABS_DEBUG
#include "MSDebugHelper.h"
#endif

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


#define BUS_STOP_OFFSET 0.5


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * some definitions (debugging only)
 * ======================================================================= */
#define DEBUG_OUT cout


/* =========================================================================
 * static member variables
 * ======================================================================= */
MSVehicle::DictType MSVehicle::myDict;


/* =========================================================================
 * method definitions
 * ======================================================================= */
/* -------------------------------------------------------------------------
 * methods of MSVehicle::State
 * ----------------------------------------------------------------------- */
MSVehicle::State::State( const State& state )
{
    myPos = state.myPos;
    mySpeed = state.mySpeed;
}

/////////////////////////////////////////////////////////////////////////////

MSVehicle::State&
MSVehicle::State::operator=( const State& state )
{
    myPos   = state.myPos;
    mySpeed = state.mySpeed;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::State::operator!=( const State& state )
{
    return ( myPos   != state.myPos ||
             mySpeed != state.mySpeed );
}

/////////////////////////////////////////////////////////////////////////////

SUMOReal
MSVehicle::State::pos() const
{
    return myPos;
}

/////////////////////////////////////////////////////////////////////////////

MSVehicle::State::State( SUMOReal pos, SUMOReal speed ) :
    myPos( pos ), mySpeed( speed )
{
}


/* -------------------------------------------------------------------------
 * MSVehicle-methods
 * ----------------------------------------------------------------------- */
MSVehicle::~MSVehicle()
{
    // remove move reminder
    for(QuitRemindedVector::iterator i=myQuitReminded.begin(); i!=myQuitReminded.end(); ++i) {
        (*i)->removeOnTripEnd(this);
    }
    //myWaitingPersons.clear();
    myRoute->decReferenceCnt();
    if(!myRoute->inFurtherUse()) {
        MSRoute::erase(myRoute->getID());
    }
    // delete
        // prior routes
    if(myDoubleCORNMap.find(MSCORN::CORN_VEH_NUMBERROUTE)!=myDoubleCORNMap.end()) {
        int noReroutes = (int) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
        for(int i=0; i<noReroutes; ++i) {
	  delete (MSRoute*) myPointerCORNMap[(MSCORN::Pointer) (i+noReroutes)];
        }
    }
        // devices
    {
        // cell phones
        if(myDoubleCORNMap.find((MSCORN::Function) MSCORN::CORN_VEH_DEV_NO_CPHONE)!=myDoubleCORNMap.end()) {
            size_t no = (size_t) myDoubleCORNMap[(MSCORN::Function) MSCORN::CORN_VEH_DEV_NO_CPHONE];
            for(size_t np=0; np<no; np++) {
        	    delete ((MSDevice_CPhone*) myPointerCORNMap[(MSCORN::Pointer) (MSCORN::CORN_P_VEH_DEV_CPHONE+np)]);
            }
        }
    }
    delete myLaneChangeModel;
}

/////////////////////////////////////////////////////////////////////////////

MSVehicle::MSVehicle( string id,
                      MSRoute* route,
                      SUMOTime departTime,
                      const MSVehicleType* type,
                      size_t noMeanData,
                      int repNo, int repOffset) :
    MSModel(type),
#ifdef RAKNET_DEMO
    Vehicle(),
#endif
    myLastLaneChangeOffset(0),
    myTarget(0),
    myWaitingTime( 0 ),
    myRepetitionNumber(repNo),
    myPeriod(repOffset),
    myID(id),
    myRoute(route),
    myDesiredDepart(departTime),
    myState(0, 0),
    myLane( 0 ),
    myCurrEdge(0),
    myAllowedLanes(0),
    myMoveReminders( 0 ),
    myOldLaneMoveReminders( 0 ),
    myOldLaneMoveReminderOffsets( 0 )
{
    myCurrEdge = myRoute->begin();
    rebuildAllowedLanes();
    myLaneChangeModel = new MSLCM_DK2004(*this);
    // init cell phones
    initDevices();
}


void
MSVehicle::initDevices()
{
    // cell phones
    OptionsCont &oc = OptionsSubSys::getOptions();
    if(oc.getFloat("device.cell-phone.probability")!=0) {
        if((((SUMOReal) rand()/(SUMOReal) RAND_MAX))<=oc.getFloat("device.cell-phone.probability")) {
            int noCellPhones = (int) (((SUMOReal) rand()/(SUMOReal) RAND_MAX)
                * (oc.getFloat("device.cell-phone.amount.max") - oc.getFloat("device.cell-phone.amount.min"))
                + oc.getFloat("device.cell-phone.amount.min"));
            myDoubleCORNMap[(MSCORN::Function) MSCORN::CORN_VEH_DEV_NO_CPHONE] =
                (SUMOReal) noCellPhones;
            for(int np=0; np<noCellPhones; np++) {
    		    myPointerCORNMap[(MSCORN::Pointer) (MSCORN::CORN_P_VEH_DEV_CPHONE+np)] =
                    (void*) new MSDevice_CPhone(*this);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

const MSEdge &
MSVehicle::departEdge()
{
    return **myCurrEdge;
}

/////////////////////////////////////////////////////////////////////////////

const MSEdge::LaneCont &
MSVehicle::departLanes()
{
    return *(myAllowedLanes[0]);
}

/////////////////////////////////////////////////////////////////////////////

SUMOTime
MSVehicle::desiredDepart() const
{
    return myDesiredDepart;
}

/////////////////////////////////////////////////////////////////////////////

const MSEdge*
MSVehicle::succEdge(unsigned int nSuccs) const
{
    if ( hasSuccEdge( nSuccs ) ) {
        return *( myCurrEdge + nSuccs );
    } else {
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::hasSuccEdge(unsigned int nSuccs) const
{
    if (myCurrEdge + nSuccs >= myRoute->end()) {
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::destReached( const MSEdge* targetEdge )
{
    if(targetEdge->getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
        return false;
    }
    // search for the target in the vehicle's route. Usually there is
    // only one iteration. Only for very short edges a vehicle can
    // "jump" over one ore more edges in one timestep.
    MSRouteIterator edgeIt = myCurrEdge;
    while ( *edgeIt != targetEdge ) {
        ++edgeIt;
        assert( edgeIt != myRoute->end() );
    }
    myCurrEdge = edgeIt;
    // Check if destination-edge is reached. Update allowedLanes makes
    // only sense if destination isn't reached.
    MSRouteIterator destination = myRoute->end() - 1;
    if ( myCurrEdge == destination ) {
        return true;
    } else {
        rebuildAllowedLanes();
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::endsOn(const MSLane &lane) const
{
    return lane.inEdge(myRoute->getLastEdge());
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::move(MSLane* lane, const MSVehicle* pred, const MSVehicle* neigh)
{
    // reset move information
    myTarget = 0;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        DEBUG_OUT << "movea/1:" << debug_globaltime << ": " << myID << " at " << myLane->getID() << ": " << pos() << ", " << speed() << endl;
    }
#endif
    // compute gap to use
    SUMOReal gap = gap2pred(*pred);
    if(gap<0.1) {
        assert(gap>-0.1);
        gap = 0;
    }
    SUMOReal vAccel = vaccel( myState.mySpeed, lane->maxSpeed() );
    SUMOReal vSafe  =
        vsafe( myState.mySpeed, myType->decel(), gap, pred->speed() );
    if(!myStops.empty()) {
        if(myStops.begin()->reached) {
            if(myStops.begin()->duration==0) {
                if(myStops.begin()->busstop!=0) {
                    myStops.begin()->busstop->leaveFrom(this);
                }
                myStops.pop_front();
            } else {
                myStops.begin()->duration--;
                myTarget = myLane;
                myState.mySpeed = 0;
                myLane->addMean2(0, length());
                return; // !!!detectore etc?
            }
        } else {
            if(myStops.begin()->lane==myLane) {
                Stop &bstop = *myStops.begin();
                SUMOReal endPos = bstop.pos;
                bool busStopsMustHaveSpace = true;
                if(bstop.busstop!=0) {
                    endPos = bstop.busstop->getLastFreePos();
                    if(endPos-5.<bstop.busstop->getBeginLanePosition()) { // !!! explicite offset
                        busStopsMustHaveSpace = false;
                    }
                }
                if(myState.pos()>=endPos-BUS_STOP_OFFSET&&busStopsMustHaveSpace) {
                    bstop.reached = true;
                    if(bstop.busstop!=0) {
                        bstop.busstop->enter(this, myState.pos(), myState.pos()-length());
                    }
                }

                vSafe = MIN2(vSafe, vsafe( myState.mySpeed, myType->decel(), endPos-myState.pos(), 0 ) );
            }
        }
    }



    SUMOReal vNext;

    // !!! non - Krau� brake when urgent lane changing failed
/*    if( !myLane->appropriate(this) &&
        (_lcAction&LCA_LANEBEGIN)==0 ) {
        vNext = MIN2(vSafe, myState.mySpeed-myType->decelSpeed()/2.0); // !!! full deceleration causes floating point problems
    }*/
    // !!! non - Krau� brake when urgent lane changing failed
//    else {
            // lane-change interaction
    ///* !!!!!
      //      */
    /*
        if ( neigh != 0 ) {

            vNext = dawdle( MIN3( vAccel, vSafe, vNeighEqualPos( *neigh ) ) );
        }
        else {
        */
            vNext = dawdle( MIN2( vAccel, vSafe ) );
        //}
    vNext =
        myLaneChangeModel->patchSpeed(
            MAX2((SUMOReal) 0, ACCEL2SPEED(myState.mySpeed-myType->decel())),
            vNext,
            MIN2(vSafe, vaccel(myState.mySpeed, myLane->maxSpeed())),
            vSafe);
    vNext = MIN3(vNext, vSafe, vaccel(myState.mySpeed, myLane->maxSpeed()));
//    }
    // check needed for the Krauss-model
    /*!!!
    SUMOReal accelSpace = accelDist()*MSNet::deltaT();
    if( gap<accelSpace &&
        //pred->speed()<accelSpace &&
        myState.mySpeed<vaccel(myLane) ) {

        vNext = DIST2SPEED(gap);
    }
*/
    SUMOReal predDec = MAX2((SUMOReal) 0, pred->speed()-decelAbility() /* !!! decelAbility of leader! */);
    if(brakeGap(vNext)+vNext*myTau > brakeGap(predDec) + gap) {

        vNext = MIN2(vNext, DIST2SPEED(gap));
    }
//    }
//    */
    vNext = MAX2((SUMOReal) 0, vNext);
    if(vNext<=0.1) {
        myWaitingTime++;
    } else {
        myWaitingTime = 0;
    }

    // call reminders after vNext is set
    workOnMoveReminders( myState.myPos,
                         myState.myPos + SPEED2DIST(vNext), vNext );
    // update position and speed
    myState.myPos  += SPEED2DIST(vNext);
    assert( myState.myPos < lane->length() );
    myState.mySpeed = vNext;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        DEBUG_OUT << "movea/2:" << debug_globaltime << ": " << myID << " at " << myLane->getID() << ": " << pos() << ", " << speed() << endl;
    }
#endif
    // !!! remove this! make this somewhere else!
    if(hasCORNDoubleValue(MSCORN::CORN_VEH_LASTREROUTEOFFSET)) {
        myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] =
            myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] + 1;
    }
    // !!! remove this! make this somewhere else!
    myLane->addMean2(vNext, length());
#ifdef RAKNET_DEMO
    setPosition(position().x(), 0, position().y());
#endif
}


void
MSVehicle::moveRegardingCritical(MSLane* lane,
                                 const MSVehicle* pred,
                                 const MSVehicle* neigh )
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        DEBUG_OUT << "moveb/1:" << debug_globaltime << ": " << myID << " at " << myLane->getID() << ": " << pos() << ", " << speed() << endl;
    }
#endif
    myLFLinkLanes.clear();
    // check whether the vehicle is not on an appropriate lane
    if(!myLane->appropriate(this)) {
        // decelerate to lane end when yes
        SUMOReal vWish =
            vsafe(myState.mySpeed,
                ACCEL2SPEED(myType->decel()),
                myLane->length()-myState.myPos/*-MSVehicleType::maxLength()*/,
                0);
        if(pred!=0) {
            vWish = MIN2(vWish,
                vsafe(myState.mySpeed, ACCEL2SPEED(myType->decel()),
                    gap2pred(*pred), pred->speed()) );
        }
        // !!! check whether the vehicle wants to stop somewhere
        if(!myStops.empty()&&&(myStops.begin()->lane->edge())==&(lane->edge())) {
            SUMOReal seen = lane->length() - myState.pos();
            SUMOReal vsafeStop = vsafe(myState.mySpeed, decelAbility(),
                seen-(lane->length()-myStops.begin()->pos), 0);
            vWish = MIN2(vWish, vsafeStop);
        }
        vWish = MAX2((SUMOReal) 0, vWish);
        /*
        }
        */
        myLFLinkLanes.push_back(
            DriveProcessItem(0, vWish, vWish));
    } else {
        // compute other values as in move
        SUMOReal vBeg = vaccel( myState.mySpeed, lane->maxSpeed() );
        if(pred!=0) {
            SUMOReal vSafe =
                vsafe(myState.mySpeed, myType->decel(),
                    gap2pred( *pred ), pred->speed());
            //  the vehcile is bound by the lane speed and must not drive faster
            //  than vsafe to the next vehicle
            vBeg = MIN2(vBeg, vSafe);
        }
        // check whether the driver wants to let someone in
        // set next links, computing possible speeds
        vsafeCriticalCont(vBeg);
    }
    // !!! remove this! make this somewhere else!
    if(hasCORNDoubleValue(MSCORN::CORN_VEH_LASTREROUTEOFFSET)) {
        myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] =
            myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] + 1;
    }
    // !!! remove this! make this somewhere else!
}


void
MSVehicle::moveFirstChecked()
{
    myTarget = 0;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2) ) {
        int textdummy = 0;
    }
#endif
    // get vsafe
    SUMOReal vSafe = 0;
    if(!myStops.empty()) {
        if(myStops.begin()->reached) {
            if(myStops.begin()->duration==0) {
                if(myStops.begin()->busstop!=0) {
                    myStops.begin()->busstop->leaveFrom(this);
                }
                myStops.pop_front();
            } else {
                myStops.begin()->duration--;
                myTarget = myLane;
                myState.mySpeed = 0;
                myLane->addMean2(0, length());
                return; // !!!detectore etc?
            }
        } else {
            if(myStops.begin()->lane==myLane) {
                Stop &bstop = *myStops.begin();
                SUMOReal endPos = bstop.pos;
                bool busStopsMustHaveSpace = true;
                if(bstop.busstop!=0) {
                    endPos = bstop.busstop->getLastFreePos();
                    if(endPos-5.<bstop.busstop->getBeginLanePosition()) { // !!! explicite offset
                        busStopsMustHaveSpace = false;
                    }
                }
                if(myState.pos()>=endPos-BUS_STOP_OFFSET&&busStopsMustHaveSpace) {
                    bstop.reached = true;
                    if(bstop.busstop!=0) {
                        bstop.busstop->enter(this, myState.pos(), myState.pos()-length());
                    }
                }
            }
        }
    }

    assert(myLFLinkLanes.size()!=0);
    DriveItemVector::iterator i;
    MSLane *currentLane = myLane;
    bool cont = true;
    for(i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end()&&cont; i++) {
        MSLink *link = (*i).myLink;
        bool onLinkEnd = link==0;
        // the vehicle must change the lane on one of the next lanes
        if(!onLinkEnd) {
            if(link->havePriority()) {
                vSafe = (*i).myVLinkPass;
            } else {
                if(link->opened()) {
                    vSafe = (*i).myVLinkPass;
                } else {
                    if(vSafe<myState.mySpeed-decelSpeed()&&link->amYellow()) {
                        vSafe = (*i).myVLinkPass;
                    } else {
                        vSafe = (*i).myVLinkWait;
                        cont = false;
                    }
                }
            }
        } else {
            vSafe = (*i).myVLinkWait;
            cont = false;
            break;
        }
        currentLane = link->getLane();
    }
    // compute vNext in considering dawdling
    SUMOReal vNext;
    if(myState.speed()==0&&vSafe<accelSpeed(0)) {
        // do not dawdle too much on short segments
        vNext = MAX2(SUMOReal(0), dawdle2( MIN2(vSafe, vaccel(myState.mySpeed, myLane->maxSpeed())) ));
    } else {
        // lane-change interaction
        ///* !!!!!
        //*/
        vNext = MAX2(SUMOReal(0), dawdle( MIN2(vSafe, vaccel(myState.mySpeed, myLane->maxSpeed())) ));
    vNext =
        myLaneChangeModel->patchSpeed(
            MAX2((SUMOReal) 0, ACCEL2SPEED(myState.mySpeed-decel())),
            vNext,
            MIN2(vSafe, vaccel(myState.mySpeed, myLane->maxSpeed())),
            vSafe);
    vNext = MIN3(vNext, vSafe, vaccel(myState.mySpeed, myLane->maxSpeed()));
    }

    // visit waiting time
    if(vNext<=0.1) {
        myWaitingTime++;
    } else {
        myWaitingTime = 0;
    }
    // call reminders after vNext is set
    SUMOReal pos = myState.myPos;

    // update position
    myState.myPos += SPEED2DIST(vNext);
    // update speed
    myState.mySpeed = vNext;
    MSLane *approachedLane = myLane;
    approachedLane->addMean2(vNext, length());


    // move the vehicle forward
    size_t no = 0;
    SUMOReal driven = approachedLane->length() - pos;
    for(i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end()
        &&
        myState.myPos>approachedLane->length();
        ++i) {

        if(approachedLane!=myLane) {
            leaveLaneAtMove(driven);
        }
        MSLink *link = (*i).myLink;
        // check whether the vehicle was allowed to enter lane
        //  otherwise it is decelareted and we do not need to test for it's
        //  approach on the following lanes when a lane changing is performed
        assert(approachedLane!=0);
        myState.myPos -= approachedLane->length();
        assert(myState.myPos>0);
        if(approachedLane!=myLane) {
            enterLaneAtMove(approachedLane, driven);
            driven += approachedLane->length();
        }
        // proceed to the next lane
        if(link!=0/*approachedLane->isLinkEnd(link)*/) {
#ifdef HAVE_INTERNAL_LANES
            approachedLane = link->getViaLane();
            if(approachedLane==0) {
                approachedLane = link->getLane();
            }
#else
            approachedLane = link->getLane();
#endif
        }
        // set information about approaching
        approachedLane->setApproaching(myState.pos(), this);
        approachedLane->addMean2(vNext, length());
        no++;
    }
    // set approaching information for consecutive lanes the vehicle may reach in the
    //  next steps
    MSLane *tmpApproached = approachedLane;
    SUMOReal dist = brakeGap(myState.mySpeed) - driven;
    SUMOReal tmpPos = myState.myPos + dist;
    for(; dist>0&&tmpApproached->length()<tmpPos&&i!=myLFLinkLanes.end(); i++) {
        MSLink *link = (*i).myLink;
        if(link==0) {
            break;
        }
        tmpPos -= tmpApproached->length();//approachedLane->length();
#ifdef HAVE_INTERNAL_LANES
        tmpApproached = link->getViaLane();
        if(tmpApproached==0) {
            tmpApproached = link->getLane();
        }
#else
        tmpApproached = link->getLane();
#endif
        tmpApproached->setApproaching(tmpPos, this);
    }
    // needed as the lane changer maybe looks back
    //  !!! needed?
    /*
    MSLane *nextLane = myTarget==0 ? myLane : myTarget;
    SUMOReal distToEnd = nextLane->length() - myState.myPos;
    if(distToEnd<MSVehicleType::maxLength()) {
        MSLinkCont::iterator link =
            nextLane->succLinkSec( *this, 1, *nextLane );
        if(!nextLane->isLinkEnd(link)) {
            nextLane = (*link)->getViaLane();
            if(nextLane==0) {
                nextLane = (*link)->getLane();
            }
            nextLane->setApproaching(-distToEnd, this);
        }

    }
    */
    // enter lane herein if no push occures (otherwise, do it there)
    if(no==0) {
        workOnMoveReminders( pos, pos + SPEED2DIST(vNext), vNext );
    }
    myTarget = approachedLane;
    assert(myTarget!=0);
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        DEBUG_OUT << "moveb/1:" << debug_globaltime << ": " << myID << " at " << getLane().getID() << ": " << myState.myPos << ", " << myState.mySpeed << endl;
    }
#endif
#ifdef RAKNET_DEMO
	if(myTarget==myLane) {
	    setPosition(position().x(), 0, position().y());
	}
#endif
    assert(myTarget->length()>=myState.myPos);
}


void
MSVehicle::_assertPos() const
{
    assert(myState.myPos<=myLane->length());
}


void
MSVehicle::vsafeCriticalCont( SUMOReal boundVSafe )
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        DEBUG_OUT << "vsafeCriticalCont/" << debug_globaltime << ":" << myID << endl;
    }
#endif
    SUMOReal decelAbility = decel();
    // the vehicle may have just to look into the next lane
    //  compute this information and use it only once in the next loop
    SUMOReal seen = myLane->length() - myState.myPos;
    MSLane *nextLane = myLane;
    // compute the way the vehicle may drive when accelerating
    SUMOReal dist = boundVSafe + brakeGap(myState.mySpeed);
    SUMOReal vLinkPass = boundVSafe;
    SUMOReal vLinkWait = vLinkPass;

    size_t view = 1;
#ifdef HAVE_INTERNAL_LANES
    bool nextInternal =
        nextLane->edge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL;
#endif
    // loop over following lanes
    while(true) {
        // !!! check whether the vehicle wants to stop somewhere
        if(!myStops.empty()&&&(myStops.begin()->lane->edge())==&(nextLane->edge())) {
            SUMOReal vsafeStop = vsafe(myState.mySpeed, decelAbility,
                seen-(nextLane->length()-myStops.begin()->pos), 0);
            vLinkPass = MIN2(vLinkPass, vsafeStop);
            vLinkWait = MIN2(vLinkWait, vsafeStop);
        }
        // !!!

        // get the next link used
        MSLinkCont::iterator link =
            myLane->succLinkSec( *this, view, *nextLane );
        // check whether the lane is a dead end
        //  (should be valid only on further loop iterations
        if(nextLane->isLinkEnd(link)) {
            SUMOReal laneEndVSafe =
                vsafe(myState.mySpeed, decelAbility, seen, 0);
            myLFLinkLanes.push_back(
                DriveProcessItem(0,
                    MIN2(vLinkPass, laneEndVSafe),
                    MIN2(vLinkPass, laneEndVSafe)));
            // the vehicle will not drive further
            return;
        }
        vLinkWait = vLinkPass; // the link was passed
        // if the vehicle drives over the end of the lane, inform the link

        // get the following lane
#ifdef HAVE_INTERNAL_LANES
        nextLane = (*link)->getViaLane();
        if(nextLane==0) {
            nextInternal = false;
            nextLane = (*link)->getLane();
        } else {
            nextInternal = true;
        }
#else
        nextLane = (*link)->getLane();
#endif
        // compute the velocity to use when the link is not blocked by oter vehicles
            // the vehicle shall be not fastern when reaching the next lane than allowed
        SUMOReal vmaxNextLane =
            vsafe(myState.mySpeed, decelAbility, seen, nextLane->maxSpeed());

            // the vehicle shall keep a secure distance to its predecessor
            //  (or approach the lane end if the predeccessor is too near)
        const State &nextLanePred = nextLane->myLastState;
        SUMOReal dist2Pred = seen+nextLanePred.pos()-MSVehicleType::maxLength(); // @!!! die echte L�nge des fahrzeugs
        SUMOReal vsafePredNextLane;
        if(dist2Pred>=0) {
            // leading vehicle is not overlapping
            vsafePredNextLane =
                vsafe(myState.mySpeed, decelAbility, dist2Pred, nextLanePred.speed());
            SUMOReal predDec = MAX2((SUMOReal) 0, nextLanePred.speed()-decelAbility /* !!! decelAbility of leader! */);
            if(brakeGap(vsafePredNextLane)+vsafePredNextLane*myTau > brakeGap(predDec) + dist2Pred) {

                vsafePredNextLane =
                    MIN2(vsafePredNextLane, DIST2SPEED(dist2Pred));
            }
        } else {
            // leading vehicle is overlapping (stands within the junction)
            vsafePredNextLane =
                vsafe(myState.mySpeed, decelAbility, seen, 0);
        }

            // compute the velocity to use when the link may be used
        vLinkPass =
            MIN3(vLinkPass, vmaxNextLane, vsafePredNextLane/*, vsafeNextLaneEnd*/);

        // if the link may not be used (is blocked by another vehicle) then let the
        //  vehicle decelerate until the end of the street
        vLinkWait =
            MIN2(vLinkWait, vsafe(myState.mySpeed, decelAbility, seen, 0));

        // valid, when a vehicle is not on a priorised lane
        if(!(*link)->havePriority()) {
            // if it has already decelerated to let priorised vehicles pass
            //  and when the distance to the vehicle on the next lane allows moving
            //  (the check whether other incoming vehicles may stop this one is done later)
            // then let it pass
            if(seen<decelAbility&&dist2Pred>0) {
                vLinkPass = MIN2(vLinkPass, vaccel(myState.mySpeed, myLane->maxSpeed())); // otherwise vsafe may become incredibly large
                (*link)->setApproaching(this);
            } else {
                // let it wait in the other cases
                vLinkPass = vLinkWait;
            }
        }
        myLFLinkLanes.push_back(
            DriveProcessItem(*link, vLinkPass, vLinkWait));
        if( vsafePredNextLane>0&&dist-seen>0 ) {
            (*link)->setApproaching(this);
        } else {
            return;
        }
        seen += nextLane->length();
        if(seen>dist) {
            return;
        }
#ifdef HAVE_INTERNAL_LANES
        if(!nextInternal) {
            view++;
        }
#else
        view++;
#endif
    }
}

SUMOReal
MSVehicle::decelAbility() const
{
    return ACCEL2SPEED(decel()); // !!! really the speed?
}


SUMOReal
MSVehicle::accelAbility() const
{
    return ACCEL2SPEED(accel(myState.mySpeed)); // !!! really the speed?
}



////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::dictionary(string id, MSVehicle* ptr)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        myDict.insert(DictType::value_type(id, ptr));
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

MSVehicle*
MSVehicle::dictionary(string id)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        return 0;
    }
    return it->second;
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::clear()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        MSVehicle *veh = (*i).second;
        delete (*i).second;
    }
    myDict.clear();
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::remove(const std::string &id)
{
    DictType::iterator i = myDict.find(id);
    MSVehicle *veh = (*i).second;
    myDict.erase(id);
    delete veh;
}


/////////////////////////////////////////////////////////////////////////////

MSVehicle*
MSVehicle::detach(const std::string &id)
{
    DictType::iterator i = myDict.find(id);
    if(i==myDict.end()) {
        return 0;
    }
    MSVehicle *ret = (*i).second;
    myDict.erase(id);
    return ret;
}


/////////////////////////////////////////////////////////////////////////////

Position2D
MSVehicle::position() const
{
	return (myLane->myShape).positionAtLengthPosition(myState.pos());
}

/////////////////////////////////////////////////////////////////////////////

const string &
MSVehicle::getID() const
{
    return myID;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSVehicle::onAllowed( const MSLane* lane ) const
{
    if(myLane->edge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
        return true;
    }
    if(myAllowedLanes.size()==0) {
        return false; // check (was assertion only)!!!
    }
    assert(myAllowedLanes.size()!=0);
    MSEdge::LaneCont::const_iterator compare =
        find( myAllowedLanes[0]->begin(), myAllowedLanes[0]->end(), lane );
    return ( compare != myAllowedLanes[0]->end() );
}


bool
MSVehicle::onAllowed( const MSLane* lane, size_t offset ) const
{
    if(myLane->edge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
        return true;
    }
    if(myAllowedLanes.size()==0) {
        return false; // check (was assertion only)!!!
    }
    assert(myAllowedLanes.size()!=0);
    MSEdge::LaneCont::const_iterator compare =
        find( myAllowedLanes[offset]->begin(), myAllowedLanes[offset]->end(), lane );
    return ( compare != myAllowedLanes[offset]->end() );
}


bool
MSVehicle::onAllowed( ) const
{
    if(myLane->edge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
        return true;
    }
    if(myAllowedLanes.size()==0) {
        return false; // check (was assertion only)!!!
    }
    assert(myAllowedLanes.size()!=0);
    MSEdge::LaneCont::const_iterator compare =
        find( myAllowedLanes[0]->begin(), myAllowedLanes[0]->end(), myLane );
    return ( compare != myAllowedLanes[0]->end() );
}


/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::enterLaneAtMove( MSLane* enteredLane, SUMOReal driven )
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
    }
#endif
    // save the old work reminders, patching the position information
    // add the information about the new offset to the old lane reminders
    SUMOReal oldLaneLength = myLane->length();
    OffsetVector::iterator i;
    for(i=myOldLaneMoveReminderOffsets.begin(); i!=myOldLaneMoveReminderOffsets.end(); i++) {
        (*i) += oldLaneLength;
    }
    for(size_t j=0; j<myMoveReminders.size(); j++) {
        myOldLaneMoveReminderOffsets.push_back(oldLaneLength);
    }
    copy(myMoveReminders.begin(), myMoveReminders.end(),
        back_inserter(myOldLaneMoveReminders));
    assert(myOldLaneMoveReminders.size()==myOldLaneMoveReminderOffsets.size());
    // set the entered lane as the current lane
    myLane = enteredLane;
    myTarget = enteredLane;
    // and update the mean data
    SUMOReal entryTimestep =
        static_cast< SUMOReal >( MSNet::getInstance()->getCurrentTimeStep() ) - 1 +
        driven / myState.mySpeed;
    assert(entryTimestep<=debug_globaltime);
    // get new move reminder
    myMoveReminders = enteredLane->getMoveReminders();
    // proceed in route
    const MSEdge &enteredEdge = enteredLane->edge();
    if(enteredEdge.getPurpose()!=MSEdge::EDGEFUNCTION_INTERNAL) {
        MSRouteIterator edgeIt = myCurrEdge;
        while ( *edgeIt != &enteredEdge ) {
            ++edgeIt;
            assert( edgeIt != myRoute->end() );
        }
        myCurrEdge = edgeIt;
    }
    if(MSCORN::wished(MSCORN::CORN_VEHCONTROL_WANTS_DEPARTURE_INFO)) {
        MSNet::getInstance()->getVehicleControl().vehicleMoves(this);
    }
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::enterLaneAtLaneChange( MSLane* enteredLane )
{
    myLane = enteredLane;
    // switch to and activate the new lane's reminders
    // keep OldLaneReminders
    myMoveReminders = enteredLane->getMoveReminders();
    myAllowedLanes.clear();
    rebuildAllowedLanes();
    activateRemindersByEmitOrLaneChange();
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::enterLaneAtEmit( MSLane* enteredLane, const State &state )
{
    myState = state;
    assert( myState.myPos >= 0 );
    assert( myState.mySpeed >= 0 );
    myWaitingTime = 0;
    myLane = enteredLane;
    // set and activate the new lane's reminders
    myMoveReminders = enteredLane->getMoveReminders();
    activateRemindersByEmitOrLaneChange();
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::leaveLaneAtMove( SUMOReal driven )
{
    if(!myAllowedLanes.empty()) {
        myAllowedLanes.pop_front();
    }
}

/////////////////////////////////////////////////////////////////////////////

void
MSVehicle::leaveLaneAtLaneChange( void )
{
    // dismiss the old lane's reminders
    SUMOReal savePos = myState.myPos; // have to do this due to SUMOReal-precision errors
    vector< MSMoveReminder* >::iterator rem;
    for(rem=myMoveReminders.begin(); rem != myMoveReminders.end(); ++rem ){
        (*rem)->dismissByLaneChange( *this );
    }
    std::vector<SUMOReal>::iterator off = myOldLaneMoveReminderOffsets.begin();
    for(rem=myOldLaneMoveReminders.begin(); rem!=myOldLaneMoveReminders.end(); ++rem, ++off) {
        myState.myPos += (*off);
        (*rem)->dismissByLaneChange( *this );
        myState.myPos -= (*off);
    }
    myState.myPos = savePos; // have to do this due to SUMOReal-precision errors
    myMoveReminders.clear();
    myOldLaneMoveReminders.clear();
    myOldLaneMoveReminderOffsets.clear();
}

/////////////////////////////////////////////////////////////////////////////

const MSEdge * const
MSVehicle::getEdge() const
{
    return *myCurrEdge;
}



bool
MSVehicle::reachingCritical(SUMOReal laneLength) const
{
    // check whether the vehicle will run over the lane when accelerating
    return (laneLength - myState.myPos - brakeGap(myState.mySpeed))
        <= 0;
}


MSLane *
MSVehicle::getTargetLane() const
{
    return myTarget;
}


const MSLane &
MSVehicle::getLane() const
{
    return *myLane;
}


bool
MSVehicle::periodical() const
{
    return myPeriod>0;
}


MSVehicle *
MSVehicle::getNextPeriodical() const
{
    // check whether another one shall be repated
    if(myRepetitionNumber<=0) {
        return 0;
    }
    return MSNet::getInstance()->getVehicleControl().buildVehicle(
        StringUtils::version1(myID), myRoute, myDesiredDepart+myPeriod,
        myType, myRepetitionNumber-1, myPeriod);
}


bool
MSVehicle::running() const
{
    return myLane!=0;
}




void
MSVehicle::workOnMoveReminders( SUMOReal oldPos, SUMOReal newPos, SUMOReal newSpeed,
                                MoveOnReminderMode mode )
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
    }
#endif
    // This erasure-idiom works for all stl-sequence-containers
    // See Meyers: Effective STL, Item 9
    for (MoveReminderContIt rem=myMoveReminders.begin(); rem!=myMoveReminders.end(); ) {
        if (!(*rem)->isStillActive(*this, oldPos, newPos, newSpeed)) {
            rem = myMoveReminders.erase(rem);
        } else {
            ++rem;
        }
    }
    if(mode!=CURRENT) {
        OffsetVector::iterator off=myOldLaneMoveReminderOffsets.begin();
        for (MoveReminderContIt rem=myOldLaneMoveReminders.begin(); rem!=myOldLaneMoveReminders.end(); ) {
            SUMOReal oldLaneLength = *off;
            if (!(*rem)->isStillActive(*this, oldLaneLength+oldPos, oldLaneLength+newPos, newSpeed) ) {
                rem = myOldLaneMoveReminders.erase( rem );
                off = myOldLaneMoveReminderOffsets.erase(off);
            } else {
                ++rem;
                ++off;
            }
        }
    }
}


void
MSVehicle::activateRemindersByEmitOrLaneChange()
{
    // This erasure-idiom works for all stl-sequence-containers
    // See Meyers: Effective STL, Item 9
    for (MoveReminderContIt rem=myMoveReminders.begin(); rem!=myMoveReminders.end(); ) {
        if (!(*rem)->isActivatedByEmitOrLaneChange( *this )) {
            rem = myMoveReminders.erase( rem );
        } else {
            ++rem;
        }
    }
}


MSAbstractLaneChangeModel &
MSVehicle::getLaneChangeModel()
{
    return *myLaneChangeModel;
}


const MSAbstractLaneChangeModel &
MSVehicle::getLaneChangeModel() const
{
    return *myLaneChangeModel;
}


size_t
MSVehicle::getWaitingTime() const
{
    return myWaitingTime;
}


bool
MSVehicle::proceedVirtualReturnWhetherEnded(const MSEdge *const newEdge)
{
    bool _destReached = destReached(newEdge);
    myAllowedLanes.clear(); // !!! not really necessary!?
    rebuildAllowedLanes();
    return _destReached;
}


void
MSVehicle::onTripEnd(bool wasAlreadySet)
{
    SUMOReal pspeed = myState.mySpeed;
    SUMOReal pos = myState.myPos;
    SUMOReal oldPos = pos - SPEED2DIST(pspeed);
    if ( pos - length() < 0 ) {
        SUMOReal pdist = (SUMOReal) (length() + 0.01) - oldPos;
        pspeed = DIST2SPEED(pdist);
        pos = (SUMOReal) (length() + 0.1);
    }
    pos += myLane->length();
    oldPos += myLane->length();
    // process reminder
        // current
    vector< MSMoveReminder* >::iterator rem;
    for (rem=myMoveReminders.begin(); rem!=myMoveReminders.end(); ++rem) {
        // the vehicle may only be at the entry occupancy correction
        if( (*rem)->isStillActive( *this, oldPos, pos, pspeed) ) {
            assert(false);
        }
    }
        // old
    rem = myOldLaneMoveReminders.begin();
    OffsetVector::iterator off = myOldLaneMoveReminderOffsets.begin();
    for (;rem!=myOldLaneMoveReminders.end(); ++rem, ++off) {
        SUMOReal oldLaneLength = *off;
        if( (*rem)->isStillActive( *this, oldPos+oldLaneLength, pos+oldLaneLength, pspeed) ) {
            assert(false); // !!!
        }
    }
    // remove from structures to be informed about it
    for(QuitRemindedVector::iterator i=myQuitReminded.begin(); i!=myQuitReminded.end(); ++i) {
        (*i)->removeOnTripEnd(this);
    }
    myQuitReminded.clear();
}


void
MSVehicle::removeApproachingInformationOnKill()
{
    DriveItemVector::iterator i = myLFLinkLanes.begin();
    while(i!=myLFLinkLanes.end()&&(*i).myLink!=0/*&&(*i).myLink->getLane()!=begin&&(*i).myLink->getViaLane()!=begin*/) {
        MSLane *tmp = (*i).myLink->getLane();
        if(tmp!=0) {
            tmp->resetApproacherDistance(this);
        }
#ifdef HAVE_INTERNAL_LANES
        tmp = (*i).myLink->getViaLane();
        if(tmp!=0) {
            tmp->resetApproacherDistance(this);
        }
#endif
        ++i;
    }
}


void
MSVehicle::onDepart()
{
    // check whether the vehicle's departure time shall be saved
    myDoubleCORNMap[MSCORN::CORN_VEH_REALDEPART] =
        (SUMOReal) MSNet::getInstance()->getCurrentTimeStep();
    // check whether the vehicle control shall be informed
    if(MSCORN::wished(MSCORN::CORN_VEHCONTROL_WANTS_DEPARTURE_INFO)) {
        MSNet::getInstance()->getVehicleControl().vehicleEmitted(this);
    }
    // initialise devices
    {
        // cell phones
        size_t no = (size_t) myDoubleCORNMap[(MSCORN::Function) MSCORN::CORN_VEH_DEV_NO_CPHONE];
        for(size_t np=0; np<no; np++) {
    	    ((MSDevice_CPhone*) myPointerCORNMap[(MSCORN::Pointer) (MSCORN::CORN_P_VEH_DEV_CPHONE+np)])->onDepart();
        }
    }
}


void
MSVehicle::quitRemindedEntered(MSVehicleQuitReminded *r)
{
    myQuitReminded.push_back(r);
}


void
MSVehicle::quitRemindedLeft(MSVehicleQuitReminded *r)
{
    QuitRemindedVector::iterator i =
        find(myQuitReminded.begin(), myQuitReminded.end(), r);
    if(i!=myQuitReminded.end()) {
        myQuitReminded.erase(i);
    }
}


SUMOReal
MSVehicle::getCORNDoubleValue(MSCORN::Function f) const
{
    return myDoubleCORNMap.find(f)->second;
}


void *
MSVehicle::getCORNPointerValue(MSCORN::Pointer f) const
{
    return myPointerCORNMap.find(f)->second;
}


bool
MSVehicle::hasCORNDoubleValue(MSCORN::Function f) const
{
    return myDoubleCORNMap.find(f)!=myDoubleCORNMap.end();
}


const MSRoute &
MSVehicle::getRoute() const
{
    return *myRoute;
}


const MSRoute &
MSVehicle::getRoute(int index) const
{
    if(index==0) {
        return *myRoute;
    }
    int routeOffset = (int) MSCORN::CORN_P_VEH_OLDROUTE + index - 1;
    std::map<MSCORN::Pointer, void*>::const_iterator i =
        myPointerCORNMap.find((MSCORN::Pointer) routeOffset);
    assert(i!=myPointerCORNMap.end());
    return *((MSRoute*) (*i).second);
}


bool
MSVehicle::replaceRoute(const MSEdgeVector &edges, size_t simTime)
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
        for(MSEdgeVector::const_iterator i=edges.begin(); i!=edges.end(); ++i) {
            DEBUG_OUT << (*i)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
        for(MSRouteIterator i2=myRoute->begin(); i2!=myRoute->end(); ++i2) {
            DEBUG_OUT << (*i2)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
    }
#endif
    MSRoute *otherr = 0;
    const MSEdge *currentEdge = *myCurrEdge;
    // check whether the information shall be saved
    if(MSCORN::wished(MSCORN::CORN_VEH_SAVEREROUTING)) {
        otherr = new MSRoute(*myRoute);
    }
    // try to replace the current route
    bool replaced = myRoute->replaceBy(edges, myCurrEdge);
    if(replaced) {
        // rebuild in-vehicle route information
        myCurrEdge = myRoute->find(currentEdge);
        myAllowedLanes.clear();
        rebuildAllowedLanes();
        // save information that the vehicle was rerouted
        myDoubleCORNMap[MSCORN::CORN_VEH_WASREROUTET] = 1;
            // ... maybe the route information shall be saved for output?
        if( MSCORN::wished(MSCORN::CORN_VEH_SAVEREROUTING)) {
    		int routeOffset = (int) MSCORN::CORN_P_VEH_OLDROUTE +
	    		(int) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
    		myPointerCORNMap[(MSCORN::Pointer) routeOffset] = (void*) otherr;
		    int begEdgeOffset = (int) MSCORN::CORN_P_VEH_ROUTE_BEGIN_EDGE +
			    (int) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
            myPointerCORNMap[(MSCORN::Pointer) begEdgeOffset] = (void*) *myCurrEdge;
		    SUMOTime timeOffset = (SUMOTime) MSCORN::CORN_VEH_REROUTE_TIME +
			    (SUMOTime) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
            myDoubleCORNMap[(MSCORN::Function) timeOffset] = (SUMOReal) simTime;
        }
        myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] = 0;
        myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] =
            myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] + 1;
    }
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
        for(MSRouteIterator i=myRoute->begin(); i!=myRoute->end(); ++i) {
            DEBUG_OUT << (*i)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
    }
#endif
    return replaced;
}


bool
MSVehicle::replaceRoute(MSRoute *newRoute, size_t simTime)
{
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
        for(MSEdgeVector::const_iterator i=newRoute->begin(); i!=newRoute->end(); ++i) {
            DEBUG_OUT << (*i)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
        for(MSRouteIterator i2=myRoute->begin(); i2!=myRoute->end(); ++i2) {
            DEBUG_OUT << (*i2)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
    }
#endif
    MSRoute *otherr = 0;
    const MSEdge *currentEdge = *myCurrEdge;
    if(newRoute->find(currentEdge)==newRoute->end()) {
        return false;
    }
    // check whether the information shall be saved
    if(MSCORN::wished(MSCORN::CORN_VEH_SAVEREROUTING)) {
        otherr = new MSRoute(*myRoute);
    }
    // try to replace the current route
    myRoute = newRoute;
    // rebuild in-vehicle route information
    myCurrEdge = myRoute->find(currentEdge);
    myAllowedLanes.clear();
    rebuildAllowedLanes();
    // save information that the vehicle was rerouted
    myDoubleCORNMap[MSCORN::CORN_VEH_WASREROUTET] = 1;
    // ... maybe the route information shall be saved for output?
    if( MSCORN::wished(MSCORN::CORN_VEH_SAVEREROUTING)) {
        int routeOffset = (int) MSCORN::CORN_P_VEH_OLDROUTE +
	        (int) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
        myPointerCORNMap[(MSCORN::Pointer) routeOffset] = (void*) otherr;
		int begEdgeOffset = (int) MSCORN::CORN_P_VEH_ROUTE_BEGIN_EDGE +
		    (int) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
        myPointerCORNMap[(MSCORN::Pointer) begEdgeOffset] = (void*) *myCurrEdge;
		SUMOTime timeOffset = (SUMOTime) MSCORN::CORN_VEH_REROUTE_TIME +
		    (SUMOTime) myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE];
        myDoubleCORNMap[(MSCORN::Function) timeOffset] = (SUMOReal) simTime;
    }
    myDoubleCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] = 0;
    myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] =
        myDoubleCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] + 1;
#ifdef ABS_DEBUG
    if(debug_globaltime>debug_searchedtime && (myID==debug_searched1||myID==debug_searched2)) {
        int textdummy = 0;
        for(MSRouteIterator i=myRoute->begin(); i!=myRoute->end(); ++i) {
            DEBUG_OUT << (*i)->getID() << ", ";
        }
        DEBUG_OUT << "-------------" << endl;
    }
#endif
    assert((MSEdge*)succEdge(1)!=0);
    return true;
}


const MSVehicleType &
MSVehicle::getVehicleType() const
{
    return *myType;
}


void
MSVehicle::rebuildAllowedLanes()
{
    SUMOReal dist = 0;
    // check what was already computed
    for(NextAllowedLanes::const_iterator i=myAllowedLanes.begin(); i!=myAllowedLanes.end(); ++i) {
        dist += ((*(*i))[0])->length();
    }
    SUMOReal MIN_DIST = 3000;
    if(dist<MIN_DIST) {
        size_t pos = distance(myRoute->begin(), myCurrEdge) + myAllowedLanes.size();
        if(pos>=myRoute->size()-1) {
            return;
        }
        const MSEdge::LaneCont *al = ( *myRoute )[pos]->allowedLanes( *( *myRoute )[pos+1] );
        /* !!!
        if(find_if(myStops.begin(), myStops.end(), edge_finder(( *myRoute )[pos+1])!=myStops.end()) {

        }
        */
        while(al!=0&&dist<MIN_DIST&&pos<myRoute->size()-1/*&&*ce!=myRoute->getLastEdge()*/) {
            assert(al!=0);
            myAllowedLanes.push_back(al);
            pos++;
            if(pos<myRoute->size()-1) {
                dist += ((*al)[0])->length();
                al = ( *myRoute )[pos]->allowedLanes( *( *myRoute )[pos+1] );
            }
        }
    }
}


int
MSVehicle::countAllowedContinuations(const MSLane *lane, int dir) const
{
    int ret = 0;
    SUMOReal MIN_DIST = 3000;
    MSRouteIterator ce = myCurrEdge;
    SUMOReal dist = -myState.pos();
    do {
        if(!myStops.empty()&&&(myStops.begin()->lane->edge())==&(lane->edge())) {
            if(lane!=myStops.begin()->lane) {
                return ret;
            } else {
                return ret+1;
            }
        }
        if(ce==myRoute->end()-1) {
            return ret + 1;
        }
        const MSEdge::LaneCont *al = ( *ce )->allowedLanes( **( ce + 1 ) );
        if(al==0) {
            return ret;
        }
        MSEdge::LaneCont::const_iterator i = find(al->begin(), al->end(), lane);
        if(i==al->end()) {
            if(dir==0) {
                return ret;
            }
            // !!!! may be a lane that must be used to leave...
            const std::vector<MSLane*> *pl = lane->edge().getLanes();
            std::vector<MSLane*>::const_iterator pli = find(pl->begin(), pl->end(), lane);
            if(dir<0) {
                // to right
                if(pli==pl->begin()) {
                    return ret;
                }
                lane = *(pli-1);
                i = find(al->begin(), al->end(), lane);
                if(i==al->end()) {
                    return ret;
                }
            }
            if(dir>0) {
                // to left
                if(pli==pl->end()-1) {
                    return ret;
                }
                lane = *(pli+1);
                i = find(al->begin(), al->end(), lane);
                if(i==al->end()) {
                    return ret;
                }
            }
        }
        ret++;
        lane = (*(lane->succLinkOneLane(*( ce + 1 ), *lane)))->getLane();
        assert(al!=0);
        ++ce;
        dist += ((*al)[0])->length();
    } while(dist<MIN_DIST&&ce!=myRoute->end()-1);
    return ret;
}


SUMOReal
MSVehicle::allowedContinuationsLength(const MSLane *lane, int dir) const
{
    SUMOReal MIN_DIST = 3000;
    MSRouteIterator ce = myCurrEdge;
    SUMOReal dist = -myState.pos();
    do {
        if(!myStops.empty()&&&(myStops.begin()->lane->edge())==&(lane->edge())) {
            if(lane!=myStops.begin()->lane) {
                return dist;
            } else {
                return dist+myStops.begin()->pos;
            }
        }
        if(ce==myRoute->end()-1) {
            dist += lane->length();
            return dist;
        }
        const MSEdge::LaneCont *al = ( *ce )->allowedLanes( **( ce + 1 ) );
        if(al==0) {
            return dist;
        }
        MSEdge::LaneCont::const_iterator i =
            find(al->begin(), al->end(), lane);
        if(i==al->end()) {
            if(dir==0) {
                return dist;
            }
            // !!!! may be a lane that must be used to leave...
            const std::vector<MSLane*> *pl = lane->edge().getLanes();
            std::vector<MSLane*>::const_iterator pli = find(pl->begin(), pl->end(), lane);
            if(dir<0) {
                // to right
                if(pli==pl->begin()) {
                    return dist;
                }
                lane = *(pli-1);
                i = find(al->begin(), al->end(), lane);
                if(i==al->end()) {
                    return dist;
                }
            }
            if(dir>0) {
                // to left
                if(pli==pl->end()-1) {
                    return dist;
                }
                lane = *(pli+1);
                i = find(al->begin(), al->end(), lane);
                if(i==al->end()) {
                    return dist;
                }
            }
        }
        lane = (*(lane->succLinkOneLane(*( ce + 1 ), *lane)))->getLane();
        assert(al!=0);
        ++ce;
        dist += ((*al)[0])->length();
    } while(dist<MIN_DIST&&ce!=myRoute->end()-1);
    return dist;
}


void
MSVehicle::writeXMLRoute(std::ostream &os, int index) const
{
	MSRoute *route2Write = myRoute;
    // check if a previous route shall be written
    os << "      <route";
	if(index>=0) {
        // write edge on which the vehicle was when the route was valid
		std::map<MSCORN::Pointer, void*>::const_iterator j =
			myPointerCORNMap.find(
                (MSCORN::Pointer) (MSCORN::CORN_P_VEH_ROUTE_BEGIN_EDGE+index));
        os << " replacedOnEdge=\"" << ((MSEdge*) (*j).second)->getID() << "\" ";
        // write the time at which the route was replaced
        std::map<MSCORN::Function, SUMOReal>::const_iterator j2 =
			myDoubleCORNMap.find(
                (MSCORN::Function) (MSCORN::CORN_VEH_REROUTE_TIME+index));
        os << " replacedAtTime=\"" << toString<size_t>((size_t) (*j2).second) << "\"";
        // get the route
        j = myPointerCORNMap.find((MSCORN::Pointer) (MSCORN::CORN_P_VEH_OLDROUTE+index));
		assert(j!=myPointerCORNMap.end());
		route2Write = (MSRoute*) j->second;
    }
    os << ">";
    // write the route
    route2Write->writeEdgeIDs(os);
    os << "</route>" << endl;
}


void
MSVehicle::interactWith(const std::vector<MSVehicle*> &vehicles)
{
}


void
MSVehicle::setCORNColor(SUMOReal red, SUMOReal green, SUMOReal blue)
{
    myDoubleCORNMap[MSCORN::CORN_VEH_OWNCOL_RED] = red;
    myDoubleCORNMap[MSCORN::CORN_VEH_OWNCOL_GREEN] = green;
    myDoubleCORNMap[MSCORN::CORN_VEH_OWNCOL_BLUE] = blue;
}


void
MSVehicle::dict_saveState(std::ostream &os, long what)
{
    FileHelpers::writeUInt(os, myDict.size());
    for(DictType::iterator it = myDict.begin(); it!=myDict.end(); ++it) {
        if((*it).second->hasCORNDoubleValue(MSCORN::CORN_VEH_REALDEPART)) {
            (*it).second->saveState(os, what);
        }
    }
    FileHelpers::writeString(os, "-----------------end---------------");
}


#ifdef HAVE_MESOSIM
#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>
#include "MSGlobals.h"
#endif

void
MSVehicle::saveState(std::ostream &os, long what)
{
    FileHelpers::writeString(os, myID);
    FileHelpers::writeInt(os, myLastLaneChangeOffset);
    FileHelpers::writeUInt(os, myWaitingTime);
    FileHelpers::writeInt(os, myRepetitionNumber);
    FileHelpers::writeInt(os, myPeriod);
    FileHelpers::writeString(os, myRoute->getID());
    FileHelpers::writeUInt(os, myDesiredDepart);
    FileHelpers::writeString(os, myType->getID());
    FileHelpers::writeUInt(os, myRoute->posInRoute(myCurrEdge));
    FileHelpers::writeUInt(os, (unsigned int) getCORNDoubleValue(MSCORN::CORN_VEH_REALDEPART));
#ifdef HAVE_MESOSIM
    // !!! several things may be missing
    if(seg==0) {
        FileHelpers::writeUInt(os, 0);
        FileHelpers::writeFloat(os, tEvent);
        FileHelpers::writeFloat(os, tLastEntry);
    } else {
        FileHelpers::writeUInt(os, seg->get_index());
        FileHelpers::writeFloat(os, tEvent);
        FileHelpers::writeFloat(os, tLastEntry);
    }
    FileHelpers::writeByte(os, inserted);
#endif
}


void
MSVehicle::dict_loadState(BinaryInputDevice &bis, long what)
{
    unsigned int size;
    bis >> size;
    string id;
    do {
        bis >> id;
    if(id=="98786") {
        int bla = 0;
    }
        if(id!="-----------------end---------------") {
            SUMOTime lastLaneChangeOffset;
            bis >> lastLaneChangeOffset; // !!! check type= FileHelpers::readInt(os);
            unsigned int waitingTime;
            bis >> waitingTime;
            int repetitionNumber;
            bis >> repetitionNumber;
            int period;
            bis >> period;
            string routeID;
            bis >> routeID;
            MSRoute* route;
            unsigned int desiredDepart;
            bis >> desiredDepart;
            string typeID;
            bis >> typeID;
            const MSVehicleType* type;
//!!!        State state;
//!!!        string laneID = FileHelpers::readString(os);
            unsigned int routeOffset;
            bis >> routeOffset;
            unsigned int wasEmitted;
            bis >> wasEmitted;

            // !!! several things may be missing
            unsigned int segIndex;
            bis >> segIndex;
            SUMOReal tEvent;
            bis >> tEvent;
            SUMOReal tLastEntry;
            bis >> tLastEntry;

            //
            route = MSRoute::dictionary(routeID);
            route->incReferenceCnt();
            assert(route!=0);
            type = MSVehicleType::dictionary(typeID);
            assert(type!=0);
        //if(wasEmitted!=0) {
            if(dictionary(id)!=0) {
                DEBUG_OUT << "Error: vehicle was already added" << endl;
                continue;
            }

            MSVehicle *v = MSNet::getInstance()->getVehicleControl().buildVehicle(id,
                route, desiredDepart, type, repetitionNumber, period);
            v->myDoubleCORNMap[MSCORN::CORN_VEH_REALDEPART] = (SUMOReal) wasEmitted;
            while(routeOffset>0) {
                v->myCurrEdge++;
                routeOffset--;
            }
#ifdef HAVE_MESOSIM
            v->seg = MSGlobals::gMesoNet->getSegmentForEdge(*(v->myCurrEdge));
            while(v->seg->get_index()!=segIndex) {
                v->seg = MSGlobals::gMesoNet->next_segment(v->seg, v);
            }
            v->tEvent = tEvent;
            v->tLastEntry = tLastEntry;
            bool inserted;
            bis >> inserted;
            v->inserted = inserted!=0;
#endif
            if(!dictionary(id, v)) {
                cout << "Could not build vehicle!!!" << endl;
                throw 1;
            }
            //
        //}
            size--;
        }
    } while(id!="-----------------end---------------");
    DEBUG_OUT << myDict.size() << " vehicles loaded."; // !!! verbose
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
