/***************************************************************************
                          MSLane.C  -  The place where Vehicles
                          operate.
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
// Revision 1.6  2002/10/28 12:59:38  dkrajzew
// vehicles are now deleted whe the tour is over
//
// Revision 1.5  2002/10/18 11:51:03  dkrajzew
// breakRequest or driveRequest may be set, althoug no first vehicle exists due to regarding a longer break gap...; assertion in moveFirst replaced by a check with a normal exit
//
// Revision 1.4  2002/10/17 13:35:23  dkrajzew
// insecure usage of potentially null-link-lanes patched
//
// Revision 1.3  2002/10/17 10:43:35  dkrajzew
// error of setting of link-driverequests out of a vehicles route repaired
//
// Revision 1.2  2002/10/16 16:43:48  dkrajzew
// regard of artifactsarising from traffic lights implemented; debugged
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.14  2002/10/15 10:24:30  roessel
// MSLane::MeanData constructor checks now for illegal intervals.
//
// Revision 1.13  2002/09/25 17:14:42  roessel
// MeanData calculation and output implemented.
//
// Revision 1.12  2002/07/31 17:33:00  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.15  2002/07/26 11:10:50  dkrajzew
// unset myAllowedLanes when departing from a source debugged; gap on
// URGENT_LANECHANGE_WISH added
//
// Revision 1.14  2002/07/24 16:29:40  croessel
// New method isEmissionSuccess(), used by MSTriggeredSource.
//
// Revision 1.13  2002/07/08 05:07:49  dkrajzew
// MSVC++-compilation problems removed
//
// Revision 1.12  2002/07/03 15:48:40  croessel
// Implementation of findNeigh changed.
// Call to MSVehicle::move adapted to new signature.
// Emit methods use now MSVehicle::safeEmitGap instead of safeGap.
// decel2much uses now MSVehicle::hasSafeGap instead of safeGap.
//
// Revision 1.11  2002/06/20 11:14:13  dkrajzew
// False assertion corrected
//
// Revision 1.10  2002/06/19 15:12:00  croessel
// In moveExceptFirst: Check for timeheadway < deltaT situations.
//
// Revision 1.9  2002/05/29 17:06:03  croessel
// Inlined some methods. See the .icc files.
//
// Revision 1.8  2002/04/24 13:06:47  croessel
// Changed signature of void detectCollisions() to void detectCollisions(
// MSNet::Time )
//
// Revision 1.7  2002/04/18 14:51:11  croessel
// In setDriveRequests(): set gap to UINT_MAX instead of 0 for vehicles
// without a predecessor.
//
// Revision 1.6  2002/04/18 14:05:21  croessel
// Changed detectCollisions() output from pointer-address to vehicle-id.
//
// Revision 1.5  2002/04/17 14:44:32  croessel
// Forgot to reset visited lanes (see previous revision).
//
// Revision 1.4  2002/04/17 14:02:11  croessel
// Bugfix in setLookForwardState: myGap may be < 0 in the PRED_ON_SUCC
// state if pred just entered the succ-lane from another source lane. In
// this case the lane's first vehicle shouldn't leave this lane.
//
// Revision 1.3  2002/04/11 15:25:55  croessel
// Changed float to double.
//
// Revision 1.2  2002/04/11 12:32:07  croessel
// Added new lookForwardState "URGENT_LANECHANGE_WISH" for vehicles that
// may drive beyond the lane but are not on a lane that is linked to
// their next route-edge. A second succLink method, named succLinkSec was
// needed.
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.7  2002/03/27 12:11:09  croessel
// In constructor initialization of member myLastVeh added.
//
// Revision 2.6  2002/03/27 10:35:10  croessel
// moveExceptFirst(...): Last vehicle state must be buffered
// for multi-lane traffic too.
//
// Revision 2.5  2002/03/20 15:58:32  croessel
// Return to previous revision.
//
// Revision 2.3  2002/03/13 17:40:59  croessel
// Calculation of rearPos in setLookForwardState() fixed by introducing
// the new member myLastVeh in addition to myLastVehState. We need both,
// the state and the length of the vehicle.
//
// Revision 2.2  2002/03/13 16:56:35  croessel
// Changed the simpleOutput to XMLOutput by introducing nested classes
// XMLOut. Output is now indented.
//
// Revision 2.1  2002/03/06 15:57:25  croessel
// Modifications to setLookForwardState() so that overlapping vehicles
// from the succeeding lane won't cause collisions.
//
// Revision 2.0  2002/02/14 14:43:16  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.16  2002/02/13 10:05:26  croessel
// Front-gap calculation in emit() and emitTry() modified by usage of
// MSVehicleType::maxLength() and MSVehicleType::minDecel().
//
// Revision 1.15  2002/02/05 13:51:52  croessel
// GPL-Notice included.
// In *.cpp files also config.h included.
//
// Revision 1.14  2002/02/05 11:47:45  croessel
// Adaption of method-name-change in MSVehicle.
//
// Revision 1.13  2002/01/30 16:05:51  croessel
// Bug in detectCollisions() fixed. myVehicles must contain two or more
// vehicles to enter the for-loop.
//
// Revision 1.12  2002/01/24 18:30:40  croessel
// Changes due to renaming of MSVehicle's gap methods.
//
// Revision 1.11  2002/01/23 11:16:28  croessel
// emit() rewritten to avoid redundant code and make things clearer. This
// leads to some overloaded emitTry()'s and a new enoughSpace() method.
//
// Revision 1.10  2002/01/16 15:41:42  croessel
// New implementation of emit(). Should be collision free now. Introduced
// a new protected method "bool emitTry()" for convenience.
//
// Revision 1.9  2002/01/09 14:58:42  croessel
// Added MSLane::Link::setPriority( bool ) for StopLights to modify the
// link's priority. Used to implement the "green arrow".
//
// Revision 1.8  2001/12/19 16:29:31  croessel
// New std::-files included.
//
// Revision 1.7  2001/12/13 15:59:47  croessel
// In request(): Adaption due to new junction hierarchy.
//
// Revision 1.6  2001/12/06 13:13:25  traffic
// Junction types extracted into a separate class
//
// Revision 1.5  2001/11/21 15:21:01  croessel
// Bug fixes.
// Adapted setLookForwardState() to the new member myLastState and the
// MSVehicle::nextState to changed parameter list.
//
// Revision 1.4  2001/11/15 17:17:35  croessel
// Outcommented the inclusion of the inline *.iC files. Currently not needed.
// Vehicle-State introduced. Simulation is now independant of vehicle's
// speed. Still, the position is fundamental, also a gap between
// vehicles. But that's it.
//
// Revision 1.3  2001/11/14 15:47:34  croessel
// Merged the diffs between the .C and .cpp versions. Numerous changes
// in MSLane, MSVehicle and MSJunction.
//
// Revision 1.2  2001/11/14 10:49:06  croessel
// CR-line-end removed.
//
// Revision 1.1  2001/10/24 07:12:36  traffic
// new extension
//
// Revision 1.7  2001/10/23 09:31:00  traffic
// parser bugs removed
//
// Revision 1.5  2001/09/06 15:47:08  croessel
// Numerous changes during debugging session.
//
// Revision 1.4  2001/08/16 13:11:46  traffic
// minor MSVC++-problems solved
//
// Revision 1.3  2001/07/25 12:17:39  traffic
// CC problems with make_pair repaired
//
// Revision 1.2  2001/07/16 12:55:47  croessel
// Changed id type from unsigned int to string. Added string-pointer
// dictionaries and dictionary methods.
//
// Revision 1.1.1.1  2001/07/11 15:51:13  traffic
// new start
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <utils/common/UtilExceptions.h>
#include "MSVehicle.h"
#include "MSVehicleType.h"
#include "MSEdge.h"
#include "MSJunction.h"
#include "MSLogicJunction.h"
#include "MSInductLoop.h"
#include "MSLink.h"
#include "MSLane.h"
#include <cmath>
#include <bitset>
#include <iostream>
#include <cassert>
#include <functional>
#include <algorithm>
#include <iterator>
#include <exception>
#include <climits>

using namespace std;


// Init static member.
MSLane::DictType MSLane::myDict;

/////////////////////////////////////////////////////////////////////////////

MSLane::PosGreater::result_type
MSLane::PosGreater::operator() (first_argument_type veh1,
                                second_argument_type veh2) const
{
    return veh1->pos() > veh2->pos();
}

/////////////////////////////////////////////////////////////////////////////

MSLane::~MSLane()
{
    for(LinkCont::iterator i=myLinks.begin(); i!=myLinks.end(); i++) {
        delete *i;
    }
    // TODO
}

/////////////////////////////////////////////////////////////////////////////

MSLane::MSLane( string id,
                double maxSpeed,
                double length,
                MSEdge* edge
                )  :
    myID( id ),
    myVehicles(),
    myLastVehState(),
    myLastVeh( 0 ),
    myMaxSpeed( maxSpeed ),
    myLength( length ),
    myEdge( edge ),
    myNextJunction( 0 ),
    myVehBuffer( 0 ),
    myRequestLane( 0 ),
    myBrakeRequest( false ),
    myLFState( UNDEFINED ),
    myLFDestReached( false ),
    myGap( 0 ),
    myPredState( MSVehicle::State() ),
    myTargetLane( 0 ),
    myTargetState( MSVehicle::State() ),
    myTargetPos( 0 ),
//      myMeanData( 3 )
    myMeanData( MSNet::getInstance()->getNDumpIntervalls() +
                MSNet::getInstance()->withGUI() )
{
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::initialize( MSJunction* backJunction,
                    LinkCont* links )
{
    myNextJunction = backJunction;
    myLinks = *links;
    delete links;
//      myMeanData.insert( myMeanData.end(),
//                         MSNet::getInstance()->getNDumpIntervalls() +
//                         MSNet::getInstance()->withGUI(),
//                         MeanDataValues() );
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::moveExceptFirst()
{
//    // If there is a vehicle in the myVehicleBuffer, i.e. it was set
//    // there by myPredJunction, insert it at the end of the lane.
//    buffer2lane();

    // Buffer last vehicles state.
    if ( myVehicles.size() == 0 ) {

        myLastVehState = MSVehicle::State();
        myLastVeh      = 0;
        return;
    }
    else {

        MSVehicle* last = *myVehicles.begin();
        myLastVehState  = last->state();
        myLastVeh       = last;
    }

    if ( myVehicles.size() >= 2 ) {
        // Move vehicles except first and all vehicles that may reach something
        //  that forces them to stop
        VehCont::iterator lastBeforeFirst = myVehicles.end() - 1;
        VehCont::iterator veh;
        for ( veh = myVehicles.begin();
              !(*veh)->reachingCritical(myLength) && veh != lastBeforeFirst; 
              ++veh ) {

            VehCont::iterator pred = veh + 1;
            ( *veh )->move( this, *pred, 0 );
            ( *veh )->meanDataMove();

	        // Check for timeheadway < deltaT
	        assert( ( *veh )->pos() < ( *pred )->pos() );
            assert( ( *veh )->pos() < myLength );
	    }
        // Move all next vehicles beside the first
        for ( ;veh != lastBeforeFirst; 
              ++veh ) {

            VehCont::iterator pred = veh + 1;
            ( *veh )->moveRegardingCritical( this, *pred, 0 );
            ( *veh )->meanDataMove();

	        // Check for timeheadway < deltaT
	        assert( ( *veh )->pos() < ( *pred )->pos() );
            assert( ( *veh )->pos() < myLength );
	    }
    }
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::moveExceptFirst( MSEdge::LaneCont::const_iterator firstNeighLane,
                         MSEdge::LaneCont::const_iterator lastNeighLane )
{
//    // If there is a vehicle in the myVehicleBuffer, i.e. it was set
//    // there by myPredJunction, insert it at the end of the lane.
//    buffer2lane();

    // Buffer last vehicles state.
    if ( myVehicles.size() == 0 ) {

        myLastVehState = MSVehicle::State();
        myLastVeh      = 0;
        return;
    }
    else {

        MSVehicle* last = *myVehicles.begin();
        myLastVehState  = last->state();
        myLastVeh       = last;
    }

    if ( myVehicles.size() >= 2 ) {
        // Move vehicles except first and all vehicles that may reach something
        //  that forces them to stop
        VehCont::const_iterator lastBeforeFirst = myVehicles.end() - 1;
        VehCont::const_iterator veh;
        for ( veh = myVehicles.begin();
              !(*veh)->reachingCritical(myLength) && veh != lastBeforeFirst; 
              ++veh ) {

            VehCont::const_iterator pred( veh + 1 );
            const MSVehicle*
                neigh = findNeigh( *veh, firstNeighLane, lastNeighLane );
            // veh has neighbour to regard.
            if ( neigh != *veh ) {

                ( *veh )->move( this, *pred, neigh );
            }

            // veh has no neighbour to regard.
            else {

                ( *veh )->move( this, *pred, 0);
            }

            ( *veh )->meanDataMove();
            // Check for timeheadway < deltaT
            assert( ( *veh )->pos() < ( *pred )->pos() );
            assert( ( *veh )->pos() < myLength );
        }
        // Move all next vehicles beside the first
        for ( ; veh != lastBeforeFirst; 
              ++veh ) {

            VehCont::const_iterator pred( veh + 1 );
            const MSVehicle*
                neigh = findNeigh( *veh, firstNeighLane, lastNeighLane );

            // veh has neighbour to regard.
            if ( neigh != *veh ) {

                ( *veh )->moveRegardingCritical( this, *pred, neigh );
            }

            // veh has no neighbour to regard.
            else {

                ( *veh )->moveRegardingCritical( this, *pred, 0);
            }

            ( *veh )->meanDataMove();
            // Check for timeheadway < deltaT
            assert( ( *veh )->pos() < ( *pred )->pos() );
            assert( ( *veh )->pos() < myLength );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::detectCollisions( MSNet::Time timestep ) const
{
    if ( myVehicles.size() < 2 ) {
        return;
    }

    VehCont::const_iterator lastVeh = myVehicles.end() - 1;
    for ( VehCont::const_iterator veh = myVehicles.begin();
          veh != lastVeh; ++veh ) {

        VehCont::const_iterator pred = veh + 1;
        double gap = ( *pred )->pos() - ( *pred )->length() - ( *veh )->pos();
        if ( gap < 0 ) {
            cerr << "MSLane::detectCollision: Collision of " << ( *veh )->id()
                 << " with " << ( *pred )->id() << " on MSLane " << myID
                 << " during timestep " << timestep << endl;
            ofstream strm("D:\\projects\\sumo\\Projekte\\I-880\\data2\\error.log");
            strm << "Yes" << endl;
            strm.close();
            throw ProcessError();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::emit( MSVehicle& veh )
{
    // If this lane is empty, set newVeh on position beyond safePos =
    // brakeGap(laneMaxSpeed) + MaxVehicleLength. (in the hope of that
    // the precening lane hasn't a much higher MaxSpeed)
    // This safePos is ugly, but we will live with it in this revision.
    double safePos = pow( myMaxSpeed, 2 ) / ( 2 * MSVehicleType::minDecel() ) +
                    MSVehicle::tau() + MSVehicleType::maxLength();
    assert( safePos < myLength ); // Lane has to be longer than safePos,
    // otherwise emission (this kind of emission) makes no sense.

    // Here the emission starts
    if ( empty() ) {

        return emitTry( veh );
    }

    // Try to emit as last veh. (in driving direction)
    VehCont::iterator leaderIt = myVehicles.begin();
    if ( emitTry( veh, leaderIt ) ) {

        return true;
    }

    // if there is only one veh on this lane, try to
    // emit in front of this veh. (leader becomes follower)
    if ( leaderIt + 1 ==  myVehicles.end() ) {

        return emitTry( leaderIt, veh );
    }

    // At least two vehicles on lane.
    // iterate over follow/leader -pairs
    VehCont::iterator followIt = leaderIt;
    ++leaderIt;
    for (;;) {

        // try to emit between follower and leader
        if ( emitTry( followIt, veh, leaderIt ) ) {

            return true;
        }

        // if leader is the first veh on this lane, try
        // to emit in front of it.
        if ( leaderIt + 1 == myVehicles.end() ) {

            return emitTry( leaderIt, veh );
        }

        // iterate
        ++leaderIt;
        ++followIt;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::isEmissionSuccess( MSVehicle* aVehicle )
{
    aVehicle->departLane();
    MSLane::VehCont::iterator predIt =
        find_if( myVehicles.begin(), myVehicles.end(),
                 bind2nd( MSInductLoop::VehPosition(), aVehicle->pos() ) );

    if ( predIt != myVehicles.end() ) {

        MSVehicle* pred = *predIt;
        if ( ! aVehicle->isInsertTimeHeadWayCond( *pred ) ||
             ! aVehicle->isInsertBrakeCond( *pred ) ) {

            return false;
        }

        // emit
        aVehicle->enterLaneAtEmit( this );
        myVehicles.insert( predIt, aVehicle );
        return true;
    }

    // no Predecessor
    myFirst = aVehicle;
    setLookForwardState();
    double predSpeed = 0;

    switch ( myLFState ) {
        case FREE_ON_CURR:
        case FREE_ON_SUCC:
        {
            aVehicle->enterLaneAtEmit( this );
            myVehicles.push_back( aVehicle );
            return true;
        }
        case YIELD_ON_CURR:
        case YIELD_ON_SUCC:
        {
            break;
        }
        case PRED_ON_SUCC:
        {
            predSpeed = myPredState.speed();
            break;
        }
        default:
        {
            assert( false ); // Never get here.
        }
    };

    if ( ! aVehicle->isInsertTimeHeadWayCond( predSpeed, myGap ) ||
         ! aVehicle->isInsertBrakeCond( predSpeed, myGap ) ) {

        return false;
    }

    // emit
    aVehicle->enterLaneAtEmit( this );
    myVehicles.push_back( aVehicle );
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::emitTry( MSVehicle& veh )
{
    // empty lane emission.
    double safeSpace = pow( myMaxSpeed, 2 ) /
                      ( 2 * MSVehicleType::minDecel() ) +
                      MSVehicle::tau() +
                      veh.length();
    if ( enoughSpace( veh, 0, myLength, safeSpace ) ) {
        veh.enterLaneAtEmit( this );
        myVehicles.push_front( &veh );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::emitTry( MSVehicle& veh, VehCont::iterator leaderIt )
{
    // emission as last car (in driving direction)
    MSVehicle *leader = *leaderIt;
    double leaderPos = leader->pos() - leader->length();
    double safeSpace = pow( myMaxSpeed, 2 ) /
                      ( 2 * MSVehicleType::minDecel() ) +
                      MSVehicle::tau() +
                      veh.length() + veh.accelDist();
    if ( enoughSpace( veh, 0, leaderPos, safeSpace ) ) {
        veh.enterLaneAtEmit( this );
        myVehicles.push_front( &veh );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::emitTry( VehCont::iterator followIt, MSVehicle& veh )
{
    // emission as first car (in driving direction)
    double followPos = (*followIt)->pos();
    double safeSpace = (*followIt)->safeEmitGap() + veh.length();
    if ( enoughSpace( veh, followPos,
                      myLength - MSVehicleType::maxLength(), safeSpace ) ) {
        veh.enterLaneAtEmit( this );
        myVehicles.push_back( &veh );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::emitTry( VehCont::iterator followIt, MSVehicle& veh,
                 VehCont::iterator leaderIt )
{
    MSVehicle *leader = *leaderIt;
    MSVehicle *follow = *followIt;
    // emission between follower and leader.
    double followPos = (*followIt)->pos();
    double leaderPos = (*leaderIt)->pos() - (*leaderIt)->length();
    double safeSpace = (*followIt)->safeEmitGap() + veh.length();
    if ( enoughSpace( veh, followPos, leaderPos, safeSpace ) ) {
        veh.enterLaneAtEmit( this );
        myVehicles.insert( leaderIt, &veh );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::enoughSpace( MSVehicle& veh,
                     double followPos, double leaderPos, double safeSpace )
{
    double free = leaderPos - followPos - safeSpace;
    if ( free >= 0 ) {

        // prepare vehicle with it's position
        MSVehicle::State state;
        state.setPos( followPos + safeSpace + free / 2 );
        veh.moveSetState( state );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::clearRequest()
{
    for ( LinkCont::iterator link = myLinks.begin(); link !=
              myLinks.end(); ++link) {
        (*link)->myDriveRequest = false;
    }

    myRequestLane = 0;
    myBrakeRequest = false;

    myLFState = UNDEFINED;
    myLFLinkLanes.clear();
    myLFDestReached = false;

}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::setRequest()
{
    if ( myVehicles.empty() ) {
        myFirst = 0;  // only for security reasons.
        // The "request"-member myBrakeRequest is set to false in
        // clearRequest(), so there is nothing to do here. In request()
        // "false" will be returned by default if no requesting car
        // is found.
    }
    else {
        myFirst = myVehicles.back( );
        setLookForwardState();
        assert( myLFState != UNDEFINED ) ;
        setDriveRequests();
    }
}

/////////////////////////////////////////////////////////////////////////////

MSLogicJunction::DriveBrakeRequest
MSLane::request() const
{
    // Initialize requests to false.
    MSLogicJunction::Request juncReq( myLinks.size(), false );
    bool driveReq( false );

    // Search for an requested link if this lane is not empty.
    if ( myFirst ) {
        LinkCont::const_iterator laneReq = find_if ( myLinks.begin(),
                                                     myLinks.end(),
                                                     MSLink::LinkRequest() );

        // If requested link found, set corresponding request to true.
        unsigned int index = distance( myLinks.begin(), laneReq );
        if ( laneReq != myLinks.end() ) {
            juncReq[ index ] = true;
            driveReq = true;
        }
    }

    return MSLogicJunction::DriveBrakeRequest( juncReq, driveReq,
                                          myBrakeRequest );
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::moveFirst( bool respond )
{
    if(myFirst==0) {
        return;
    }
//    assert( myFirst );
    assert( myFirst->pos()<myLength);
    double bla = myFirst->speed();
    // update position and speed
    // Move as calculated if response is true
    if ( respond == true /*|| myTargetLane!=this*/) {

        myFirst->moveSetState( myTargetState );

        // change onto targetLane
        if ( myTargetLane != this ) {

            myTargetLane->push( pop() );
        }

    }

    // Slow down towards lane end.
    else {
        myFirst->moveDecel2laneEnd( this );
        if(myFirst->pos()>=myLength) {
            myFirst->moveSetState(myTargetState);
            myTargetLane->push( pop() );
        }
    }
    myFirst->_assertPos();
    assert(bla-myFirst->decelAbility() <= myFirst->speed());
}

