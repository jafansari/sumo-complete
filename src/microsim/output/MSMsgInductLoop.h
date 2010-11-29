/****************************************************************************/
/// @file    MSMsgInductLoop.h
/// @author  Clemens Honomichl
/// @date    2008-02-21
///
// An unextended detector measuring at a fixed position on a fixed lane.
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef MSMsgInductLoop_h
#define MSMsgInductLoop_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#ifdef _MESSAGES

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <microsim/MSMoveReminder.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <utils/common/Named.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class SUMOVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMsgInductLoop
 * @brief An unextended detector measuring at a fixed position on a fixed lane.
 *
 * Only vehicles that passed the entire detector are counted. We
 *  ignore vehicles that are emitted onto the detector and vehicles
 *  that change their lane while they are on the detector, because we
 *  cannot determine a meaningful enter/leave-times.
 *
 * This detector uses the MSMoveReminder mechanism, i.e. the vehicles
 *  call the detector if they pass it.
 *
 * Due to the inheritance from MSDetectorFileOutput this detector can
 *  print data to a file at fixed intervals via MSDetector2File.
 *
 * @see MSDetector2File
 * @see MSMoveReminder
 * @see MSDetectorFileOutput
 */
class MSMsgInductLoop
        : public MSMoveReminder, public MSDetectorFileOutput,
            public Named {
public:
    /**
     * @brief Constructor.
     *
     * Adds reminder to MSLane.
     *
     * @param id Unique id.
     * @param msg The message to write
     * @param lane Lane where detector woks on.
     * @param position Position of the detector within the lane.
     * @param deleteDataAfterSeconds Dismiss-time for collected data.
     */
    MSMsgInductLoop(const std::string& id, const std::string& msg,
                    MSLane* lane,
                    SUMOReal positionInMeters) throw();


    /// @brief Destructor
    ~MSMsgInductLoop() throw();


    /** @brief Resets all generated values to allow computation of next interval
     */
    void reset() throw();


    /// @name Methods inherited from MSMoveReminder.
    /// @{

    /** @brief Checks whether the vehicle shall be counted and/or shall still touch this MSMoveReminder
     *
     * As soon a vehicle enters the detector, its entry time is computed and stored
     *  in myVehiclesOnDet via enterDetectorByMove. If it passes the detector, the
     *  according leaving time is computed and stored, too, using leaveDetectorByMove.
     *
     * @param[in] veh Vehicle that asks this remider.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     * @return True if vehicle hasn't passed the detector completely.
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyMove
     * @see enterDetectorByMove
     * @see leaveDetectorByMove
     */
    bool notifyMove(SUMOVehicle& veh, SUMOReal oldPos, SUMOReal newPos, SUMOReal newSpeed) throw();


    /** @brief Dismisses the vehicle if it is on the detector due to a lane change
     *
     * If the vehicle is on the detector, it will be dismissed by incrementing
     *  myDismissedVehicleNumber and removing this vehicle's entering time from
     *  myVehiclesOnDet.
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @see leaveDetectorByLaneChange
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyLeave
     */
    bool notifyLeave(SUMOVehicle& veh, SUMOReal lastPos, MSMoveReminder::Notification reason) throw();


    /** @brief Returns whether the detector may has to be concerned during the vehicle's further movement
     *
     * If the detector is in front of the vehicle, true is returned. If
     *  the vehicle's front has passed the detector, false, because
     *  the vehicle is no longer relevant for the detector.
     *
     * @param veh The entering vehilcle.
     * @param[in] isEmit whether the vehicle was just emitted into the net
     * @param[in] isLaneChange whether the vehicle changed to the lane
     * @return True if vehicle is on or in front of the detector.
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyEnter
     */
    bool notifyEnter(SUMOVehicle& veh, MSMoveReminder::Notification reason) throw();
    //@}



    /// @name Methods returning current values
    /// @{

    /** @brief Returns the speed of the vehicle on the detector
     *
     * If no vehicle is on the detector, -1 is returned, otherwise
     *  this vehicle's current speed.
     *
     * @return The speed [m/s] of the vehicle if one is on the detector, -1 otherwise
     */
    SUMOReal getCurrentSpeed() const throw();


    /** @brief Returns the length of the vehicle on the detector
     *
     * If no vehicle is on the detector, -1 is returned, otherwise
     *  this vehicle's length.
     *
     * @return The length [m] of the vehicle if one is on the detector, -1 otherwise
     */
    SUMOReal getCurrentLength() const throw();


    /** @brief Returns the current occupancy
     *
     * If a vehicle is on the detector, 1 is returned. If a vehicle has passed the detector
     *  in this timestep, its occupancy value is returned. If no vehicle has passed,
     *  0 is returned.
     *
     * @return This detector's current occupancy
     * @todo recheck (especially if more than one vehicle has passed)
     */
    SUMOReal getCurrentOccupancy() const throw();


    /** @brief Returns the number of vehicles that have passed the detector
     *
     * If a vehicle is on the detector, 1 is returned. If a vehicle has passed the detector
     *  in this timestep, 1 is returned. If no vehicle has passed,
     *  0 is returned.
     *
     * @return The number of vehicles that have passed the detector
     * @todo recheck (especially if more than one vehicle has passed)
     */
    SUMOReal getCurrentPassedNumber() const throw();


    /** @brief Returns the time since the last vehicle left the detector
     *
     * @return Timesteps from last leaving (detection) of the detector
     */
    SUMOReal getTimestepsSinceLastDetection() const throw();
    //@}



    /// @name Methods returning aggregated values
    /// @{

    unsigned getNVehContributed() const throw();
    //@}



    /// @name Methods inherited from MSDetectorFileOutput.
    /// @{

    /** @brief Writes collected values into the given stream
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @see MSDetectorFileOutput::writeXMLOutput
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLOutput(OutputDevice &dev, SUMOTime startTime, SUMOTime stopTime) throw(IOError);


    /** @brief Opens the XML-output using "detector" as root element
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLDetectorProlog(OutputDevice &dev) const throw(IOError);
    /// @}


protected:
    /// @name Methods that add and remove vehicles from internal container
    /// @{

    /** @brief Introduces a vehicle to the detector's map myVehiclesOnDet.
     * @param veh The entering vehicle.
     * @param entryTimestep Timestep (not necessary integer) of entrance.
     */
    void enterDetectorByMove(SUMOVehicle& veh, SUMOReal entryTimestep) throw();


    /** @brief Processes a vehicle that leaves the detector
     *
     * Removes a vehicle from the detector's map myVehiclesOnDet and
     * adds the vehicle data to the internal myVehicleDataCont.
     *
     * @param veh The leaving vehicle.
     * @param leaveTimestep Timestep (not necessary integer) of leaving.
     */
    void leaveDetectorByMove(SUMOVehicle& veh, SUMOReal leaveTimestep) throw();


    /** @brief Removes a vehicle from the detector's map myVehiclesOnDet.
     * @param veh The leaving vehicle.
     */
    void leaveDetectorByLaneChange(SUMOVehicle& veh) throw();
    /// @}


protected:
    /** @brief Struct to store the data of the counted vehicle internally.
     *
     * These data is fed into a container.
     *
     * @see myVehicleDataCont
     */
    struct VehicleData {
        /** @brief Constructor
         *
         * Used if the vehicle has passed the induct loop completely
         *
         * @param[in] vehLength The length of the vehicle
         * @param[in] entryTimestep The time at which the vehicle entered the detector
         * @param[in] leaveTimestep The time at which the vehicle left the detector
         */
        VehicleData(SUMOReal vehLength, SUMOReal entryTimestep, SUMOReal leaveTimestep) throw()
                : lengthM(vehLength), entryTimeM(entryTimestep), leaveTimeM(leaveTimestep),
                speedM(lengthM / (leaveTimeM - entryTimeM)),
                occupancyM(leaveTimeM - entryTimeM) {}

        /** @brief Length of the vehicle. */
        SUMOReal lengthM;
        /** @brief Entry-time of the vehicle in [s]. */
        SUMOReal entryTimeM;
        /** @brief Leave-time of the vehicle in [s]. */
        SUMOReal leaveTimeM;
        /** @brief Speed of the vehicle in [m/s]. */
        SUMOReal speedM;
        /** @brief Occupancy of the detector in [s]. */
        SUMOReal occupancyM;
    };


protected:
    /// @name Function for summing up values
    ///@{

    /// @brief Adds up VehicleData::speedM
    static inline SUMOReal speedSum(SUMOReal sumSoFar, const MSMsgInductLoop::VehicleData& data) throw() {
        return sumSoFar + data.speedM;
    }

    /// @brief Adds up VehicleData::occupancyM
    static inline SUMOReal occupancySum(SUMOReal sumSoFar, const MSMsgInductLoop::VehicleData& data) throw() {
        return sumSoFar + data.occupancyM;
    }

    /// @brief Adds up VehicleData::lengthM
    static inline SUMOReal lengthSum(SUMOReal sumSoFar, const MSMsgInductLoop::VehicleData& data) throw() {
        return sumSoFar + data.lengthM;
    }
    ///@}


protected:
    /// @brief The vehicle that is currently on the detector
    SUMOVehicle *myCurrentVehicle;

    /// @brief Detector's position on lane [m]
    const SUMOReal myPosition;

    /// @brief Leave-time of the last vehicle detected [s]
    SUMOReal myLastLeaveTime;

    /// @brief Occupancy by the last vehicle detected.
    SUMOReal myLastOccupancy;

    /// @brief The number of dismissed vehicles
    unsigned myDismissedVehicleNumber;


    /// @brief Type of myVehicleDataCont.
    typedef std::deque< VehicleData > VehicleDataCont;

    /// @brief Data of vehicles that have completely passed the detector
    VehicleDataCont myVehicleDataCont;


    /// @brief Type of myVehiclesOnDet
    typedef std::map< SUMOVehicle*, SUMOReal > VehicleMap;

    /// @brief Data for vehicles that have entered the detector
    VehicleMap myVehiclesOnDet;

    /// @brief The message
    std::string myMsg;

    std::string myCurrentID;


private:
    /// @brief Invalidated copy constructor.
    MSMsgInductLoop(const MSMsgInductLoop&);

    /// @brief Invalidated assignment operator.
    MSMsgInductLoop& operator=(const MSMsgInductLoop&);


};
#endif //_MESSAGES

#endif

/****************************************************************************/

