/****************************************************************************/
/// @file    MSE2Collector.cpp
/// @author  Christian Roessel
/// @date    Tue Dec 02 2003 22:13 CET
/// @version $Id$
///
// An areal (along a single lane) detector
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

#include "MSE2Collector.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSE2Collector::MSE2Collector(const std::string &id, DetectorUsage usage,
                             MSLane* lane, SUMOReal startPos, SUMOReal detLength,
                             SUMOTime haltingTimeThreshold,
                             SUMOReal haltingSpeedThreshold,
                             SUMOReal jamDistThreshold) throw()
        : Named(id), MSMoveReminder(lane),
        myJamHaltingSpeedThreshold(haltingSpeedThreshold),
        myJamHaltingTimeThreshold(haltingTimeThreshold),
        myJamDistanceThreshold(jamDistThreshold),
        myStartPos(startPos),
        myEndPos(startPos + detLength),
        myUsage(usage),
        myCurrentOccupancy(0), myCurrentMeanSpeed(-1), myCurrentJamNo(0),
        myCurrentMaxJamLengthInMeters(0), myCurrentMaxJamLengthInVehicles(0),
        myCurrentJamLengthInMeters(0), myCurrentJamLengthInVehicles(0), myCurrentStartedHalts(0)

{
    assert(laneM != 0);
    assert(myStartPos >= 0 && myStartPos < laneM->length());
    assert(myEndPos - myStartPos > 0 && myEndPos <= laneM->length());
    reset();
}


MSE2Collector::~MSE2Collector() throw()
{
    for (std::list<MSVehicle*>::iterator i=myKnownVehicles.begin(); i!=myKnownVehicles.end(); ++i) {
        (*i)->quitRemindedLeft(this);
    }
    myKnownVehicles.clear();
}


bool
MSE2Collector::isStillActive(MSVehicle& veh, SUMOReal oldPos,
                             SUMOReal newPos, SUMOReal) throw()
{
    if (newPos <= myStartPos) {
        // detector not yet reached
        return true;
    }
    if (oldPos <= myStartPos && newPos > myStartPos) {
        if (find(myKnownVehicles.begin(), myKnownVehicles.end(), &veh)==myKnownVehicles.end()) {
            myKnownVehicles.push_back(&veh);
            veh.quitRemindedEntered(this);
        }
    }
    if (newPos - veh.getLength() > myEndPos) {
        veh.quitRemindedLeft(this);
        myKnownVehicles.erase(find(myKnownVehicles.begin(), myKnownVehicles.end(), &veh));
        return false;
    }
    return true;
}


void
MSE2Collector::dismissByLaneChange(MSVehicle& veh) throw()
{
    if (veh.getPositionOnLane() >= myStartPos && veh.getPositionOnLane() - veh.getLength() < myEndPos) {
        myKnownVehicles.erase(find(myKnownVehicles.begin(), myKnownVehicles.end(), &veh));
        veh.quitRemindedLeft(this);
    }
}


bool
MSE2Collector::isActivatedByEmitOrLaneChange(MSVehicle& veh) throw()
{
    if (veh.getPositionOnLane() >= myStartPos && veh.getPositionOnLane() - veh.getLength() < myEndPos) {
        // vehicle is on detector
        veh.quitRemindedEntered(this);
        myKnownVehicles.push_back(&veh);
        return true;
    }
    if (veh.getPositionOnLane() - veh.getLength() > myEndPos) {
        // vehicle is beyond detector
        return false;
    }
    // vehicle is in front of detector
    return true;
}


void
MSE2Collector::reset() throw()
{
    mySpeedSum = 0;
    myMaxHaltingDuration = 0;
    myIntervalMaxHaltingDuration = 0;
    myStartedHalts = 0;
    myJamLengthInMetersSum = 0;
    myJamLengthInVehiclesSum = 0;
    myVehicleSamples = 0;
    myOccupancySum = 0;
    myMaxOccupancy = 0;
    myHaltingDurationSum = 0;
    myIntervalHaltingDurationSum = 0;
    myMeanMaxJamInVehicles = 0;
    myMeanMaxJamInMeters = 0;
    myMaxJamInVehicles = 0;
    myMaxJamInMeters = 0;
    mySpeedSum = 0;
    myTimeSamples = 0;
    myMeanVehicleNumber = 0;
    myMaxVehicleNumber = 0;
    for (std::map<MSVehicle*, SUMOTime>::iterator i=myIntervalHaltingVehicleDurations.begin(); i!=myIntervalHaltingVehicleDurations.end(); ++i) {
        (*i).second = 0;
    }
}



void
MSE2Collector::update(SUMOTime) throw()
{
    JamInfo *currentJam = 0;
    std::map<MSVehicle*, SUMOTime> haltingVehicles;
    std::map<MSVehicle*, SUMOTime> intervalHaltingVehicles;
    std::vector<JamInfo*> jams;

    SUMOReal lengthSum = 0;
    SUMOTime haltingDurationSum = 0;
    SUMOTime intervalHaltingDurationSum = 0;
    myCurrentMeanSpeed = 0;
    myCurrentMeanLength = 0;
    myCurrentStartedHalts = 0;

    // go through the (sorted) list of vehicles positioned on the detector
    //  sum up values and prepare the list of jams
    myKnownVehicles.sort(by_vehicle_position_sorter(getLane()));
    for (std::list<MSVehicle*>::const_iterator i=myKnownVehicles.begin(); i!=myKnownVehicles.end(); ++i) {
        MSVehicle *veh = *i;

        SUMOReal length = veh->getLength();
        if (&(veh->getLane())==getLane()) {
            if (veh->getPositionOnLane() - veh->getLength() < myStartPos) {
                // vehicle entered detector partially
                length -= (veh->getLength() - (veh->getPositionOnLane()-myStartPos));
            }
            if (veh->getPositionOnLane()>myEndPos && veh->getPositionOnLane()-veh->getLength()<=myEndPos) {
                // vehicle left detector partially
                length -= (veh->getPositionOnLane()-myEndPos);
            }
        } else {
            // ok, the vehicle is only partially still on the detector, has already moved to the
            //  next lane; still, we do not know how far away it is
            assert(veh->getPositionOnActiveMoveReminderLane(getLane())>0);
            length -= (veh->getPositionOnActiveMoveReminderLane(getLane())-myEndPos);
        }
        assert(length>=0);

        mySpeedSum += veh->getSpeed();
        myCurrentMeanSpeed += veh->getSpeed();
        lengthSum += length;
        myCurrentMeanLength += length;

        // jam-checking begins
        bool isInJam = false;
        // first, check whether the vehicle is slow enough to be states as halting
        if (veh->getSpeed()<myJamHaltingSpeedThreshold) {
            // we have to track the time it was halting;
            //  so let's look up whether it was halting before and compute the overall halting time
            bool wasHalting = myHaltingVehicleDurations.find(veh)!=myHaltingVehicleDurations.end();
            if (wasHalting) {
                haltingVehicles[veh] = myHaltingVehicleDurations[veh] + 1;
                intervalHaltingVehicles[veh] = myIntervalHaltingVehicleDurations[veh] + 1;
            } else {
                haltingVehicles[veh] = 1;
                intervalHaltingVehicles[veh] = 1;
            }
            // we now check whether the halting time is large enough
            if (haltingVehicles[veh]>myJamHaltingTimeThreshold) {
                // yep --> the vehicle is a part of a jam
                isInJam = true;
                // save the halting duration for further statistics
                ++haltingDurationSum;
                ++intervalHaltingDurationSum;
                myMaxHaltingDuration = MAX2(myMaxHaltingDuration, haltingVehicles[veh]);
                myIntervalMaxHaltingDuration = MAX2(myIntervalMaxHaltingDuration, intervalHaltingVehicles[veh]);
                // now, we have to check whether it has started this time...
                if (haltingVehicles[veh]-1<=myJamHaltingTimeThreshold) {
                    myCurrentStartedHalts++;
                    myStartedHalts++;
                }
            }
        }

        // jam-building
        if (isInJam) {
            // the vehicle is in a jam;
            //  it may be a new one or already an existing one
            if (currentJam==0) {
                // the vehicle is the first vehicle in a jam
                currentJam = new JamInfo;
                currentJam->firstStandingVehicle = i;
            } else {
                // ok, we have a jam already. But - maybe it is too far away
                //  ... honestly, I can hardly find a reason for doing this,
                //  but jams were defined this way in an earlier version...
                if (veh->getPositionOnLane()-(*currentJam->lastStandingVehicle)->getPositionOnLane()>myJamDistanceThreshold) {
                    // yep, yep, yep - it's a new one...
                    //  close the frist, build a new
                    jams.push_back(currentJam);
                    currentJam = new JamInfo;
                    currentJam->firstStandingVehicle = i;
                }
            }
            currentJam->lastStandingVehicle = i;
        } else {
            // the vehicle is not part of a jam...
            //  maybe we have to close an already computed jam
            if (currentJam!=0) {
                jams.push_back(currentJam);
                currentJam = 0;
            }
        }
    }
    if (currentJam!=0) {
        jams.push_back(currentJam);
        currentJam = 0;
    }

    myCurrentMaxJamLengthInMeters = 0;
    myCurrentMaxJamLengthInVehicles = 0;
    myCurrentJamLengthInMeters = 0;
    myCurrentJamLengthInVehicles = 0;
    // process jam information
    for (std::vector<JamInfo*>::iterator i=jams.begin(); i!=jams.end(); ++i) {
        // compute current jam's values
        SUMOReal jamLengthInMeters =
            (*(*i)->firstStandingVehicle)->getPositionOnActiveMoveReminderLane(getLane())
            - (*(*i)->lastStandingVehicle)->getPositionOnActiveMoveReminderLane(getLane())
            + (*(*i)->lastStandingVehicle)->getLength();
        unsigned jamLengthInVehicles = (unsigned) distance((*i)->firstStandingVehicle, (*i)->lastStandingVehicle) + 1;
        // apply them to the statistics
        myCurrentMaxJamLengthInMeters = MAX2(myCurrentMaxJamLengthInMeters, jamLengthInMeters);
        myCurrentMaxJamLengthInVehicles = MAX2(myCurrentMaxJamLengthInVehicles, jamLengthInVehicles);
        myJamLengthInMetersSum += jamLengthInMeters;
        myJamLengthInVehiclesSum += jamLengthInVehicles;
        myCurrentJamLengthInMeters += jamLengthInMeters;
        myCurrentJamLengthInVehicles += jamLengthInVehicles;
    }
    myCurrentJamNo = (unsigned) jams.size();


    unsigned noVehicles = (unsigned) myKnownVehicles.size();
    myVehicleSamples += noVehicles;
    myTimeSamples += 1;
    // compute occupancy values
    SUMOReal currentOccupancy = lengthSum / (myEndPos-myStartPos) * (SUMOReal) 100.;
    myCurrentOccupancy = currentOccupancy;
    myOccupancySum += currentOccupancy;
    myMaxOccupancy = MAX2(myMaxOccupancy, currentOccupancy);
    // compute halting duration values
    myHaltingDurationSum += haltingDurationSum;
    myIntervalHaltingDurationSum += intervalHaltingDurationSum;
    // compute jam values
    myMeanMaxJamInVehicles += myCurrentMaxJamLengthInVehicles;
    myMeanMaxJamInMeters += myCurrentMaxJamLengthInMeters;
    myMaxJamInVehicles = MAX2(myMaxJamInVehicles, myCurrentMaxJamLengthInVehicles);
    myMaxJamInMeters = MAX2(myMaxJamInMeters, myCurrentMaxJamLengthInMeters);
    // save information about halting vehicles
    myHaltingVehicleDurations = haltingVehicles;
    myIntervalHaltingVehicleDurations = intervalHaltingVehicles;
    // compute information about vehicle numbers
    myMeanVehicleNumber += (unsigned) myKnownVehicles.size();
    myMaxVehicleNumber = MAX2((unsigned) myKnownVehicles.size(), myMaxVehicleNumber);
    // norm current values
    myCurrentMeanSpeed = noVehicles!=0 ? myCurrentMeanSpeed / (SUMOReal) noVehicles : -1;
    myCurrentMeanLength = noVehicles!=0 ? myCurrentMeanLength / (SUMOReal) noVehicles : -1;
}



void
MSE2Collector::writeXMLOutput(OutputDevice &dev, SUMOTime startTime, SUMOTime stopTime) throw(IOError)
{
    dev<<"   <interval begin=\""<<startTime<<"\" end=\""<< stopTime<<"\" "<<"id=\""<<getID()<<"\" ";
    SUMOReal meanSpeed = myVehicleSamples!=0 ? mySpeedSum / (SUMOReal) myVehicleSamples : -1;
    SUMOReal meanOccupancy = myTimeSamples!=0 ? myOccupancySum / (SUMOReal) myTimeSamples : 0;
    SUMOReal meanJamLengthInMeters = myTimeSamples!=0 ? myMeanMaxJamInMeters / (SUMOReal) myTimeSamples : 0;
    SUMOReal meanJamLengthInVehicles = myTimeSamples!=0 ? myMeanMaxJamInVehicles / (SUMOReal) myTimeSamples : 0;
    SUMOReal meanVehicleNumber = myTimeSamples!=0 ? (SUMOReal) myMeanVehicleNumber / (SUMOReal) myTimeSamples : 0;
    SUMOReal meanHaltingDuration = meanVehicleNumber!=0 ? myHaltingDurationSum / (SUMOReal) meanVehicleNumber : 0;
    SUMOReal meanIntervalHaltingDuration = meanVehicleNumber!=0 ? myIntervalHaltingDurationSum / (SUMOReal) meanVehicleNumber : 0;
    dev << "nSamples=\"" << myVehicleSamples << "\" "
    << "meanSpeed=\"" << meanSpeed << "\" "
    << "meanOccupancy=\"" << meanOccupancy << "\" "
    << "maxOccupancy=\"" << myMaxOccupancy << "\" "
    << "meanMaxJamLengthInVehicles=\"" << meanJamLengthInVehicles << "\" "
    << "meanMaxJamLengthInMeters=\"" << meanJamLengthInMeters << "\" "
    << "maxJamLengthInVehicles=\"" << myMaxJamInVehicles << "\" "
    << "maxJamLengthInMeters=\"" << myMaxJamInMeters << "\" "
    << "jamLengthInVehiclesSum=\"" << myJamLengthInVehiclesSum << "\" "
    << "jamLengthInMetersSum=\"" << myJamLengthInMetersSum << "\" "
    << "meanHaltingDuration=\"" << meanHaltingDuration << "\" "
    << "maxHaltingDuration=\"" << myMaxHaltingDuration << "\" "
    << "haltingDurationSum=\"" << myHaltingDurationSum << "\" "
    << "meanIntervalHaltingDuration=\"" << meanIntervalHaltingDuration << "\" "
    << "maxIntervalHaltingDuration=\"" << myIntervalMaxHaltingDuration << "\" "
    << "intervalHaltingDurationSum=\"" << myIntervalHaltingDurationSum << "\" "
    << "startedHalts=\"" << myStartedHalts << "\" "
    << "meanVehicleNumber=\"" << meanVehicleNumber << "\" "
    << "maxVehicleNumber=\"" << myMaxVehicleNumber << "\" "
    << "/>\n";
    reset();
}


void
MSE2Collector::writeXMLDetectorProlog(OutputDevice &dev) const throw(IOError)
{
    dev.writeXMLHeader("detector");
}


unsigned
MSE2Collector::getCurrentVehicleNumber() const throw()
{
    return (unsigned) myKnownVehicles.size();
}


SUMOReal
MSE2Collector::getCurrentOccupancy() const throw()
{
    return myCurrentOccupancy * (SUMOReal) 100.;
}


SUMOReal
MSE2Collector::getCurrentMeanSpeed() const throw()
{
    return myCurrentMeanSpeed;
}


SUMOReal
MSE2Collector::getCurrentMeanLength() const throw()
{
    return myCurrentMeanLength;
}


unsigned
MSE2Collector::getCurrentJamNumber() const throw()
{
    return myCurrentJamNo;
}


unsigned
MSE2Collector::getCurrentMaxJamLengthInVehicles() const throw()
{
    return myCurrentMaxJamLengthInVehicles;
}


SUMOReal
MSE2Collector::getCurrentMaxJamLengthInMeters() const throw()
{
    return myCurrentMaxJamLengthInMeters;
}


unsigned
MSE2Collector::getCurrentJamLengthInVehicles() const throw()
{
    return myCurrentJamLengthInVehicles;
}


SUMOReal
MSE2Collector::getCurrentJamLengthInMeters() const throw()
{
    return myCurrentJamLengthInMeters;
}


unsigned
MSE2Collector::getCurrentStartedHalts() const throw()
{
    return myCurrentStartedHalts;
}


void
MSE2Collector::removeOnTripEnd(MSVehicle *veh) throw()
{
    myKnownVehicles.erase(find(myKnownVehicles.begin(), myKnownVehicles.end(), veh));
}


/****************************************************************************/

