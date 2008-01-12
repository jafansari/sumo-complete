/****************************************************************************/
/// @file    MSInductLoop.cpp
/// @author  Christian Roessel
/// @date    2004-11-23
/// @version $Id$
///
// An unextended detector measuring at a fixed position on a fixed lane. 
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

#include "MSInductLoop.h"
#include <cassert>
#include <numeric>
#include <utility>
#include <utils/common/WrappingCommand.h>
#include <utils/common/ToString.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSLane.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
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
MSInductLoop::MSInductLoop(const string& id,
                           MSLane* lane,
                           SUMOReal positionInMeters)
        : MSMoveReminder(lane), Named(id), myCurrentVehicle(0),
        myPosition(positionInMeters), myLastLeaveTimestep(0),
        myVehiclesOnDet(), myVehicleDataCont()
{
    assert(myPosition >= 0 && myPosition <= laneM->length());
    reset();
    myLastLeaveTimestep = (SUMOReal) MSNet::getInstance()->getCurrentTimeStep();
}


MSInductLoop::~MSInductLoop()
{
    if(myCurrentVehicle!=0) {
        myCurrentVehicle->quitRemindedLeft(this);
    }
    myCurrentVehicle = 0;
}


void 
MSInductLoop::reset()
{
    myDismissedVehicleNumber = 0;
    myVehicleDataCont.clear();
}


bool
MSInductLoop::isStillActive(MSVehicle& veh, SUMOReal oldPos,
                            SUMOReal newPos, SUMOReal newSpeed)
{
    if (newPos < myPosition) {
        // detector not reached yet
        return true;
    }
    if (myVehiclesOnDet.find(&veh) == myVehiclesOnDet.end()) {
        // entered the detector by move
        SUMOReal entryTimestep = (SUMOReal) 
            ((SUMOReal) MSNet::getInstance()->getCurrentTimeStep() + ((myPosition - oldPos) / newSpeed));
        if (newPos - veh.getLength() > myPosition) {
            // entered and passed detector in a single timestep
            SUMOReal leaveTimestep = (SUMOReal) 
                ((SUMOReal) MSNet::getInstance()->getCurrentTimeStep() + ((myPosition - oldPos + veh.getLength()) / newSpeed));
            enterDetectorByMove(veh, entryTimestep);
            leaveDetectorByMove(veh, leaveTimestep);
            return false;
        }
        // entered detector, but not passed
        enterDetectorByMove(veh, entryTimestep);
        return true;
    } else {
        // vehicle has been on the detector the previous timestep
        if (newPos - veh.getLength() >= myPosition) {
            // vehicle passed the detector
            SUMOReal leaveTimestep = (SUMOReal) 
                ((SUMOReal) MSNet::getInstance()->getCurrentTimeStep() + ((myPosition - oldPos + veh.getLength()) / newSpeed));
            leaveDetectorByMove(veh, leaveTimestep);
            return false;
        }
        // vehicle stays on the detector
        return true;
    }
}


void
MSInductLoop::dismissByLaneChange(MSVehicle& veh)
{
    if (veh.getPositionOnLane() > myPosition && veh.getPositionOnLane() - veh.getLength() <= myPosition) {
        // vehicle is on detector during lane change
        leaveDetectorByLaneChange(veh);
    }
}


bool
MSInductLoop::isActivatedByEmitOrLaneChange(MSVehicle& veh)
{
    if (veh.getPositionOnLane() - veh.getLength() > myPosition) {
        // vehicle-front is beyond detector. Ignore
        return false;
    }
    // vehicle is in front of detector
    return true;
}


SUMOReal
MSInductLoop::getCurrentSpeed() const
{
    if(myCurrentVehicle!=0) {
        return myCurrentVehicle->getSpeed();
    }
    return -1;
}


SUMOReal
MSInductLoop::getCurrentLength() const
{
    if(myCurrentVehicle!=0) {
        return myCurrentVehicle->getLength();
    }
    return -1;
}


SUMOReal
MSInductLoop::getCurrentOccupancy() const
{
    if(myCurrentVehicle!=0) {
        return 1.;
    }
    if(myLastLeaveTimestep>MSNet::getInstance()->getCurrentTimeStep()-DELTA_T) {
        return 0.;
    }
    return myLastOccupancy;
}


SUMOReal
MSInductLoop::getCurrentPassedNumber() const
{
    if(myCurrentVehicle!=0) {
        return 1.;
    }
    if(myLastLeaveTimestep>MSNet::getInstance()->getCurrentTimeStep()-DELTA_T) {
        return 0.;
    }
    return 1.;
}


unsigned 
MSInductLoop::getNVehContributed() const
{
    return myVehicleDataCont.size();
}


SUMOReal
MSInductLoop::getTimestepsSinceLastDetection() const
{
    if (myVehiclesOnDet.size() != 0) {
        // detector is occupied
        return 0;
    }
    return MSNet::getInstance()->getCurrentTimeStep() - myLastLeaveTimestep;
}


void
MSInductLoop::writeXMLDetectorProlog(OutputDevice &dev) const
{
    dev.writeXMLHeader("detector");
}


void
MSInductLoop::writeXMLOutput(OutputDevice &dev,
                             SUMOTime startTime, SUMOTime stopTime)
{
    SUMOTime t(stopTime-startTime+1);
    unsigned nVehCrossed = myVehicleDataCont.size() + myDismissedVehicleNumber;
    SUMOReal flow = ((SUMOReal) myVehicleDataCont.size() / (SUMOReal) t) / DELTA_T * (SUMOReal) 3600.0;
    SUMOReal occupancy = accumulate(myVehicleDataCont.begin(), myVehicleDataCont.end(), (SUMOReal) 0.0, occupancySum) / (SUMOReal) t;
    SUMOReal meanSpeed = myVehicleDataCont.size()!=0
        ? accumulate(myVehicleDataCont.begin(), myVehicleDataCont.end(), (SUMOReal) 0.0, speedSum) / (SUMOReal) myVehicleDataCont.size()
        : -1;
    SUMOReal meanLength = myVehicleDataCont.size()!=0
        ? accumulate(myVehicleDataCont.begin(), myVehicleDataCont.end(), (SUMOReal) 0.0, lengthSum) / (SUMOReal) myVehicleDataCont.size()
        : -1;
    dev<<"   <interval begin=\""<<startTime<<"\" end=\""<<
    stopTime<<"\" "<<"id=\""<<getID()<<"\" ";
    dev<<"nVehContrib=\""<<myVehicleDataCont.size()<<"\" flow=\""<<flow<<
    "\" occupancy=\""<<occupancy<<"\" speed=\""<<meanSpeed<<
    "\" length=\""<<meanLength<<
    "\" nVehCrossed=\""<<nVehCrossed<<"\"/>\n";
    reset();
}


void
MSInductLoop::enterDetectorByMove(MSVehicle& veh,
                                  SUMOReal entryTimestep)
{
    myVehiclesOnDet.insert(make_pair(&veh, entryTimestep));
    veh.quitRemindedEntered(this);
    myCurrentVehicle = &veh;
}


void
MSInductLoop::leaveDetectorByMove(MSVehicle& veh,
                                  SUMOReal leaveTimestep)
{
    VehicleMap::iterator it = myVehiclesOnDet.find(&veh);
    assert(it != myVehiclesOnDet.end());
    SUMOReal entryTimestep = it->second;
    myVehiclesOnDet.erase(it);
    assert(entryTimestep < leaveTimestep);
    myVehicleDataCont.push_back(VehicleData(veh.getLength(), entryTimestep, leaveTimestep));
    myLastOccupancy = leaveTimestep - entryTimestep;
    myLastLeaveTimestep = leaveTimestep;
    myCurrentVehicle = 0;
    veh.quitRemindedLeft(this);
}


void
MSInductLoop::leaveDetectorByLaneChange(MSVehicle& veh)
{
    // Discard entry data
    myVehiclesOnDet.erase(&veh);
    myDismissedVehicleNumber++;
    myCurrentVehicle = 0;
    veh.quitRemindedLeft(this);
}


void 
MSInductLoop::removeOnTripEnd(MSVehicle *veh)
{
    myCurrentVehicle = 0;
    myVehiclesOnDet.erase(veh);
}


/****************************************************************************/

