/****************************************************************************/
/// @file    ROJTRRouter.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Jan 2004
/// @version $Id$
///
// The junction-percentage router
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
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

#include <router/RONet.h>
#include "ROJTRRouter.h"
#include "ROJTREdge.h"
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
ROJTRRouter::ROJTRRouter(RONet &net, bool unbuildIsWarningOnly,
                         bool acceptAllDestinations)
        : myNet(net), myUnbuildIsWarningOnly(unbuildIsWarningOnly),
        myAcceptAllDestination(acceptAllDestinations)
{
    myMaxEdges = (int)(
                     ((SUMOReal) net.getEdgeNo()) *
                     OptionsCont::getOptions().getFloat("max-edges-factor"));
}


ROJTRRouter::~ROJTRRouter()
{}


void
ROJTRRouter::compute(const ROEdge *from, const ROEdge * /*to*/,
                     const ROVehicle * const vehicle,
                     SUMOTime time, std::vector<const ROEdge*> &into)
{
    const ROJTREdge *current = static_cast<const ROJTREdge*>(from);
    // route until a sinks has been found
    while (current!=0
            &&
            current->getType()!=ROEdge::ET_SINK
            &&
            (int) into.size()<myMaxEdges) {

        into.push_back(current);
        time += (SUMOTime) current->getDuration(vehicle, time);
        current = current->chooseNext(time);
    }
    // check whether no valid ending edge was found
    if ((int) into.size()>=myMaxEdges) {
        if (myAcceptAllDestination) {
            return;
        } else {
            MsgHandler *mh = 0;
            if (myUnbuildIsWarningOnly) {
                mh = MsgHandler::getWarningInstance();
            } else {
                mh = MsgHandler::getErrorInstance();
            }
            mh->inform("The route starting at edge '" + from->getID() + "' could not be closed.");
        }
    }
    // append the sink
    if (current!=0) {
        into.push_back(current);
    }
}



/****************************************************************************/

