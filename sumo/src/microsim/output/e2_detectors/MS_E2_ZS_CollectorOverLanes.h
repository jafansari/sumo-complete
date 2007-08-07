/****************************************************************************/
/// @file    MS_E2_ZS_CollectorOverLanes.h
/// @author  Daniel Krajzewicz
/// @date    Oct 2003
/// @version $Id$
///
// A detector which joins E2Collectors over consecutive lanes (backward)
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
#ifndef MS_E2_ZS_CollectorOverLanes_h
#define MS_E2_ZS_CollectorOverLanes_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSE2Collector.h"
#include <utils/iodevices/OutputDevice.h>



class MSEdgeContinuations;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MS_E2_ZS_CollectorOverLanes
 * This class is somekind of a wrapper over several MSE2Collectors.
 * For some reasons it may be necessary to use MSE2Collectors that are
 *  longer than the lane they begin at. In this case, this class should be
 *  used. MSE2Collectors are laid on consecutive lanes backwards, building
 *  a virtual detector for each lane combination.
 * There are still some problems with it: we do not know how the different
 *  combinations shall be treated. Really verified is only the
 *  CURRENT_HALTING_DURATION_SUM_PER_VEHICLE-detector.
 */
class MS_E2_ZS_CollectorOverLanes :
            public MSDetectorFileOutput
{
public:
    /// Definition of a E2-collector storage
    typedef std::vector< MSE2Collector* > CollectorCont;

    /// Constructor
    MS_E2_ZS_CollectorOverLanes(std::string id,
                                DetectorUsage usage, MSLane* lane, SUMOReal startPos,
                                SUMOReal haltingTimeThreshold,
                                MSUnit::MetersPerSecond haltingSpeedThreshold,
                                SUMOReal jamDistThreshold,
                                SUMOTime deleteDataAfterSeconds);

    /** @brief Builds the consecutive E2-detectors
    	This is not done within the constructor to allow overriding of
    	most functions but the building of detectors itself which in fact
    	is depending on whether the normal or the gui-version is used */
    void init(MSLane *lane, SUMOReal detLength,
              const MSEdgeContinuations &edgeContinuations);

    /// Destructor
    virtual ~MS_E2_ZS_CollectorOverLanes(void);

    /// Returns this detector's current value for the measure of the given type
    SUMOReal getCurrent(E2::DetType type);

    /// Returns this detector's aggregated value for the given measure
    SUMOReal getAggregate(E2::DetType type, SUMOReal lastNSeconds);

    /// Returns the information whether the given type is computed
    bool hasDetector(E2::DetType type);

    /// Adds the measure of the given type
    void addDetector(E2::DetType type, std::string detId = "");

    /// Returns this detector's id
    const std::string &getID() const;

    /// Returns the id of the lane this detector starts at
    const std::string &getStartLaneID() const;

    /// ... have to override this method
    void resetQueueLengthAheadOfTrafficLights(void);

    /**
     * Get the XML-formatted output of the concrete detector.
     *
     * @param lastNTimesteps Generate data out of the interval
     * (now-lastNTimesteps, now].
     */
    void writeXMLOutput(OutputDevice &dev,
                        SUMOTime startTime, SUMOTime stopTime);

    /**
     * Get an opening XML-element containing information about the detector.
     */
    void writeXMLDetectorProlog(OutputDevice &dev) const;

    /// Returns this detector's length
    SUMOReal getLength() const
    {
        return myLength;
    }

protected:
    /** @brief This method extends the current length up to the given
    	This method is called consecutively until all paths have the
    	desired length */
    void extendTo(SUMOReal length,
                  const MSEdgeContinuations &edgeContinuations);

    /// Builds an id for one of the E2-collectors this detector uses
    std::string  makeID(const std::string &baseID,
                        size_t c, size_t r) const;

    /// Builds a single E2-collector
    virtual MSE2Collector *buildCollector(size_t c, size_t r,
                                          MSLane *l, SUMOReal start, SUMOReal end);


    std::vector<MSLane*> getLanePredeccessorLanes(MSLane *l,
            const MSEdgeContinuations &edgeContinuations);

protected:
    /// The position the collector starts at
    SUMOReal startPosM;

    /// The length of the collector
    SUMOReal myLength;

    /// The information for how many seconds data shall be saved
    SUMOTime deleteDataAfterSecondsM;

    /// Describes how long a vehicle shall stay before being assigned to a jam
    MSUnit::Steps haltingTimeThresholdM;

    /// Describes how slow a vehicle must be before being assigned to a jam
    MSUnit::CellsPerStep haltingSpeedThresholdM;

    /// Describes how long a jam must be before being recognized
    SUMOReal jamDistThresholdM;

    /// Definition of a lane storage
    typedef std::vector<MSLane*> LaneVector;

    /// Definition of a storage for lane vectors
    typedef std::vector<LaneVector> LaneVectorVector;

    /// Definition of a detector storage
    typedef std::vector<MSE2Collector*> DetectorVector;

    /// Definition of astorage for detector vectors
    typedef std::vector<DetectorVector> DetectorVectorVector;

    /// Definition of a SUMOReal storage
    typedef std::vector<SUMOReal> DoubleVector;

    /// Definition of a storage for SUMOReal vectors
    typedef DoubleVector LengthVector;

    /** @brief Storage for lane combinations
    	Each lane combination is a vector of consecutive lanes (backwards) */
    LaneVectorVector myLaneCombinations;

    /** @brief Storage for detector combinations
    	Each detector combination is a vector of consecutive lanes (backwards) */
    DetectorVectorVector myDetectorCombinations;
    /** @brief Storage for length combinations
    	Each length combination is a vector of consecutive lanes (backwards) */
    LengthVector myLengths;

    /// The id of this detector
    std::string myID;

    /// The id of the lane this detector starts at
    std::string myStartLaneID;

    /// Definition of a map from a lane to the detector lying on it
    typedef std::map<MSLane*, MSE2Collector*> LaneDetMap;

    /// Storage for detectors which already have been build for a single lane
    LaneDetMap myAlreadyBuild;

    DetectorUsage myUsage;

};


#endif

/****************************************************************************/

