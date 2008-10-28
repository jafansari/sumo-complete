/****************************************************************************/
/// @file    MSMoveReminder.h
/// @author  Christian Roessel
/// @date    2003-05-21
/// @version $Id$
///
// Something on a lane to be noticed about vehicle movement
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
#ifndef MSMoveReminder_h
#define MSMoveReminder_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMoveReminder
 * @brief Something on a lane to be noticed about vehicle movement
 *
 * Base class of all move-reminders. During move, the vehicles call
 *  isStillActive() for all reminders on their current lane (all lanes
 *  they pass during one step). If a vehicle enters the lane the reminder is
 *  positioned at during emit or lanechange isActivatedByEmitOrLaneChange() is
 *  called. If a vehicle leaves the reminder lane by lanechange it calls
 *  dismissByLaneChange().
 *
 * The reminder knows whom to tell about move, emit and lanechange. The
 * vehicles will remove the reminder that is not isStillActive() from
 * their reminder container.
 *
 * @see MSLane::addMoveReminder
 * @see MSLane::getMoveReminder
 */
class MSMoveReminder
{
public:
    /** @brief Constructor.
     *
     * @param[in] lane Lane on which the reminder will work.
     * @todo Why is the lane not given as a reference?
     */
    MSMoveReminder(MSLane* lane) throw();


    /** @brief Destructor
     */
    virtual ~MSMoveReminder(void) throw() {}


    /** @brief Checks whether the reminder still has to be notified aboutthe vehicle moves
     *
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this reminder
     * from it's reminder-container.
     *
     * @param veh Vehicle that asks this remider.
     * @param oldPos Position before move.
     * @param newPos Position after move with newSpeed.
     * @param newSpeed Moving speed.
     *
     * @return True if vehicle hasn't passed the reminder completely.
     */
    virtual bool isStillActive(MSVehicle& veh,
                               SUMOReal oldPos,
                               SUMOReal newPos,
                               SUMOReal newSpeed) throw() = 0;


    /** @brief Called if the vehicle leaves the reminder's lane
     *
     *  Informs if vehicle leaves reminder by lanechange.
     *
     * @param veh The leaving vehicle.
     */
    virtual void dismissByLaneChange(MSVehicle& veh) throw() = 0;


    /** @brief Checks whether the reminder is activated by the vehicle's emission on lane change
     *
     * Lane change means in this case that the vehicle changes to the lane
     *  the reminder is placed at.
     *
     * @param veh The entering vehilcle.
     *
     * @return True if vehicle enters the reminder.
     */
    virtual bool isActivatedByEmitOrLaneChange(MSVehicle& veh) throw() = 0;


    /** @brief Returns the lane the reminder works on.
     *
     * @return The lane the reminder works on.
     */
    const MSLane* getLane(void) const {
        return laneM;
    }


protected:
    /// @brief Lane on which the reminder works
    MSLane* laneM;

};


#endif

/****************************************************************************/

