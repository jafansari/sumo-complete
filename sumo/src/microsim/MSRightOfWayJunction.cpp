/***************************************************************************
                          MSRightOfWayJunction.cpp  -  Usual right-of-way
                          junction.
                             -------------------
    begin                : Wed, 12 Dez 2001
    copyright            : (C) 2001 by Christian Roessel
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
// Revision 1.3  2003/02/07 10:41:50  dkrajzew
// updated
//
// Revision 1.2  2002/10/16 16:42:29  dkrajzew
// complete deletion within destructors implemented; clear-operator added for container; global file include; junction extended by position information (should be revalidated later)
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.3  2002/04/18 10:53:20  croessel
// In findCompetitor we now ignore lanes, that don't have a vehicle that
// is able to leave the lane.
//
// Revision 1.2  2002/04/11 15:25:56  croessel
// Changed float to double.
//
// Revision 1.1.1.1  2002/04/08 07:21:23  traffic
// new project name
//
// Revision 2.3  2002/03/06 10:56:36  croessel
// Bugfix: myRespond will have always the correct size before being passed to myLogic.
//
// Revision 2.2  2002/02/27 13:47:57  croessel
// Additional assert's because of parameter-passing-problems.
//
// Revision 2.1  2002/02/21 18:49:45  croessel
// Deadlock-killer implemented.
//
// Revision 2.0  2002/02/14 14:43:19  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.5  2002/02/01 15:48:26  croessel
// Changed condition in moveFirstVehicles() again.
//
// Revision 1.4  2002/02/01 14:14:33  croessel
// Changed condition in moveFirstVehicles(). Now vehicles with a
// BrakeRequest only will also be moved.
//
// Revision 1.3  2002/02/01 11:52:28  croessel
// Removed function-adaptor findCompetitor from inside the class to the
// outside to please MSVC++.
//
// Revision 1.2  2002/02/01 11:40:34  croessel
// Changed return-type of some void methods used in for_each-loops to
// bool in order to please MSVC++.
//
// Revision 1.1  2001/12/13 15:54:49  croessel
// Initial commit. Has been MSJunction.cpp before.
//

/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "MSRightOfWayJunction.h"
#include "MSLane.h"
#include "MSJunctionLogic.h"
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;

//-------------------------------------------------------------------------//

MSRightOfWayJunction::InLane::InLane(MSLane* lane) :
    myLane( lane )
{
}

//-------------------------------------------------------------------------//

MSRightOfWayJunction::MSRightOfWayJunction( string id,
                                            double x, double y,
                                            InLaneCont in,
                                            MSJunctionLogic* logic)
    : MSLogicJunction( id, x, y, in ),
    myLogic( logic )
{
}


bool
MSRightOfWayJunction::clearRequests()
{
    myRequest.reset();
//    myRespond.reset();
    return true;
}

//-------------------------------------------------------------------------//

MSRightOfWayJunction::~MSRightOfWayJunction()
{
/*    for(InLaneCont::iterator i1=myInLanes->begin(); i1!=myInLanes->end(); i1++) {
        delete (*i1);
    }
    delete myInLanes;*/
    // Remark: All logics are deleted using MSJunctionLogic::clear()
}

//-------------------------------------------------------------------------//

bool
MSRightOfWayJunction::setAllowed()
{
    // Get myRespond from logic and check for deadlocks.
    myLogic->respond( myRequest, myRespond );
    deadlockKiller();
    return true;
}

//-------------------------------------------------------------------------//


void
MSRightOfWayJunction::deadlockKiller()
{
    if ( myRequest.none() ) {
        return;
    }

    if ( myRespond.none() ) {

        // Handle deadlock: Create randomly a deadlock-free request out of
        // myRequest, i.e. a "single bit" request. Then again, send it
        // through myLogic (this is neccessary because we don't have a
        // mapping between requests and lanes.) !!! (we do now!!)
        vector< unsigned > trueRequests;
        trueRequests.reserve( myRespond.size() );
        for ( unsigned i = 0; i < myRequest.size(); ++i ) {

            if ( myRequest.test(i) ) {

                trueRequests.push_back( i );
                assert( trueRequests.size() <= myRespond.size() );
            }
        }
        // Choose randomly an index out of [0,trueRequests.size()];
        // !!! random choosing may choose one of less priorised lanes
        unsigned noLockIndex = static_cast< unsigned > (
            floor( static_cast< double >( rand() ) /
                    static_cast< double >( RAND_MAX ) *
                    trueRequests.size()
                )
            );

        // Create deadlock-free request.
        std::bitset<64> noLockRequest(false);
        assert(trueRequests.size()>noLockIndex);
        noLockRequest.set( trueRequests[ noLockIndex ] );
        // Calculate respond with deadlock-free request.
        myLogic->respond( noLockRequest, myRespond );
    }
    return;
}

//-------------------------------------------------------------------------//





/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "MSRightOfWayJunction.icc"
//#endif

// Local Variables:
// mode:C++
// End:
