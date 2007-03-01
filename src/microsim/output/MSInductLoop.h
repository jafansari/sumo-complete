/****************************************************************************/
/// @file    MSInductLoop.h
/// @author  Christian Roessel
/// @date    2004-11-23
/// @version $Id$
///
// * @author Christian Roessel
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
#ifndef MSInductLoop_h
#define MSInductLoop_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <microsim/MSMoveReminder.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <utils/iodevices/XMLDevice.h>
#include <utils/common/Named.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class GUIDetectorWrapper;
class GUIGlObjectStorage;
class GUILaneWrapper;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * An unextended detector that measures at a fixed position on a fixed
 * lane. Only vehicles that passed the entire detector are counted. We
 * ignore vehicles that are emitted onto the detector and vehicles
 * that change their lane while they are on the detector, because we
 * cannot determine a meaningful enter/leave-times.
 *
 * This detector uses the MSMoveReminder mechanism, i.e. the vehicles
 * call the detector if they pass it.
 *
 * This detector inserts itself into a SingletonDictionary so that you
 * can access all MSInductLoop instances globally. Use this dictionary
 * to delete the detectors.
 *
 * See the get*-methods to learn about what is measured. You can vary
 * the sample-intervall during runtime.
 *
 * Due to the inheritance from MSDetectorFileOutput this detector can
 * print data to a file at fixed intervals via MSDetector2File.
 *
 * @see MSDetector2File
 * @see MSMoveReminder
 * @see SingletonDictionary
 * @see MSDetectorFileOutput
 */
class MSInductLoop
            : public MSMoveReminder,
            public MSDetectorFileOutput,
            public Named
{
public:
    /// Type of the dictionary where all MSInductLoop are registered.

    /**
     * Constructor. Adds object into a SingletonDictionary. Sets old-data
     * removal event. Adds reminder to MSLane.
     *
     * @param id Unique id.
     * @param lane Lane where detector woks on.
     * @param position Position of the detector within the lane.
     * @param deleteDataAfterSeconds Dismiss-time for collected data.
     */
    MSInductLoop(const std::string& id,
                 MSLane* lane,
                 SUMOReal positionInMeters,
                 SUMOTime deleteDataAfterSeconds);


    /// Destructor. Clears containers.
    ~MSInductLoop();


    /**
     * @name MSMoveReminder methods.
     *
     * Methods in this group are inherited from MSMoveReminder. They are
     * called by the moving, entering and leaving vehicles.
     *
     */
    //@{
    /**
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this
     * reminder from it's reminder-container. This method will
     * determine the entry- and leave-time of the counted vehicle and
     * pass this information to the methods enterDetectorByMove() and
     * eaveDetectorByMove().
     *
     * @param veh Vehicle that asks this remider.
     * @param oldPos Position before move.
     * @param newPos Position after move with newSpeed.
     * @param newSpeed Moving speed.
     *
     * @return True if vehicle hasn't passed the detector completely.
     *
     * @see enterDetectorByMove
     * @see leaveDetectorByMove
     */
    bool isStillActive(MSVehicle& veh,
                       SUMOReal oldPos,
                       SUMOReal newPos,
                       SUMOReal newSpeed);


    /**
     *  Informs corresponding detector via leaveDetectorByLaneChange()
     *  if vehicle leaves by lanechange.
     *
     * @param veh The leaving vehicle.
     *
     * @see leaveDetectorByLaneChange
     */
    void dismissByLaneChange(MSVehicle& veh);

    /**
     * Informs corresponding detector if vehicle enters the reminder
     * by emit or lanechange. Only vehicles that are completely in
     * front of the detector will return true.
     *
     * @param veh The entering vehilcle.
     *
     * @return True if vehicle is on or in front of the detector.
     */
    bool isActivatedByEmitOrLaneChange(MSVehicle& veh);
    //@}

    /**
     * @name Get-methods. These are the methods to access the
     * collected vehicle-data.
     *
     */
    //@{
    /**
     * Calculates the flow in [veh/h] over the given interval.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return Flow in [veh/h].
     */
    SUMOReal getFlow(SUMOTime lastNTimesteps) const;

    /**
     * Calculates the mean-speed in [m/s] over the given interval.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return Mean-speed in [m/s] averaged over the vehicles that
     * passed during the lastNTimesteps.
     */
    SUMOReal getMeanSpeed(SUMOTime lastNTimesteps) const;

    /**
     * Calculates the occupancy in [%], where 100% means lastNTimesteps.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return Occupancy in [%].
     */
    SUMOReal getOccupancy(SUMOTime lastNTimesteps) const;

    /**
     * Calculate the mean-vehicle-length in [m] averaged over the
     * vehicles that passed during the lastNTimesteps.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return Mean vehicle-length in [m] averaged over the vehicles
     * that passed during the lastNTimesteps.
     */
    SUMOReal getMeanVehicleLength(SUMOTime lastNTimesteps) const;

    /**
     * Counts the timesteps from the last leaving of the detector to
     * now. If the detector is currently occupied, we return 0.
     *
     * @return Timesteps from last leaving (detection) of the detector
     * to now or 0 if detector is occupied.
     */
    SUMOReal getTimestepsSinceLastDetection() const;

    /**
     * How many vehicles passed the detector completely over the
     * lastNTimesteps.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return Number of vehicles that passed the detector completely
     * over the lastNTimesteps.
     */
    SUMOReal getNVehContributed(SUMOTime lastNTimesteps) const;
    //@}

    /**
     * @name Inherited MSDetectorFileOutput methods.
     *
     * @see MSDetectorFileOutput
     */
    //@{
    /**
     * Get a header for file output via MSDetector2File
     *
     * @return The static XML-Header which explains the output of getXMLOutput.
     *
     * @see MSDetector2File
     * @see getXMLOutput
     */
    void writeXMLHeader(XMLDevice &dev) const;


    /**
     * Get the XML-formatted output of all the get*-methods except
     * getTimestepsSinceLastDetection.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return XML-formatted output of all the get*-methods except
     * getTimestepsSinceLastDetection.
     *
     * @see MSDetector2File
     */
    void writeXMLOutput(XMLDevice &dev,
                        SUMOTime startTime, SUMOTime stopTime);

    /**
     * Get an opening XML-element containing information about the detector.
     *
     * @return <detector type="inductionloop" id="det_id"
     * lane="lane_id" pos="det_pos">
     *
     * @see MSDetector2File
     * @see getXMLDetectorInfoEnd
     */
    void writeXMLDetectorInfoStart(XMLDevice &dev) const;


    /**
     * Get the closing XML-element to getXMLDetectorInfoStart
     *
     * @return </detector>
     *
     * @see MSDetector2File
     * @see getXMLDetectorInfoStart
     */
    void writeXMLDetectorInfoEnd(XMLDevice &dev) const;
    //@}

    /**
     * Struct to store the data of the counted vehicle
     * internally. These data is fed into a container on which the
     * get*-methods work.
     *
     * @see vehicleDataContM
     */
    struct VehicleData
    {
        /// Use this constructor if the vehicle has passed the induct loop completely
        VehicleData(MSVehicle& veh,
                    SUMOReal entryTimestep,
                    SUMOReal leaveTimestep)
                : lengthM(veh.getLength()),
                entryTimeM(MSNet::getSeconds(entryTimestep)),
                leaveTimeM(MSNet::getSeconds(leaveTimestep)),
                speedM(lengthM / (leaveTimeM - entryTimeM)),
                speedSquareM(speedM * speedM),
                occupancyM(leaveTimeM - entryTimeM)
        {}

        SUMOReal lengthM;         /**< Length of the vehicle. */
        SUMOReal entryTimeM;      /**< Entry-time of the vehicle in [s]. */
        SUMOReal leaveTimeM;      /**< Leave-time of the vehicle in [s]. */
        SUMOReal speedM;          /**< Speed of the vehicle in [m/s]. */
        SUMOReal speedSquareM;    /**< SpeedSquare of the vehicle in [m/(s^2)].*/
        SUMOReal occupancyM;      /**< Occupancy of the detector in [s]. */
    };


    typedef std::vector<SUMOReal> DismissedCont;

protected:
    /**
     * @name Methods called by Reminder methods.
     *
     * Methods in this group are called by the MSMoveReminder methods
     * only. They collect data to calculate the get* values.
     *
     *  @see isStillActive
     *  @see dismissByLaneChange
     *  @see isActivatedByEmitOrLaneChange
     */
    //@{
    /**
     * Introduces a vehicle to the detector's map vehiclesOnDetM.
     *
     * @param veh The entering vehicle.
     * @param entryTimestep Timestep (not neccessary integer) of entrance.
     */
    void enterDetectorByMove(MSVehicle& veh,
                             SUMOReal entryTimestep);

    /**
     * Removes a vehicle from the detector's map vehiclesOnDetM and
     * adds the vehicle data to the internal vehicleDataContM.
     *
     * @param veh The leaving vehicle.
     * @param leaveTimestep Timestep (not neccessary integer) of leaving.
     */
    void leaveDetectorByMove(MSVehicle& veh,
                             SUMOReal leaveTimestep);

    /**
     * Removes a vehicle from the detector's map vehiclesOnDetM.
     *
     * @param veh The leaving vehicle.
     */
    void leaveDetectorByLaneChange(MSVehicle& veh);
    //@}

    /**
     * Binary predicate that compares the passed VehicleData's
     * leaveTime to a fixed leaveTimeBound and returns true if the
     * passed value is lesser than the fixed bound. Used by
     * deleteOldData and getStartIterator.
     *
     */
struct leaveTimeLesser :
                public std::binary_function< VehicleData, SUMOReal, bool >
    {
        bool operator()(const VehicleData& firstVData,
                        const VehicleData& secondVData) const
        {
            return firstVData.leaveTimeM < secondVData.leaveTimeM;
        }
    };

    /**
     * Deletes data from vehicleDataContM if deleteDataAfterStepsM
     * is over. Is called via MSEventControl.
     *
     * @return deleteDataAfterStepsM to recur the event.
     *
     * @see MSEventControl
     */
    SUMOTime deleteOldData(SUMOTime currentTime);

    typedef std::deque< VehicleData > VehicleDataCont; /**< Type of
                                                            * vehicleDataContM. */

    /**
     * Get the iterator to vehicleDataContM that corresponds to the
     * first element with a leaveTime that is greater than now -
     * lastNTimesteps.
     *
     * @param lastNTimesteps Time-bound to search in is now - lastNTimesteps.
     *
     * @return Iterator to vehicleDataContM.
     */
    VehicleDataCont::const_iterator getStartIterator(
        SUMOTime lastNTimesteps) const;

    DismissedCont::const_iterator getDismissedStartIterator(
        SUMOTime lastNTimesteps) const;


    const SUMOReal posM;          /**< Detector's position on lane [cells]. */

    SUMOTime deleteDataAfterStepsM; /**< Deletion interval. */

    SUMOReal lastLeaveTimestepM;  /**< Leave-timestep of the last
                                     * vehicle detected. */

    typedef std::map< MSVehicle*, SUMOReal > VehicleMap; /**< Type of
                                                            * vehiclesOnDetM. */

    VehicleMap vehiclesOnDetM;  /**< Map that holds the vehicles that
                                     * are currently on the detector. */

    DismissedCont dismissedContM;

    /**
     * Container that hold the data of the counted vehicles. It is
     * used to calculate the get* values. After
     * deleteDataAfterSecondsM this container is cleared partially.
     */
    VehicleDataCont vehicleDataContM;

    static std::string xmlHeaderM; /**< Contains a XML-comment that
                                        * explain the printed data. */

    static std::string xmlDetectorInfoEndM; /**< Closing tag for detector. */

private:

    /// Hidden default constructor.
    MSInductLoop();

    /// Hidden copy constructor.
    MSInductLoop(const MSInductLoop&);

    /// Hidden assignment operator.
    MSInductLoop& operator=(const MSInductLoop&);
};

namespace
{

/**
 * @name Binary-functions to use with std::accumulate.
 *
 */
//@{
/// Adds up VehicleData::speedM
inline SUMOReal speedSum(SUMOReal sumSoFar,
                         const MSInductLoop::VehicleData& data)
{
    return sumSoFar + data.speedM;
}

/// Adds up VehicleData::speedSquareM
inline SUMOReal speedSquareSum(SUMOReal sumSoFar,
                               const MSInductLoop::VehicleData& data)
{
    return sumSoFar + data.speedSquareM;
}

/// Adds up VehicleData::occupancyM
inline SUMOReal occupancySum(SUMOReal sumSoFar,
                             const MSInductLoop::VehicleData& data)
{
    return sumSoFar + data.occupancyM;
}

/// Adds up VehicleData::lengthM
inline SUMOReal lengthSum(SUMOReal sumSoFar,
                          const MSInductLoop::VehicleData& data)
{
    return sumSoFar + data.lengthM;
}
//@}
}


#endif

/****************************************************************************/

