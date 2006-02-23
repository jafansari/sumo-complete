//---------------------------------------------------------------------------//
//                        MSActuatedTrafficLightLogic.cpp -
//  The basic traffic light logic
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
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
// Revision 1.12  2006/02/23 11:27:57  dkrajzew
// tls may have now several programs
//
// Revision 1.11  2006/01/09 11:55:19  dkrajzew
// lanestates removed
//
// Revision 1.10  2005/11/09 06:36:48  dkrajzew
// changing the LSA-API: MSEdgeContinuation added; changed the calling API
//
// Revision 1.9  2005/10/10 11:56:09  dkrajzew
// reworking the tls-API: made tls-control non-static; made net an element of traffic lights
//
// Revision 1.8  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/22 13:45:52  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.6  2005/09/15 11:09:53  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/05/04 08:22:18  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.4  2005/02/01 10:10:46  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.3  2005/01/27 14:22:44  dkrajzew
// ability to open the complete phase definition added; code style adapted
//
// Revision 1.2  2004/12/10 11:43:57  dksumo
// parametrisation of actuated traffic lights added
//
// Revision 1.1  2004/10/22 12:49:44  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.27  2004/07/02 09:53:58  dkrajzew
// some design things
//
// Revision 1.26  2004/04/02 11:38:28  dkrajzew
// extended traffic lights are now no longer template classes
//
// Revision 1.25  2004/01/26 07:32:46  dkrajzew
// added the possibility to specify the position (actuated-tlls) / length
//  (agentbased-tlls) of used detectors
//
// Revision 1.24  2004/01/12 15:04:16  dkrajzew
// more wise definition of lane predeccessors implemented
//
// Revision 1.23  2003/11/24 10:21:20  dkrajzew
// some documentation added and dead code removed
//
// Revision 1.22  2003/11/17 07:18:21  dkrajzew
// e2-detector over lanes merger added
//
// Revision 1.21  2003/11/11 08:36:21  dkrajzew
// removed some debug-variables
//
// Revision 1.20  2003/10/08 07:09:16  dkrajzew
// gcc did not knew a const_iterator to a map (?)
//
// Revision 1.19  2003/10/01 13:59:53  dkrajzew
// logic building completed (Julia Ringel)
//
// Revision 1.18  2003/09/25 09:01:49  dkrajzew
// ambigous naming of detectors changed
//
// Revision 1.17  2003/09/24 09:55:11  dkrajzew
// bug on duplictae induct loop ids patched
//
// Revision 1.16  2003/09/23 14:19:59  dkrajzew
// an easier usage of the current actuated phase definition added
//
// Revision 1.15  2003/09/22 12:31:06  dkrajzew
// actuated traffic lights are now derived from simple traffic lights
//
// Revision 1.14  2003/07/21 18:13:05  roessel
// Changes due to new MSInductLoop.
//
// Revision 1.13  2003/07/18 12:35:04  dkrajzew
// removed some warnings
//
// Revision 1.12  2003/06/06 10:39:16  dkrajzew
// new usage of MSEventControl applied
//
// Revision 1.11  2003/06/05 16:01:28  dkrajzew
// MSTLLogicControl added
//
// Revision 1.9  2003/05/27 18:47:35  roessel
// Changed call to MSLaneState ctor.
//
// Revision 1.8  2003/05/22 09:20:49  roessel
// Renamed method call numberOfWaiting to getCurrentNumberOfWaiting.
//
// Revision 1.7  2003/05/21 16:20:44  dkrajzew
// further work detectors
//
// Revision 1.6  2003/05/21 15:15:41  dkrajzew
// yellow lights implemented (vehicle movements debugged
//
// Revision 1.5  2003/04/04 15:26:55  roessel
// Added the keyword "typename" for derived types in for-loops
//
// Revision 1.4  2003/04/02 11:44:02  dkrajzew
// continuation of implementation of actuated traffic lights
//
// Revision 1.2  2003/02/07 10:41:50  dkrajzew
// updated
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

