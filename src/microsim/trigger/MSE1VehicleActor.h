/****************************************************************************/
/// @file    MSE1VehicleActor.h
/// @author  Daniel Krajzewicz
/// @date    23.03.2006
/// @version $Id$
///
// An actor which changes a vehicle's state
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
#ifndef MSE1VehicleActor_h
#define MSE1VehicleActor_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <microsim/MSMoveReminder.h>
#include <microsim/trigger/MSTrigger.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/devices/MSDevice_CPhone.h>


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class MSE1VehicleActor
 */
class MSLane;
class GUIDetectorWrapper;
class GUIGlObjectStorage;
class GUILaneWrapper;

enum ActorType {
    LA,
    CELL
};

extern std::map<MSVehicle *, MSPhoneCell*> LastCells;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 */
class MSE1VehicleActor : public MSMoveReminder, public MSTrigger
{
public:
    /// Type of the dictionary where all MSE1VehicleActor are registered.

    /**
     * Constructor. Adds object into a SingletonDictionary. Sets old-data
     * removal event. Adds reminder to MSLane.
     *
     * @param id Unique id.
     * @param lane Lane where detector woks on.
     * @param position Position of the detector within the lane.
     */
    MSE1VehicleActor(const std::string& id, MSLane* lane,
                     SUMOReal positionInMeters, unsigned int laid, unsigned int cellid, unsigned int type) throw();


    /// Destructor. Clears containers.
    ~MSE1VehicleActor() throw();


    /// @name Methods inherited from MSMoveReminder
    /// @{

    /**
     *
     * @param[in, changed] veh Vehicle that asks this remider.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     * @return True if vehicle hasn't passed the detector completely.
     * @see MSMoveReminder
     * @see MSMoveReminder::isStillActive
     * @see enterDetectorByMove
     * @see leaveDetectorByMove
     */
    bool isStillActive(MSVehicle& veh,
                       SUMOReal oldPos,
                       SUMOReal newPos,
                       SUMOReal newSpeed) throw();


    /** @brief Nothing is done here
     *
     * @param[in] veh The leaving vehicle.
     * @see MSMoveReminder::dismissByLaneChange
     */
    void dismissByLaneChange(MSVehicle& veh) throw();


    /** @brief Returns whether the detector may has to be concerned during the vehicle's further movement
     *
     * If the detector is in front of the vehicle, true is returned. If
     *  the vehicle's front has passed the detector, false, because
     *  the vehicle is no longer relevant for the detector.
     *
     * @param[in] veh The entering vehicle.
     * @param[in] isEmit true means emit, false: lane change
     * @return True if vehicle is on or in front of the detector.
     * @see MSMoveReminder
     * @see MSMoveReminder::isActivatedByEmitOrLaneChange
     */
    bool isActivatedByEmitOrLaneChange(MSVehicle& veh, bool isEmit) throw();
    /// @}



    /// Returns the number of vehicles that have passed this actor
    unsigned int getPassedVehicleNumber() const {
        return myPassedVehicleNo;
    }

    /// Returns the number of mobile phones that have passed this actor
    unsigned int getPassedCPhoneNumber() const {
        return myPassedCPhonesNo;
    }

    /// Returns the number of mobile phones that have passed this actor being in connected mode
    unsigned int getPassedConnectedCPhoneNumber() const {
        return myPassedConnectedCPhonesNo;
    }

protected:
    const SUMOReal posM;          /**< Detector's position on lane [cells]. */
    //const ActorType myType;
    unsigned int myLAId;
    unsigned int myAreaId;
    unsigned int myActorType;

    /// The number of vehicles that have passed this actor
    unsigned int myPassedVehicleNo;

    /// The number of mobile phones that have passed this actor
    unsigned int myPassedCPhonesNo;

    /// The number of mobile phones that have passed this actor being in connected mode
    unsigned int myPassedConnectedCPhonesNo;

    bool percentOfActivity;


private:
    /// @brief Invalidated copy constructor.
    MSE1VehicleActor(const MSE1VehicleActor&);

    /// @brief Invalidated assignment operator.
    MSE1VehicleActor& operator=(const MSE1VehicleActor&);


};


#endif

/****************************************************************************/