/////////////////////////////////////////////////////////////////////////////

MSLane*
MSLane::firstVehSuccLane()
{
    assert( myFirst );
    return ( *succLink( *myFirst, 1, *this ) )->myLane;
}

/////////////////////////////////////////////////////////////////////////////

MSLane*
MSLane::firstVehSuccLane(const MSLane* srcLane)
{
    // Find the source lane within the look-forward-lanes.
    LFLinkLanes::const_iterator ll = myLFLinkLanes.begin();

    if ( srcLane == this ) {
        return ll->myLink->myLane;
    }

    while ( ll->myLane != srcLane ) {

        assert( ll != myLFLinkLanes.end() ); // Don't get into
        // infinite loop.

        ++ll;
    }
    assert( ll != myLFLinkLanes.end() );

    return ll->myLink->myLane;
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::decel2much(const MSLane* compete, const MSLane* target,
                   double decelFactor)
{
    assert( decelFactor >= 0 && decelFactor <= 1 );

    // calculate the gap from this lane's first vehicle to the beginning
    // of the target lane. Then add the competing vehicle's desired position.
    double gap2compete = myLength - myFirst->pos();
    LFLinkLanes::const_iterator ll = myLFLinkLanes.begin();

    while ( ll->myLane != target ) {

        assert( ll != myLFLinkLanes.end() );
        gap2compete += ll->myLane->myLength;
        ++ll;
    }
    gap2compete += compete->myTargetPos;

    // Is there enough space between the two vehicles.
    if ( myFirst->hasSafeGap( *(compete->myFirst), compete, gap2compete ) ) {

        // Prioritized vehicle has now predecessor. Update desired State
        myTargetState = myFirst->nextStateCompete( this,
                                                   compete->myTargetState,
                                                   gap2compete );
        return false;
    }
    else {

        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool
MSLane::dictionary(string id, MSLane* ptr)
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

MSLane*
MSLane::dictionary(string id)
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
MSLane::clear()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::push(MSVehicle* veh)
{
    // Insert vehicle only if it's destination isn't reached.
    assert( myVehBuffer == 0 );
    if ( ! veh->destReached( myEdge ) ) { // adjusts vehicles routeIterator
        myVehBuffer = veh;
        veh->enterLaneAtMove( this );
    }
    else {
        MSVehicle::remove(veh->id());
        // TODO
        // This part has to be discussed, quick an dirty solution:
        // Destination reached. Vehicle vanishes.
        // maybe introduce a vehicle state ...
    }
}

/////////////////////////////////////////////////////////////////////////////

MSVehicle*
MSLane::pop()
{
    assert( ! myVehicles.empty() );
    MSVehicle* first = myVehicles.back( );
    first->leaveLaneAtMove();
    myVehicles.pop_back();
    return first;
}

/////////////////////////////////////////////////////////////////////////////

void MSLane::setLookForwardState()
{
    assert( myFirst );
    assert( myFirst->hasSuccEdge(1) == true );
    assert(myFirst->pos()<myLength);

    myOldState  = myFirst->state();
    myGap       = 0;
    myPredState = MSVehicle::State();

    // Calculate maxLook. This is the distance the vehicle
    // should check for predecessors or yield-junctions.
    double maxLook = myFirst->brakeGap( this ) + MSVehicleType::maxLength();

    double looked = myLength - myFirst->pos(); // The distance already
    // looked at.

    // Cannot look past lane end and considers overlapping vehicle.
    if ( looked - MSVehicleType::maxLength() >= maxLook ) {

        // State free driving on current lane found
        myLFState = FREE_ON_CURR;
        return;
    }

    // Can look beyond lane end. Determine the link that will be used.
    MSLane::LinkCont::const_iterator link = 
        succLinkSec( *myFirst, 1, *this );

    // The vehicle is on a wrong lane; we assume the vehicle wants to
    // change the lane but the destination lane is congested.
    if ( link == myLinks.end() ) {

        myLFState = URGENT_LANECHANGE_WISH;
        myGap = looked - MSVehicleType::maxLength();
        return;
    }

    firstPriorised = (*link)->myPrio;

    // If link is yield-link and veh is still to fast for safe driving.
    if ( ( (*link)->myPrio == false ) &&
         ( willLeaveLane( *myFirst ) == false ) ) {

        // State yield on current lane found
        myLFState = YIELD_ON_CURR;
        // Check for overlapping vehicle. If there is one we assume that
        // it has speed = 0. Then all we need is a sufficient gap.
        MSLane* succLane = (*link)->myLane;
        assert( succLane->length() > MSVehicleType::maxLength() );
        if ( ! succLane->empty() ) {

            MSVehicle::State predState = succLane->myLastVehState;
            MSVehicle*       pred      = succLane->myLastVeh;
            assert ( pred != 0 );
            double rearPos = predState.pos() - pred->length();

            // Does pred overlap?
            if ( rearPos < 0 ) {
                myGap = looked + rearPos;
                return;
            }
        }

        myGap = looked;
        return;
    }

    // do something with vehicles that cannot manage to break
    if( linkClosed(*link) && ( willLeaveLane( *myFirst ) ) ) {

    }
    // We are going to look into a succeeding lane. Let this lane remenber
    // that it was visited.
    this->myRequestLane = this;
    this->myBrakeRequest = true;

    // Look into succLanes until maxlook exhausted, a succLane occupied,
    // vehicle looks at destination or succLink is yield-link.
    MSLane* succLane = 0;
    for (;;) {

        // Look into succLane, remenber this lane and set let this lane know
        // that a vehicle is going to visit it.
        succLane = (*link)->myLane;
        myLFLinkLanes.push_back( LFLinkLane( *link, succLane ) );

        // Is succLane empty?
        if ( succLane->empty() ) {

            // Is maxLook exhausted?
            looked += succLane->myLength;
            if ( looked > maxLook ) {

                // State free driving on a succLane found
                // Check for destReached outside the loop
                myLFState = FREE_ON_SUCC;
                break;
            }
            else {

                // If there is no next link, the vehicle is looking into it's
                // destination lane. Determine state outside the loop.
                if ( ! myFirst->hasSuccEdge( myLFLinkLanes.size() + 1 ) ) {
                    break;
                }

                // Determine existing next link
                link = succLink( *myFirst, myLFLinkLanes.size() + 1,
                                 *succLane );

                // If Link to next Lane is a yield-link
                if ( (*link)->myPrio == false ) {

                    // State yield on a succLane found
                    myLFState = YIELD_ON_SUCC;
                    myGap     = looked;
                    return;
                }
                else {
                    // Look into succLane
                    // This is the only branch the forever-loop loops

                    // We are going to look into a succeeding lane. Let the
                    // current lane remenber that it was visited.
                    succLane->myRequestLane = this;
                    succLane->myBrakeRequest = true;
                }
            }
        }

        // SuccLane is not empty
        else {
            break;
        }
    } // for (;;)

    // SuccLane is not empty or state is FREE_ON_SUCC and/or
    // vehicle is looking at destination lane

    // If vehicle is looking at destination lane and set destReached flag.
    if ( ! myFirst->hasSuccEdge( myLFLinkLanes.size() + 1 ) ) {

        myLFDestReached = true;
    }

    // succLane is empty. Set empty-lane-state if not already done.
    if ( succLane->empty() ) {

        if ( myLFState == FREE_ON_SUCC ) {

            // State already set
        }
        else {

            // Vehicle can look beyond it's destination lane.
            myLFState = BEYOND_DEST;

            // We need gap here for a simple handling of this state:
            // decelleration towards dest-lane-end.
            myGap     = looked;
            return;
        }
        return;
    }

    // succLane not empty. Set state.
    else {

        // Is maxLook exhausted?
        MSVehicle::State predState = succLane->myLastVehState;
        MSVehicle*            pred = succLane->myLastVeh;
        assert( pred != 0 );

        double gap2pred = looked + predState.pos() - pred->length();
        if ( gap2pred < 0 ) {

            // Predecessor is overlapping it's lane. Don't leave lane.
            // Reset visited lanes.
            myLFState = YIELD_ON_CURR;
            myGap     = myLength - myFirst->pos();
            myBrakeRequest = false;
            for ( LFLinkLanes::iterator it = myLFLinkLanes.begin();
                  it != myLFLinkLanes.end(); ++it ) {
                it->myLane->myRequestLane  = 0;
                it->myLane->myBrakeRequest = false;
            }
            return;
        }

        if ( gap2pred > maxLook ) {

            // State free free driving on a succLane found
            myLFState = FREE_ON_SUCC;
            return;
        }

        // State having a predecessor on a succlane found
        myLFState   = PRED_ON_SUCC;
        myGap       = gap2pred;
        myPredState = predState;
        return;
    }
    assert( false ); // You should not be here.
}

bool 
MSLane::appropriate(const MSVehicle *veh) const
{
    MSLane::LinkCont::const_iterator link = succLinkSec( *veh, 1, *this );
    return ( link != myLinks.end() );
}

//////////////////////////////////////////////////////////////////////////

bool
MSLane::willLeaveLane( const MSVehicle& first ) const {

    // TODO
    // Simple implementation:
    // Will vehicle leave lane due to acceleration?
    return ( first.pos() + first.driveDist( first.accelState( this ) )
             > myLength );
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::setDriveRequests()
{
    assert( myFirst );

    myTargetLane  = 0;
    myTargetState = MSVehicle::State();
    myTargetPos   = 0;
    // With correspondence to the LFstate, myGap and myPred, calculate the
    // (maybe) temporary targetSpeed, targetLane and targetPosition.
    // Set the hopefully used link-requests to true.
    // Move the vehicles that won't interact (even with brake-requests)
    // with the junction.

    // TODO: in a later version, take care of the neighbour while calculating
    // the targetSpeed

    switch ( myLFState ) {
        case FREE_ON_CURR: {
            // Move vehicle freely.
            myFirst->moveUpdateState(
                myFirst->nextState( this,
                                    MSVehicle::State(),
                                    static_cast< double >( UINT_MAX ),
                                    MSVehicle::State(),
                                    0) );
            return;
        }

        case URGENT_LANECHANGE_WISH: {
            // the vehicle must change the lane to get on a valid lane but
            // can not because this lane is congested
            // slow the vehicle down and let it wait as long as necessary
            myFirst->moveUpdateState( myFirst->nextState( this, myGap ) );
            myFirst->_assertPos();
            return;
        }

        case YIELD_ON_CURR: {
            // Move vehicle slowing down towards the end of this lane.
            myFirst->moveUpdateState( myFirst->nextState( this, myGap ) );
            myFirst->_assertPos();
            return;
        }

        case FREE_ON_SUCC: {
            myTargetState =
                myFirst->nextState( this,
                                    MSVehicle::State(),
                                    static_cast< double >( UINT_MAX ),
                                    MSVehicle::State(), 0 );
            break;
        }

        case YIELD_ON_SUCC: {
            myTargetState = myFirst->nextState( this, myGap );
            break;
        }

        case PRED_ON_SUCC: {
            myTargetState = myFirst->nextState( this, myPredState, myGap,
                                                MSVehicle::State(), 0 );
            break;
        }

        case BEYOND_DEST: {
            // Don't know exactely what to do with this case. Where in a lane
            // will the vehicle park/disappear?

            // To Do something, slow down towards the end of the
            // destination-lane.
            myTargetState = myFirst->nextState( this, myGap );

            break;
        }

        default: {
            assert( false ); // State should be set.
            return;
        }
    };

    if(!firstPriorised&&myTargetLane!=this) {
        if(myTargetState.speed()>myFirst->decelAbility()) {
            myTargetState = myFirst->nextState( this, myLength-myFirst->pos() );
/*            double decelratedSpeed = myFirst->speed() - myFirst->decelAbility();
            double maximumPossible = myFirst->decelAbility();
            double speed = decelratedSpeed > maximumPossible ?
                decelratedSpeed : maximumPossible;
            double pos = 
                myFirst->pos() + speed * MSNet::deltaT();
            myTargetState = MSVehicle::State(pos, speed);*/
            if(myTargetState.pos()<=myLength) {
                myTargetLane = this;
            }   
        }
    }

    // With myTargetState set, determine Target-Pos and Lane.

    // Distance the vehicle can drive in one timestep.
    double driveDist = myFirst->driveDist( myTargetState );
    double seenDriveDist = myFirst->rigorousBrakeGap(myTargetState)
        - (myLength - myFirst->pos() );
    if(seenDriveDist<driveDist) {
        seenDriveDist = driveDist;
    }
    // set the requests 
    double looked = myLength - myTargetState.pos();
    MSLane *currLane = this;
    if(myFirst->hasSuccEdge(1)) {
        MSLane::LinkCont::const_iterator link = 
            succLinkSec( *myFirst, 1, *this );
        size_t followerNo = 1;
        while ( seenDriveDist > looked && myFirst->hasSuccEdge(followerNo)) {

            //assert( ll != myLFLinkLanes.end() ); // Loop-exit condition
            // should be reached earlier.
            (*link)->myDriveRequest = true;
            if(myFirst->hasSuccEdge(++followerNo)) {
                link = succLinkSec( *myFirst, 1, *currLane );
                currLane = (*link)->myLane;
                looked += currLane->myLength;
            }
        }
    }


    // Set the end position
    looked = myLength - myFirst->pos();
    LFLinkLanes::iterator ll = myLFLinkLanes.begin();
    myTargetLane = 0;
    if ( driveDist < looked ) {
        myTargetLane = this;
    } else {
        while ( driveDist > looked ) {
            assert( ll != myLFLinkLanes.end() ); // Loop-exit condition
            // should be reached earlier.

//            ll->myLink->myDriveRequest = true;
            looked += ll->myLane->myLength;

            ++ll;
        }
        --ll;
        myTargetLane  = ll->myLane;
        myTargetPos   = ll->myLane->myLength - (looked - driveDist);
        myTargetState.setPos( myTargetPos );
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::integrateNewVehicle()
{
    if ( myVehBuffer ) {
        myVehicles.push_front( myVehBuffer );
        myVehBuffer = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////

MSLane::LinkCont::const_iterator
MSLane::succLink(const MSVehicle& veh, unsigned int nRouteSuccs,
                 const MSLane& succLinkSource) const
{
    const MSEdge* nRouteEdge = veh.succEdge( nRouteSuccs );
    assert( nRouteEdge != 0 );

    // Check which link's lane belongs to the nRouteEdge.
    for ( LinkCont::const_iterator link = succLinkSource.myLinks.begin();
          link != succLinkSource.myLinks.end() ; ++link ) {

        if ( ( *link )->myLane->myEdge == nRouteEdge ) {
            return link;
        }
    }
    assert( false ); // There must be a matching edge.
    return succLinkSource.myLinks.end();
}

bool 
MSLane::inEdge(const MSEdge *edge) const
{
    return myEdge==edge;
}

const MSVehicle * const 
MSLane::getLastVehicle() const
{
    return myLastVeh;
}


bool 
MSLane::linkClosed(const MSLink *link) const
{
    return myNextJunction->linkClosed(link);
}


/////////////////////////////////////////////////////////////////////////////

MSLane::LinkCont::const_iterator
MSLane::succLinkSec(const MSVehicle& veh, unsigned int nRouteSuccs,
                    const MSLane& succLinkSource) const
{
    const MSEdge* nRouteEdge = veh.succEdge( nRouteSuccs );
    assert( nRouteEdge != 0 );

    // Check which link's lane belongs to the nRouteEdge.
    LinkCont::const_iterator link;
    for ( link = succLinkSource.myLinks.begin();
          link != succLinkSource.myLinks.end() ; ++link ) {

        const MSLane * lane = ( *link )->myLane;
        if ( lane!=0 && lane->myEdge == nRouteEdge ) {
            return link;
        }
    }
    return link;
}

/////////////////////////////////////////////////////////////////////////////

const MSVehicle*
MSLane::findNeigh( MSVehicle* veh,
                   MSEdge::LaneCont::const_iterator first,
                   MSEdge::LaneCont::const_iterator last )
{
    MSVehicle* neighbour = veh;
    double vNeighEqual( 0 );

    for ( MSEdge::LaneCont::const_iterator neighLane = first;
          neighLane != last; ++neighLane ) {

        VehCont::const_iterator tmpNeighbour =
            find_if( ( *neighLane )->myVehicles.begin(),
                     ( *neighLane )->myVehicles.end(),
                     bind2nd( PosGreater(), veh ) );
        if ( tmpNeighbour == ( *neighLane )->myVehicles.end() ) {

            continue;
        }

        // neighbour found
        if ( (*tmpNeighbour)-> congested() ) {
            continue;
        }

        double tmpVNeighEqual = veh->vNeighEqualPos( **tmpNeighbour );
        if ( neighbour == veh || tmpVNeighEqual < vNeighEqual ) {

            neighbour   = *tmpNeighbour;
            vNeighEqual = tmpVNeighEqual;
        }
    }
    return neighbour;
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::resetMeanData( unsigned index )
{
    MeanDataValues& md = myMeanData[ index ];
    md.nVehFinishedLane = 0;
    md.nVehContributed = 0;
    md.contTimestepSum = 0;
    md.discreteTimestepSum = 0;
    md.distanceSum = 0;
    md.speedSum = 0;
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::addVehicleData( double contTimesteps,
                        unsigned discreteTimesteps,
                        double travelDistance,
                        double speedSum,
                        double speedSquareSum,
                        unsigned index,
                        bool hasFinishedLane )
{
    MeanDataValues& md = myMeanData[ index ];

    md.nVehFinishedLane     += hasFinishedLane;
    md.nVehContributed      += 1;
    md.contTimestepSum      += contTimesteps;
    md.discreteTimestepSum  += discreteTimesteps;
    md.distanceSum          += travelDistance;
    md.speedSum             += speedSum;
    md.speedSquareSum       += speedSquareSum;
}

/////////////////////////////////////////////////////////////////////////////

void
MSLane::collectVehicleData( unsigned index )
{
    for ( VehCont::iterator it = myVehicles.begin();
          it != myVehicles.end(); ++it ) {

        (*it)->dumpData( index );
    }
}


/////////////////////////////////////////////////////////////////////////////

ostream&
operator<<( ostream& os, const MSLane& lane )
{
    os << "MSLane: Id = " << lane.myID << endl;
    return os;
}

/////////////////////////////////////////////////////////////////////////////

MSLane::XMLOut::XMLOut( const MSLane& obj,
                        unsigned indentWidth,
                        bool withChildElemes ) :
    myObj( obj ),
    myIndentWidth( indentWidth ),
    myWithChildElemes( withChildElemes )
{
}

/////////////////////////////////////////////////////////////////////////////

ostream&
operator<<( ostream& os, const MSLane::XMLOut& obj )
{
    string indent( obj.myIndentWidth , ' ' );

    if ( obj.myWithChildElemes ) {

        if ( obj.myObj.myVehicles.empty() == true &&
             obj.myObj.myVehBuffer == 0 ) {

            os << indent << "<lane id=\"" << obj.myObj.myID
               << "\"/>" << endl;
        }
        else { // not empty
            os << indent << "<lane id=\"" << obj.myObj.myID << "\">"
               << endl;

            if ( obj.myObj.myVehBuffer != 0 ) {

                os << MSVehicle::XMLOut( *(obj.myObj.myVehBuffer),
                                         obj.myIndentWidth + 4,
                                         true );
            }

            for ( MSLane::VehCont::const_iterator veh =
                      obj.myObj.myVehicles.begin();
                  veh != obj.myObj.myVehicles.end(); ++veh ) {

                os << MSVehicle::XMLOut( **veh, obj.myIndentWidth + 4,
                                         false );
            }

            os << indent << "</lane>" << endl;

        }
    }

    else { // no child elemets, just print the number of vehicles

        unsigned nVeh = obj.myObj.myVehicles.size() +
                        ( obj.myObj.myVehBuffer != 0 );
        os << indent << "<lane id=\"" << obj.myObj.myID << "\" nVehicles=\""
           << nVeh << "\" />" << endl;
    }

    return os;
}

/////////////////////////////////////////////////////////////////////////////

MSLane::MeanData::MeanData( const MSLane& obj,
                            unsigned index,
                            MSNet::Time interval ) :
    myObj( obj ),
    myIndex( index ),
    myInterval( interval )
{
    if ( myInterval == 0 ){
        cerr << "MSLane::MeanData constructor: interval = 0, should be > 0.\n"
             << "I will set it to 5 minutes.\n";
        myInterval = static_cast<unsigned>( 300 / MSNet::deltaT() );
    }
}

/////////////////////////////////////////////////////////////////////////////

ostream&
operator<<( ostream& os, const MSLane::MeanData& obj )
{
    const double meanVehLength = 7.5;
    const MSLane& lane = obj.myObj;
    const MSLane::MeanDataValues& meanData = lane.myMeanData[ obj.myIndex ];

    const_cast< MSLane& >( lane ).collectVehicleData( obj.myIndex );
    
    // calculate mean data
    double travelTime;
    double meanSpeed;
    double meanDensity;
    double meanFlow;

    if ( meanData.nVehContributed > 0 ) {

        travelTime  = ( meanData.contTimestepSum *
                        static_cast< double >( MSNet::deltaT() ) ) /
            ( meanData.distanceSum / lane.myLength );

        meanSpeed   = meanData.speedSum /
            static_cast< double >( meanData.discreteTimestepSum );

        meanDensity = static_cast< double >
            ( meanData.discreteTimestepSum * MSNet::deltaT() ) /
            ( lane.myLength / meanVehLength *
              static_cast< double >( obj.myInterval * MSNet::deltaT() ) );

        meanFlow    = static_cast< double >( meanData.nVehFinishedLane ) /
            static_cast< double >( obj.myInterval ); 
    }
    else {

        assert( meanData.nVehContributed == 0 );
        travelTime  = lane.myLength / lane.myMaxSpeed;
        meanSpeed   = -1;
        meanDensity = 0;
        meanFlow    = 0;
    }
    
    os << "      <lane id=\""      << obj.myObj.myID
       << "\" traveltime=\"" << travelTime
       << "\" speed=\""      << meanSpeed 
       << "\" density=\""    << meanDensity
       << "\" flow=\""       << meanFlow
       << "\"/>" << endl;
  
    const_cast< MSLane& >( lane ).resetMeanData( obj.myIndex );
        
    return os;
}

void 
MSLane::setLinkPriorities(const std::bitset<64> &prios, size_t &beginPos)
{
    for(LinkCont::iterator i=myLinks.begin(); i!=myLinks.end(); i++) {
        (*i)->setPriority(prios.test(beginPos++));
    }
}


const MSLane::LinkCont &
MSLane::getLinkCont() const
{
    return myLinks;
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#ifdef DISABLE_INLINE
#include "MSLane.icc"
#endif

// Local Variables:
// mode:C++
// End:









