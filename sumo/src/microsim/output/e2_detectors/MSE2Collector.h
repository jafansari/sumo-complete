/****************************************************************************/
/// @file    MSE2Collector.h
/// @author  Christian Roessel
/// @date    Tue Dec 02 2003 22:13 CET
/// @version $Id$
///
//	�missingDescription�
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
#ifndef MSE2Collector_h
#define MSE2Collector_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <microsim/MSMoveReminder.h>
#include <microsim/output/MSDetectorTypedefs.h>
#include <microsim/MSUnit.h>
#include <microsim/MSLane.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/output/MSDetectorContainerWrapperBase.h>
#include <microsim/output/MSDetectorContainerWrapper.h>
#include <microsim/output/MSDetectorHaltingContainerWrapper.h>
#include <microsim/output/MSDetectorCounterContainerWrapper.h>
#include <microsim/output/MSDetectorOccupancyCorrection.h>
#include <utils/common/ToString.h>
#include <string>
#include <cassert>
#include <vector>
#include <limits>
#include <microsim/output/MSApproachingVehiclesStates.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/Named.h>

///
/// Introduces some enums and consts for use in MSE2Collector.
///
namespace E2
{
///
/// Collection of all possible E2-detectors. Names should be
/// self-explanatory. Keep the detectors sorted in the order TD
/// (timestep-data), ED (event-data), LD (leave-data).
///
enum DetType { DENSITY = 0, // TD
               MAX_JAM_LENGTH_IN_VEHICLES,
               MAX_JAM_LENGTH_IN_METERS,
               JAM_LENGTH_SUM_IN_VEHICLES,
               JAM_LENGTH_SUM_IN_METERS,
               QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES,
               QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_METERS,
               N_VEHICLES,
               OCCUPANCY_DEGREE,
               SPACE_MEAN_SPEED,
               CURRENT_HALTING_DURATION_SUM_PER_VEHICLE,
               N_STARTED_HALTS, // ED
               HALTING_DURATION_SUM, // ED
               HALTING_DURATION_MEAN, // LD
               APPROACHING_VEHICLES_STATES, // special treatment
               ALL          ///< Use this to generate all possible
               ///detectors in
               ///MSE2Collector::addDetector().
             };

/// @name Markers, where the typ of detection (TD, ED, LD) changes.
/// @{
const DetType lastTD = CURRENT_HALTING_DURATION_SUM_PER_VEHICLE;
const DetType lastED = HALTING_DURATION_SUM;
const DetType lastLD = HALTING_DURATION_MEAN;
/// @}

/// Increment operator that allows us to iterate over the
/// E2::DetType detectors.
///
/// @param det A detector out of E2::DetType (an integral type).
///
/// @return The detector that follows det.
///
DetType& operator++(DetType& det);

///
/// Collection of different "vehicle" containers used by MSE2Collector.
///
enum Containers
{
    COUNTER = 0,
    VEHICLES,
    HALTINGS
};

/// Increment operator that allows us to iterate over the
/// E2::Containers detectors.
///
/// @param cont A container out of E2::Containers (an integral type).
///
/// @return The container that follows cont.
///
Containers& operator++(Containers& cont);
}

/// Collection of headover-detectors. This class defines the place and
/// size of the detection-area. You can add detectors via
/// addDetector() out of E2::DetType to work on the detection
/// area. The detectors are updated every timestep. You can retrieve
/// their current detection-value via getCurrent() and a sampled value
/// via getAggregate(). As MSE2Collector inherits from
/// MSDetectorFileOutput there is the possibility to get file output
/// by calling MSDetector2File::addDetectorAndInterval().
///
class
            MSE2Collector : public Named,
            public MSMoveReminder,
            public MSDetectorFileOutput
{
public:

    typedef TD::MSDetectorInterface TDDetector;
    typedef std::vector< TDDetector* > DetTDCont;
    typedef DetTDCont::iterator DetTDContIter;

    typedef ED::MSDetectorInterface EDDetector;
    typedef std::vector< EDDetector* > DetEDCont;
    typedef DetEDCont::iterator DetEDContIter;

    typedef LD::MSDetectorInterface LDDetector;
    typedef std::vector< LDDetector* > DetLDCont;
    typedef DetLDCont::iterator DetLDContIter;

    typedef std::vector< MSDetectorContainerWrapperBase* > ContainerCont;
    typedef ContainerCont::iterator ContainerContIter;

    /// Ctor. Checks position-params and inserts itself to an E2Dictionary for
    /// global access to all MSE2Collector objects.
    ///
    /// @param id The detector's unique id.
    /// @param usage ???
    /// @param lane The lane on which the detetcor works.
    /// @param startPos Start-position on that lane. Assure that it is within
    /// the lane.
    /// @param detLength Length of the detector. Assure that startPos +
    /// detLength is within the lane.
    /// @param haltingTimeThreshold A vehicle must at least drive that time
    /// at a speed lesser than haltingSpeedThreshold to be a "halting" vehicle.
    /// @param haltingSpeedThreshold A vehicle must not drive a greater speed
    /// for at more than haltingTimeThreshold  to be a "halting" vehicle.
    /// @param jamDistThreshold Two or more vehicles are "in jam" if they are
    /// halting and their distance is lesser than jamDistThreshold.
    /// @param deleteDataAfterSeconds Delete the data that is collected every
    /// timestep so that data is always present for at least
    /// deleteDataAfterSeconds.
    MSE2Collector(
        const std::string &id,
        DetectorUsage usage,
        MSLane* lane, SUMOReal startPos, SUMOReal detLength,
        MSUnit::Seconds haltingTimeThreshold,
        MSUnit::MetersPerSecond haltingSpeedThreshold,
        SUMOReal jamDistThreshold,
        SUMOTime deleteDataAfterSeconds)
            : Named(id), MSMoveReminder(lane),
            startPosM(startPos),
            endPosM(startPos + detLength),
            deleteDataAfterSecondsM(deleteDataAfterSeconds),
            haltingTimeThresholdM(MSUnit::getInstance()->getSteps(
                                      haltingTimeThreshold)),
            haltingSpeedThresholdM(MSUnit::getInstance()->getCellsPerStep(
                                       haltingSpeedThreshold)),
            jamDistThresholdM(jamDistThreshold),
            detectorsTDM(11),
            detectorsEDM(2),
            detectorsLDM(1),
            containersM(3),
            occupancyCorrectionM(),
            approachingVehStatesDetectorM(0),
            myUsage(usage)
    {
        assert(laneM != 0);
        SUMOReal laneLength = laneM->length();
        assert(startPosM >= 0 &&
               startPosM < laneLength);
        assert(endPosM - startPosM > 0 && endPosM < laneLength);
        /*
                    // insert object into dictionary
                    if ( ! E2Dictionary::getInstance()->isInsertSuccess(
                             idM, this ) ) {
                        MsgHandler::getErrorInstance()->inform(
                            "e2-detector '" + idM + "' could not be build;");
                        MsgHandler::getErrorInstance()->inform(
                            " (declared twice?)");
                        throw ProcessError();            }*/
    }

    /// Dtor. Deletes the created detectors.
    virtual ~MSE2Collector(void)
    {
        deleteContainer(detectorsTDM);
        deleteContainer(detectorsEDM);
        deleteContainer(detectorsLDM);
        deleteContainer(containersM);
        if (approachingVehStatesDetectorM != 0) {
            delete approachingVehStatesDetectorM;
        }
    }

    /// ???
    virtual DetectorUsage getUsageType() const
    {
        return myUsage;
    }

    /// Get the current state of the E2::APPROACHING_VEHICLES_STATES detector.
    ///
    /// @param nApproachingVeh Number of vehicles/states you are interested in.
    ///
    /// @return A MSApproachingVehiclesStates::DetectorAggregate container
    /// of size nApproachingVeh.
    ///
    const MSApproachingVehiclesStates::DetectorAggregate
    getCurrentApproachingStates(unsigned nApproachingVeh)
    {
        if (approachingVehStatesDetectorM == 0) {
            addDetector(E2::APPROACHING_VEHICLES_STATES);
        }
        return approachingVehStatesDetectorM->getDetectorAggregate(
                   nApproachingVeh);
    }

    /// Get the current value of a TDDetector. If the detector doesn't
    /// exist, create it and return -1 for this first call.
    ///
    /// @param type The detector <= E2::lastTD out of E2::DetType you
    /// are interested in.
    ///
    /// @return If the detector exists, return it's current value,
    /// else return -1.
    ///
    SUMOReal getCurrent(E2::DetType type)
    {
        assert(type <= E2::lastTD);

        TDDetector* det = static_cast< TDDetector* >(getDetector(type));
        if (det != 0) {
            return det->getCurrent();
        }
        // requested type not present
        // create it and return nonsens value for the first access
        addDetector(type, "");
        return -1;
    }

    /// Get the aggregated value of a TDDetector, EDDetector or
    /// LDDetector. If the detector doesn't exist, create it and
    /// return -1 for this first call.
    ///
    /// @param type The detector <= E2::lastLD out of E2::DetType you
    /// are interested in.
    /// @param lastNSeconds Length of the aggregation intervall
    /// (now-lastNSeconds, now].
    ///
    /// @return If the detector exists, return it's aggregated value,
    /// else return -1.
    ///
    SUMOReal getAggregate(E2::DetType type, MSUnit::Seconds lastNSeconds)
    {
        assert(type <= E2::lastLD);

        MSDetectorInterfaceCommon* det = getDetector(type);
        if (det != 0) {
            return det->getAggregate(lastNSeconds);
        }

        // requested type not present
        // create it and return nonsens value for the first access
        addDetector(type, "");
        return -1;
    }

    /// Checks if the requested detector is present.
    ///
    /// @param type The detector out of E2::DetType you are interested in.
    ///
    /// @return True if the detector exists, false otherwise.
    ///
    bool hasDetector(E2::DetType type) const
    {
        assert(type < E2::ALL);
        return getDetector(type) != 0;
    }

    /// Add, i.e. create the requested detector or all detectors out
    /// of E2::DetType.
    ///
    /// @param type One detetcor out of E2::DetType. ALL will create
    /// all detectors.
    /// @param detId Optional id of the newly created detector(s).
    ///
    void addDetector(E2::DetType type, std::string detId = "")
    {
        if (detId == "") {
            detId = getID();
        }
        if (type != E2::ALL) {
            createDetector(type, detId);
        } else {
            for (E2::DetType typ = E2::DENSITY; typ < E2::ALL; ++typ) {
                createDetector(typ, detId);
            }
        }
    }

    /// The detectors E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_*
    /// have monoton growing current-values for use with
    /// traffic-light-controls. These current-values have to be resetted
    /// from time to time. This is done here.
    ///
    void resetQueueLengthAheadOfTrafficLights(void)
    {
        using namespace Detector;
        MSQueueLengthAheadOfTrafficLightsInVehicles* det1 = 0;
        if ((det1 = static_cast<
                    E2QueueLengthAheadOfTrafficLightsInVehicles* >(
                        detectorsTDM[
                            E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES ]))) {
            det1->resetMax();
        }
        MSQueueLengthAheadOfTrafficLightsInMeters* det2 = 0;
        if ((det2 = static_cast<
                    E2QueueLengthAheadOfTrafficLightsInMeters* >(
                        detectorsTDM[
                            E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_METERS ]))) {
            det2->resetMax();
        }
    }

    ///
    /// @name Methods, inherited from MSMoveReminder.
    /// They are called by the moving, entering and leaving vehicles.
    /// @{

    /// Indicator, if a vehicle has passed the detector entirely. In
    /// this case, the vehicle removes this MSMoveReminder from it's
    /// MSMoveReminder-container.
    ///
    /// @param veh The vehicle in question.
    /// @param oldPos Position before the move-micro-timestep.
    /// @param newPos Position after the move-micro-timestep.
    /// @param SUMOReal Unused here.
    ///
    /// @return False, if vehicle passed the detector entierly, else true.
    ///
    bool isStillActive(MSVehicle& veh,
                       SUMOReal oldPos,
                       SUMOReal newPos,
                       SUMOReal)
    {
        if (newPos <= startPosM) {
            // detector not yet reached
            return true;
        }
        if (oldPos <= startPosM && newPos > startPosM) {
            // vehicle will enter detectors
            for (ContainerContIter it = containersM.begin();
                    it != containersM.end(); ++it) {
                if (*it != 0) {
                    (*it)->enterDetectorByMove(&veh);
                }
            }
        }
        if (newPos - veh.getLength() < startPosM) {
            // vehicle entered detector partially
            occupancyCorrectionM.setOccupancyEntryCorrection(
                veh, (newPos - startPosM) / veh.getLength());
        }
        if (newPos > endPosM && newPos - veh.getLength() <= endPosM) {
            // vehicle left detector partially
            occupancyCorrectionM.setOccupancyLeaveCorrection(
                veh, (endPosM - (newPos - veh.getLength())) /
                veh.getLength());
        }
        if (newPos - veh.getLength() > endPosM) {
            // vehicle will leave detector
            for (ContainerContIter it = containersM.begin();
                    it != containersM.end(); ++it) {
                if (*it != 0) {
                    (*it)->leaveDetectorByMove(&veh);
                }
            }
            for (DetLDContIter ld = detectorsLDM.begin();
                    ld != detectorsLDM.end(); ++ld) {
                if (*ld != 0) {
                    (*ld)->leave(veh);
                }
            }
            return false;
        }
        return true;
    }

    /// If the vehicle leaves the detection-area by lane-change, it
    /// may need to be removed from some internal containers. This is
    /// done here.
    ///
    /// @param veh The leaving vehicle.
    ///
    void dismissByLaneChange(MSVehicle& veh)
    {
        if (veh.getPositionOnLane() >= startPosM && veh.getPositionOnLane() - veh.getLength() < endPosM) {
            // vehicle is on detector
            for (ContainerContIter it = containersM.begin();
                    it != containersM.end(); ++it) {
                if (*it != 0) {
                    (*it)->leaveDetectorByLaneChange(&veh);
                }
            }
            if (veh.getPositionOnLane() - veh.getLength() < startPosM ||
                    veh.getPositionOnLane()>endPosM && veh.getPositionOnLane()-veh.getLength()<=endPosM) {
                // vehicle partially on det
                occupancyCorrectionM.dismissOccupancyCorrection(veh);
            }
            for (DetLDContIter ld = detectorsLDM.begin();
                    ld != detectorsLDM.end(); ++ld) {
                if (*ld != 0) {
                    (*ld)->leave(veh);
                }
            }
        }
    }

    /// If the vehicle enters the detection-area by emit or
    /// lane-change, it may need to be introduced into some internal
    /// containers. This is done here. Dependent on the position this
    /// MSMoveReminder is intoduced to the vehicles
    /// MSMoveReminder-container.
    ///
    /// @param veh The entering vehicle.
    ///
    /// @return False if vehicle is entirely beyond the detector, else true.
    ///
    bool isActivatedByEmitOrLaneChange(MSVehicle& veh)
    {
        if (veh.getPositionOnLane() >= startPosM && veh.getPositionOnLane() - veh.getLength() < endPosM) {
            // vehicle is on detector
            for (ContainerContIter it = containersM.begin();
                    it != containersM.end(); ++it) {
                if (*it != 0) {
                    (*it)->enterDetectorByEmitOrLaneChange(&veh);
                }
            }
            if (veh.getPositionOnLane() - veh.getLength() < startPosM) {
                // vehicle entered detector partially
                occupancyCorrectionM.setOccupancyEntryCorrection(
                    veh, (veh.getPositionOnLane() - startPosM) / veh.getLength());
            }
            if (veh.getPositionOnLane()>endPosM && veh.getPositionOnLane()-veh.getLength()<=endPosM) {
                // vehicle left detector partially
                occupancyCorrectionM.setOccupancyLeaveCorrection(
                    veh, (endPosM - (veh.getPositionOnLane() - veh.getLength())) /
                    veh.getLength());
            }
            return true;
        }
        if (veh.getPositionOnLane() - veh.getLength() > endPosM) {
            // vehicle is beyond detector
            return false;
        }
        // vehicle is in front of detector
        return true;
    }
    /// @}

    /// Get a header for file output which shall contain some
    /// explanation of the output generated by getXMLOutput.
    ///
    /// @return The member xmlHeaderM.
    ///
    void writeXMLHeader(OutputDevice &dev) const
    {
        dev.writeString(xmlHeaderM);
    }

    /// Get the XML-formatted output of the concrete detector.  Calls
    /// resetQueueLengthAheadOfTrafficLights() if the detector
    /// E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES exists.
    ///
    /// @param lastNTimesteps Generate data out of the interval
    /// (now-lastNTimesteps, now].
    ///
    /// @return XML-output of all existing concrete detectors. Except
    /// APPROACHING_VEHICLES_STATES.
    ///
    void writeXMLOutput(OutputDevice &dev, SUMOTime startTime, SUMOTime stopTime)
    {
        dev.writeString("   <interval begin=\"").writeString(
            toString(startTime)).writeString("\" end=\"").writeString(
                toString(stopTime)).writeString("\" ");
        if (dev.needsDetectorName()) {
            dev.writeString("id=\"").writeString(getID()).writeString("\" ");
        }
        if (hasDetector(
                    E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES)) {
            dev.writeString("queueLengthAheadOfTrafficLightsInVehiclesMax=\"");
            dev.writeString(
                toString(getCurrent(E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES)));
            dev.writeString("\" ");
            resetQueueLengthAheadOfTrafficLights();
        }
        writeXMLOutput(dev, detectorsTDM, startTime, stopTime);
        writeXMLOutput(dev, detectorsEDM, startTime, stopTime);
        writeXMLOutput(dev, detectorsLDM, startTime, stopTime);
        dev.writeString("/>");
    }

    /// Get an opening XML-tag containing information about the detector.
    ///
    /// @return String describing the detetctor-collection.
    ///
    void writeXMLDetectorInfoStart(OutputDevice &dev) const
    {
        dev.writeString("<detector>\n");
    }

    /// Get an closing XML-tag corresponding to the opening tag from
    /// getXMLDetectorInfoStart().
    ///
    /// @return The member infoEndM.
    ///
    void writeXMLDetectorInfoEnd(OutputDevice &dev) const
    {
        dev.writeString(infoEndM);
    }

    /// @}

    /// Get the start-position of the detector-collection on it's lane.
    ///
    /// @return The menber startPosM [meter].
    ///
    SUMOReal getStartPos() const
    {
        return startPosM;
    }

    /// Get the end-position of the detector-collection on it's lane.
    ///
    /// @return The menber endPosM [meter].
    ///
    SUMOReal getEndPos() const
    {
        return endPosM;
    }

protected:

    /// Get a pointer to the requested detector out of E2::DetType.
    ///
    /// @param type The detector you are interested in.
    ///
    /// @return If the detector exists, a pointer to the valid object,
    /// 0 otherwise.
    ///
    MSDetectorInterfaceCommon* getDetector(E2::DetType type) const
    {
        assert(type <= E2::lastLD);

        if (type <= E2::lastTD) {
            return detectorsTDM[ type ];
        }
        if (type <= E2::lastED) {
            return detectorsEDM[ type - E2::lastTD - 1 ];
        } else {
            return detectorsLDM[ type - E2::lastED - 1 ];
        }
    }

    /// Get the index of the requested detector regarding the adequate
    /// detector-container detectorsTDM, detectorsEDM or detectorsLDM.
    ///
    /// @param type The detector you are interested in.
    ///
    /// @return The index of the detector.
    ///
    E2::DetType getIndex(E2::DetType type) const
    {
        assert(type <= E2::lastLD);

        if (type <= E2::lastTD) {
            return type;
        }
        if (type <= E2::lastED) {
            return E2::DetType(type - E2::lastTD - 1);
        } else {
            return E2::DetType(type - E2::lastED - 1);
        }
    }

private:
    SUMOReal startPosM;   ///< Start position.
    SUMOReal endPosM;     ///< End position.

    SUMOTime deleteDataAfterSecondsM; ///< Data removal interval.
    MSUnit::Steps haltingTimeThresholdM; ///< Time-theshold to
    ///determine if a vehicle is
    ///halting.
    MSUnit::CellsPerStep haltingSpeedThresholdM; ///< Speed-theshold
    ///to determine if a
    ///vehicle is
    ///halting.
    SUMOReal jamDistThresholdM; ///< Distance-theshold to
    ///determine if two halting
    ///vehicles are "in jam".

    DetTDCont detectorsTDM;     ///< Container of TDDetectors.
    DetEDCont detectorsEDM;     ///< Container of EDDetectors.
    DetLDCont detectorsLDM;     ///< Container of LDDetectors.

    ContainerCont containersM;  ///< Container of helper-ccontainers.

    /// Object that corrects the detector values if vehicles are only
    /// partially on the detector.
    MSDetectorOccupancyCorrection occupancyCorrectionM;

    /// MSApproachingVehiclesStates detector. This one can't be
    /// handled in the way the TD, ED and LD detectors can.
    MSApproachingVehiclesStates* approachingVehStatesDetectorM;

    static std::string xmlHeaderM; ///< Header for the XML-output.
    static std::string infoEndM; ///< Closing detector tag.
    DetectorUsage myUsage;      ///< ???

    /// Create a "vehicle"-container out of E2::Containers. They may
    /// be shared among several detectors. The created container is
    /// put into the member containersM. A container holds pointers to
    /// all "vehicles" currently on the detector.
    ///
    /// @param type The container you are interested in.
    ///
    void createContainer(E2::Containers type)
    {
        if (containersM[ type ] != 0) {
            return;
        }
        switch (type) {
        case E2::COUNTER: {
            containersM[ E2::COUNTER ] =
                new DetectorContainer::Count(occupancyCorrectionM);
            break;
        }
        case E2::VEHICLES: {
            containersM[ E2::VEHICLES ] =
                new DetectorContainer::VehiclesList(
                    occupancyCorrectionM);
            break;
        }
        case E2::HALTINGS: {
            containersM[ E2::HALTINGS ] =
                new DetectorContainer::HaltingsList(
                    *laneM,
                    occupancyCorrectionM,
                    haltingTimeThresholdM,
                    haltingSpeedThresholdM,
                    jamDistThresholdM);
            break;
        }
        default: {
            assert(0);
        }
        }
    }

    /// Create a detector out of E2::DetType. The created detector
    /// (except E2::APPROACHING_VEHICLES_STATES) is put into one of
    /// the containers detectorsTDM, detectorsEDM or detectorsLDM. On
    /// creation, the detector gets one of the "vehicle"-containers
    /// out of E2::Containers.
    ///
    /// @param type The detector you are interested in.
    /// @param detId Detector-id, need not be unique.
    ///
    void createDetector(E2::DetType type, std::string detId)
    {
        if (type == E2::APPROACHING_VEHICLES_STATES) {
            if (approachingVehStatesDetectorM != 0) {
                return;
            }
        } else if (hasDetector(type)) {
            return;
// ===========================================================================
// used namespaces
// ===========================================================================
        }
        using namespace Detector;
        switch (type) {
        case E2::DENSITY: {
            createContainer(E2::COUNTER);
            detectorsTDM[ E2::DENSITY ] =
                new E2Density(
                    E2Density::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::Count* >(
                        containersM[ E2::COUNTER ]));
            break;
        }
        case E2::MAX_JAM_LENGTH_IN_VEHICLES: {
            createContainer(E2::HALTINGS);
            detectorsTDM[ E2::MAX_JAM_LENGTH_IN_VEHICLES ] =
                new E2MaxJamLengthInVehicles(
                    E2MaxJamLengthInVehicles::getDetectorName() +detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        case E2::MAX_JAM_LENGTH_IN_METERS: {
            createContainer(E2::HALTINGS);
            detectorsTDM[ E2::MAX_JAM_LENGTH_IN_METERS ] =
                new E2MaxJamLengthInMeters(
                    E2MaxJamLengthInMeters::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        case E2::JAM_LENGTH_SUM_IN_VEHICLES: {
            createContainer(E2::HALTINGS);
            detectorsTDM[ E2::JAM_LENGTH_SUM_IN_VEHICLES ] =
                new E2JamLengthSumInVehicles(
                    E2JamLengthSumInVehicles::getDetectorName()+detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        case E2::JAM_LENGTH_SUM_IN_METERS: {
            createContainer(E2::HALTINGS);
            detectorsTDM[ E2::JAM_LENGTH_SUM_IN_METERS ] =
                new E2JamLengthSumInMeters(
                    E2JamLengthSumInMeters::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));

            break;
        }
        case E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES: {
            if (detectorsTDM[ E2::MAX_JAM_LENGTH_IN_VEHICLES ] == 0) {
                createDetector(E2::MAX_JAM_LENGTH_IN_VEHICLES, detId);
            }
            detectorsTDM[
                E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_VEHICLES ] =
                    new E2QueueLengthAheadOfTrafficLightsInVehicles(
                        E2QueueLengthAheadOfTrafficLightsInVehicles::getDetectorName() + detId,
                        (SUMOReal)(endPosM - startPosM),
                        (SUMOReal) deleteDataAfterSecondsM,
                        *detectorsTDM[ E2::MAX_JAM_LENGTH_IN_VEHICLES ]);
            break;
        }
        case E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_METERS: {
            if (detectorsTDM[ E2::MAX_JAM_LENGTH_IN_METERS ] == 0) {
                createDetector(E2::MAX_JAM_LENGTH_IN_METERS, detId);
            }
            detectorsTDM[
                E2::QUEUE_LENGTH_AHEAD_OF_TRAFFIC_LIGHTS_IN_METERS ] =
                    new E2QueueLengthAheadOfTrafficLightsInMeters(
                        E2QueueLengthAheadOfTrafficLightsInMeters::getDetectorName() + detId,
                        (SUMOReal)(endPosM - startPosM),
                        (SUMOReal) deleteDataAfterSecondsM,
                        *detectorsTDM[ E2::MAX_JAM_LENGTH_IN_METERS ]);
            break;
        }
        case E2::N_VEHICLES: {
            createContainer(E2::COUNTER);
            detectorsTDM[ E2::N_VEHICLES ] =
                new E2NVehicles(
                    E2NVehicles::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::Count* >(
                        containersM[ E2::COUNTER ]));
            break;
        }
        case E2::OCCUPANCY_DEGREE: {
            createContainer(E2::VEHICLES);
            detectorsTDM[ E2::OCCUPANCY_DEGREE ] =
                new E2OccupancyDegree(
                    E2OccupancyDegree::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::VehiclesList* >(
                        containersM[ E2::VEHICLES ]));
            break;
        }
        case E2::SPACE_MEAN_SPEED: {
            createContainer(E2::VEHICLES);
            detectorsTDM[ E2::SPACE_MEAN_SPEED ] =
                new E2SpaceMeanSpeed(
                    E2SpaceMeanSpeed::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::VehiclesList* >(
                        containersM[ E2::VEHICLES ]));
            break;
        }
        case E2::CURRENT_HALTING_DURATION_SUM_PER_VEHICLE: {
            createContainer(E2::HALTINGS);
            detectorsTDM[ E2::CURRENT_HALTING_DURATION_SUM_PER_VEHICLE] =
                new E2CurrentHaltingDurationSumPerVehicle(
                    E2CurrentHaltingDurationSumPerVehicle::getDetectorName() + detId,
                    (SUMOReal)(endPosM - startPosM),
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        case E2::APPROACHING_VEHICLES_STATES: {
            createContainer(E2::VEHICLES);
            approachingVehStatesDetectorM = new MSApproachingVehiclesStates(
                                                endPosM,
                                                *static_cast< DetectorContainer::VehiclesList* >(
                                                    containersM[ E2::VEHICLES ]));
            break;
        }
        case E2::N_STARTED_HALTS: {
            createContainer(E2::HALTINGS);
            detectorsEDM[ getIndex(E2::N_STARTED_HALTS)] =
                new E2NStartedHalts(
                    E2NStartedHalts::getDetectorName() + detId,
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        case E2::HALTING_DURATION_SUM: {
            /* !!!
                createContainer( E2::HALTINGS );
                detectorsEDM[ getIndex( E2::HALTING_DURATION_SUM ) ] =
                    new E2NStartedHalts(
                        E2NStartedHalts::getDetectorName() + detId,
                        deleteDataAfterSecondsM,
                        *static_cast< DetectorContainer::HaltingsList* >(
                            containersM[ E2::HALTINGS ] ) );
            */
            break;
        }
        case E2::HALTING_DURATION_MEAN: {
            createContainer(E2::HALTINGS);
            detectorsLDM[ getIndex(E2::HALTING_DURATION_MEAN)] =
                new E2HaltingDurationMean(
                    E2HaltingDurationMean::getDetectorName() + detId,
                    (SUMOReal) deleteDataAfterSecondsM,
                    *static_cast< DetectorContainer::HaltingsList* >(
                        containersM[ E2::HALTINGS ]));
            break;
        }
        default: {
            assert(0);
        }
        }
    }

    /// Get aggregated XML-output for all detectors in a container.
    ///
    /// @param container A container holding detectors.
    /// @param lastNTimesteps The length of the aggregation interval.
    ///
    /// @return A XML-formatted string.
    ///
    template< class Cont >
    void writeXMLOutput(OutputDevice &dev, Cont& container,
                        SUMOTime startTime, SUMOTime stopTime)
    {
        MSUnit::Seconds lastNSeconds =
            (MSUnit::Steps)(stopTime-startTime+1);
        size_t i = 0;
        for (typename Cont::iterator it = container.begin();
                it != container.end(); ++it, ++i) {

            if (*it == 0) {
                continue;
            }
            dev.writeString((*it)->getName()).writeString("=\"").writeString(
                toString((*it)->getAggregate(lastNSeconds))).writeString(
                    "\" ");
        }
    }

    /// Deletes the elements of a container.
    ///
    /// @param cont The container whose elements shall be deleted.
    ///
    template< class Cont >
    void deleteContainer(Cont& cont)
    {
        for (typename Cont::iterator it = cont.begin();
                it != cont.end(); ++it) {
            if (*it != 0) {
                delete *it;
            }
        }
    }
};


#endif

/****************************************************************************/

