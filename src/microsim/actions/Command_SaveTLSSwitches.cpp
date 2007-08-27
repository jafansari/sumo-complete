/****************************************************************************/
/// @file    Command_SaveTLSSwitches.cpp
/// @author  Daniel Krajzewicz
/// @date    06 Jul 2006
/// @version $Id$
///
// Writes the switch times of a tls into a file
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

#include "Command_SaveTLSSwitches.h"
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSLink.h>
#include <microsim/MSLane.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>

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
Command_SaveTLSSwitches::Command_SaveTLSSwitches(
    const MSTLLogicControl::TLSLogicVariants &logics,
    OutputDevice &od)
        : myOutputDevice(od), myLogics(logics)
{
    MSNet::getInstance()->getEndOfTimestepEvents().addEvent(this,
            0, MSEventControl::ADAPT_AFTER_EXECUTION);
    myOutputDevice << "<tls-switches>" << "\n";
}


Command_SaveTLSSwitches::~Command_SaveTLSSwitches()
{
    myOutputDevice << "</tls-switches>" << "\n";
}


SUMOTime
Command_SaveTLSSwitches::execute(SUMOTime currentTime)
{
    MSTrafficLightLogic *light = myLogics.defaultTL;
    const MSTrafficLightLogic::LinkVectorVector &links = light->getLinks();
    const std::bitset<64> &allowedLinks = light->allowed();
    for (size_t i=0; i<links.size(); i++) {
        if (!allowedLinks.test(i)) {
            const MSTrafficLightLogic::LinkVector &currLinks = links[i];
            const MSTrafficLightLogic::LaneVector &currLanes = light->getLanesAt(i);
            for (int j=0; j<(int) currLinks.size(); j++) {
                if (myPreviousLinkStates.find(currLinks[j])==myPreviousLinkStates.end()) {
                    continue;
                } else {
                    MSLink *link = currLinks[j];
                    SUMOTime lastOn = myPreviousLinkStates[link].first;
                    bool saved = myPreviousLinkStates[link].second;
                    if (!saved) {
                        myOutputDevice << "   <tlsswitch tls=\"" << light->getID()
                        << "\" subid=\"" << light->getSubID()
                        << "\" fromLane=\"" << currLanes[j]->getID()
                        << "\" toLane=\"" << link->getLane()->getID()
                        << "\" begin=\"" << lastOn
                        << "\" end=\"" << currentTime
                        << "\" duration=\"" << (currentTime-lastOn)
                        << "\"/>" << "\n";
                        myPreviousLinkStates[link] = make_pair<SUMOTime, bool>(lastOn, true);
                    }
                }
            }
        } else {
            const MSTrafficLightLogic::LinkVector &currLinks = links[i];
            for (MSTrafficLightLogic::LinkVector::const_iterator j=currLinks.begin(); j!=currLinks.end(); j++) {
                if (myPreviousLinkStates.find(*j)!=myPreviousLinkStates.end()) {
                    if (!myPreviousLinkStates[*j].second) {
                        continue;
                    }
                }
                myPreviousLinkStates[*j] = make_pair<SUMOTime, bool>(currentTime, false);
            }
        }
    }
    return 1;
}



/****************************************************************************/