#include <utility>
#include <vector>
#include <bitset>
#include <microsim/MSEventControl.h>
#include <microsim/output/MSInductLoop.h>
#include <microsim/MSNet.h>
#include "MSTrafficLightLogic.h"
#include "MSActuatedTrafficLightLogic.h"
#include <microsim/MSLane.h>
#include <netload/NLDetectorBuilder.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * method definitions
 * ======================================================================= */
MSActuatedTrafficLightLogic::MSActuatedTrafficLightLogic(
            MSNet &net, MSTLLogicControl &tlcontrol,
            const std::string &id, const std::string &subid,
            const Phases &phases,
            size_t step, size_t delay, SUMOReal maxGap, SUMOReal passingTime,
            SUMOReal detectorGap)
    : MSExtendedTrafficLightLogic(net, tlcontrol, id, subid, phases, step, delay),
    _continue(false),
    myMaxGap(maxGap), myPassingTime(passingTime), myDetectorGap(detectorGap)
{
}


void
MSActuatedTrafficLightLogic::init(NLDetectorBuilder &nb,
        const std::vector<MSLane*> &lanes,
        const MSEdgeContinuations &edgeContinuations,
        SUMOReal det_offset)
{
    // change values for setting the loops and lanestate-detectors, here
    SUMOTime inductLoopInterval = 1; //
    // as the laneStateDetector shall end at the end of the lane, the position
    // is calculated, not given
    SUMOTime laneStateDetectorInterval = 1; //

    std::vector<MSLane*>::const_iterator i;
    // build the induct loops
    for(i=lanes.begin(); i!=lanes.end(); i++) {
        MSLane *lane = (*i);
        SUMOReal length = lane->length();
        SUMOReal speed = lane->maxSpeed();
        SUMOReal inductLoopPosition = myDetectorGap * speed;
        // check whether the lane is long enough
        SUMOReal ilpos = length - inductLoopPosition;
        if(ilpos<0) {
            ilpos = 0;
        }
        // Build the induct loop and set it into the container
        std::string id = "TLS" + myID + "_" + mySubID + "_InductLoopOn_" + lane->id();
        if(myInductLoops.find(lane)==myInductLoops.end()) {
            myInductLoops[lane] =
                nb.createInductLoop(id, lane, ilpos, inductLoopInterval);
        }
    }
    // build the lane state-detectors
    for(i=lanes.begin(); i!=lanes.end(); i++) {
        MSLane *lane = (*i);
        SUMOReal length = lane->length();
        // check whether the position is o.k. (not longer than the lane)
        SUMOReal lslen = det_offset;
        if(lslen>length) {
            lslen = length;
        }
        SUMOReal lspos = length - lslen;
        // Build the lane state detetcor and set it into the container
        std::string id = "TLS" + myID + "_" + mySubID + "_LaneStateOff_" + lane->id();
            /*!!!!

        if(myLaneStates.find(lane)==myLaneStates.end()) {
            MSLaneState* loop =
                new MSLaneState( id, lane, lspos, lslen,
                    laneStateDetectorInterval );
            myLaneStates[lane] = loop;
        }
            */
    }
}



MSActuatedTrafficLightLogic::~MSActuatedTrafficLightLogic()
{
    {
        for(InductLoopMap::iterator i=myInductLoops.begin(); i!=myInductLoops.end(); ++i) {
            delete (*i).second;
        }
    }
    /*!!!!
    {
        for(LaneStateMap::iterator i=myLaneStates.begin(); i!=myLaneStates.end(); ++i) {
            delete (*i).second;
        }
    }
    */
}


SUMOTime
MSActuatedTrafficLightLogic::duration() const
{
    if(_continue) {
        return 1;
    }
    assert(myPhases.size()>myStep);
    if(!isGreenPhase()) {
        return currentPhaseDef()->duration;
    }
    // define the duration depending from the number of waiting vehicles of the actual phase
    int newduration = currentPhaseDef()->minDuration;
    const std::bitset<64> &isgreen = currentPhaseDef()->getDriveMask();
    for (size_t i=0; i<isgreen.size(); i++)  {
        if(isgreen.test(i))  {
            const std::vector<MSLane*> &lanes = getLanesAt(i);
            if (lanes.empty())    {
                break;
            }
            for (LaneVector::const_iterator j=lanes.begin(); j!=lanes.end();j++) {
                InductLoopMap::const_iterator k = myInductLoops.find(*j);
                SUMOReal waiting =  (SUMOReal) (*k).second->getNVehContributed(0/* !!! */);
                SUMOReal tmpdur =  myPassingTime * waiting;
                if (tmpdur > newduration) {
                    // here we cut the decimal places, because we have to return an integer
                    newduration = (int) tmpdur;
                }
                if (newduration > (int) currentPhaseDef()->maxDuration)  {
                    return currentPhaseDef()->maxDuration;
                }

                /*!!!!
                LaneStateMap::const_iterator k = myLaneStates.find(*j);
                SUMOReal waiting =  (SUMOReal) (*k).second->getCurrentNumberOfWaiting();
                SUMOReal tmpdur =  myPassingTime * waiting;
                if (tmpdur > newduration) {
                    // here we cut the decimal places, because we have to return an integer
                    newduration = (int) tmpdur;
                }
                if (newduration > (int) currentPhaseDef()->maxDuration)  {
                    return currentPhaseDef()->maxDuration;
                }
                */
            }
        }
    }
//    currentPhaseDef()->duration = newduration;
    return newduration;
}


SUMOTime
MSActuatedTrafficLightLogic::trySwitch(bool)
{
    // checks if the actual phase should be continued
    gapControl();
    if(_continue) {
        return duration();
    }
    // increment the index to the current phase
    myStep++;
    assert(myStep<=myPhases.size());
    if(myStep==myPhases.size()) {
        myStep = 0;
    }
    //stores the time the phase started
    static_cast<MSActuatedPhaseDefinition*>(myPhases[myStep])->_lastSwitch =
        MSNet::getInstance()->getCurrentTimeStep();
    // set the next event
    return duration();
}


bool
MSActuatedTrafficLightLogic::isGreenPhase() const
{
    if (currentPhaseDef()->getDriveMask().none()) {
        return false;
    }
    if (currentPhaseDef()->getYellowMask().any()) {
        return false;
    }
    return true;
}


void
MSActuatedTrafficLightLogic::gapControl()
{
    //intergreen times should not be lenghtend
    assert(myPhases.size()>myStep);
    if(!isGreenPhase()) {
        _continue = false;
        return;
    }

    // Checks, if the maxDuration is kept. No phase should longer send than maxDuration.
    SUMOTime actDuration =
        MSNet::getInstance()->getCurrentTimeStep() - static_cast<MSActuatedPhaseDefinition*>(myPhases[myStep])->_lastSwitch;
    if (actDuration >= currentPhaseDef()->maxDuration) {
        _continue = false;
        return;
    }

    // now the gapcontrol starts
    const std::bitset<64> &isgreen = currentPhaseDef()->getDriveMask();
    for (size_t i=0; i<isgreen.size(); i++)  {
        if(isgreen.test(i))  {
            const std::vector<MSLane*> &lanes = getLanesAt(i);
            if (lanes.empty())    {
                break;
            }
            for (LaneVector::const_iterator j=lanes.begin(); j!=lanes.end(); j++) {
                if(myInductLoops.find(*j)==myInductLoops.end()) {
                    continue;
                }
                SUMOReal actualGap =
                    myInductLoops.find(*j)->second->getTimestepsSinceLastDetection();
                if (actualGap < myMaxGap) {
                    _continue = true;
                    return;
                }
            }
        }
    }
    _continue = false;
}


MSActuatedPhaseDefinition *
MSActuatedTrafficLightLogic::currentPhaseDef() const
{
    assert(myPhases.size()>myStep);
    return static_cast<MSActuatedPhaseDefinition*>(myPhases[myStep]);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


