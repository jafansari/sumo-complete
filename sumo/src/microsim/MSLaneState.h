#ifndef MSLaneState_H
#define MSLaneState_H

//---------------------------------------------------------------------------//
//                        MSLaneState.h  -
//  Some kind of induct loops with a length
//                           -------------------
//  begin                : Tue, 18 Feb 2003
//  copyright            : (C) 2003 by Daniel Krajzewicz
//  organisation         : IVF/DLR
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

/**
 * @file   MSLaneState.h
 * @author Christian Roessel
 * @date   Started Tue, 18 Feb 2003
 * $Revision$ from $date$ by $Author$
 * @brief  Declaration of class MSLaneState and helper functions.
 * 
 */


//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//

// $Log$
// Revision 1.18  2003/07/03 11:00:52  roessel
// Put global functions in an unnamed namespace to make them local.
//
// Revision 1.17  2003/06/10 12:55:11  roessel
// Added documentation.
//
// Revision 1.16  2003/06/06 07:42:31  roessel
// Thanks, but removed umlaut.
//
// Revision 1.15  2003/06/05 16:04:46  dkrajzew
// had to remove sorting-methods from the VehicleData-structure
//
// Revision 1.14  2003/06/05 09:53:46  roessel
// Numerous changes and new methods/members.
//
// Revision 1.13  2003/05/28 15:36:32  roessel
// Added argument MSNet::Time deleteDataAfterSeconds with default value 900
// timesteps to constructor.
//
// Revision 1.12  2003/05/28 07:51:25  dkrajzew
// had to add a return value due to the usage of the mem_func-function in
// combination with for_each (MSVC++-reasons?)
//
// Revision 1.11  2003/05/27 19:01:26  roessel
// Removed OutputStyle in ctor (output will be xml).
//
// Revision 1.10  2003/05/26 15:31:09  roessel
// Removed superflous #include "MSLane.h"
//
// Revision 1.9  2003/05/26 15:24:15  roessel
// Removed warnings/errors. Changed return-type of getNumberOfWaiting to
// double.
//
// Revision 1.8  2003/05/26 13:19:20  roessel
// Completed all get* methods.
//
// Revision 1.7  2003/05/25 17:50:31  roessel
// Implemented getCurrentNumberOfWaiting.
// Added methods actionBeforeMove and actionAfterMove. actionBeforeMove creates
// a TimestepData entry in timestepDataM every timestep (makes live easier).
// actionAfterMove calculates the waitingQueueLength and updates the current
// TimestepData.
// These two methods must be called in the simulation loop.
//
// Revision 1.6  2003/05/23 16:42:22  roessel
// Added method getCurrentDensity().
//
// Revision 1.5  2003/05/21 16:20:44  dkrajzew
// further work detectors
//
// Revision 1.4  2003/04/02 11:44:03  dkrajzew
// continuation of implementation of actuated traffic lights
//
// Revision 1.3  2003/03/19 08:02:02  dkrajzew
// debugging due to Linux-build errors
//
// Revision 1.2  2003/03/17 14:12:19  dkrajzew
// Windows eol removed
//
// Revision 1.1  2003/03/03 14:56:19  dkrajzew
// some debugging; new detector types added; actuated traffic lights added
//
//

/* =========================================================================
 * included modules
 * ======================================================================= */
#include "MSNet.h"
#include <string>
#include <functional>
#include <deque>
#include <map>
#include <iostream>

/* =========================================================================
 * class declarations
 * ======================================================================= */

class MSMoveReminder;
class MSLane;


/**
 * Overhead-detector that works on a part of a MSLane. It provides current and
 * aggregated data for waitingQueueLength, MeanSpeed, MeanSpeedSquare,
 * MeanDensity, Traveltime and counts of vehicles that contributed to the
 * data calculation, that enterd the detector, left the detector by move and
 * that passed the detector entirely. If used with MSTravelcostDetector it
 * provides a XML output string. The vehicles connect to the detector
 * via reminder objects, i.e. MSLaneStateReminder. The Detector creates such
 * a reminder and passes it to the corresponding lane. The lane passes the
 * reminder to the vehicles when they enter the lane. In each timestep, the
 * vehicles work on their reminders. If the reminder is active, it passes
 * information to the detector.
 * @see MSLaneStateReminder
 * @see MSMoveReminder
 * @see MSTravelcostDetector
 */
class MSLaneState
{
public:
    /** 
     * Constructor. Adds object into a SingletonDictionary. Set old-data
     * removal event. Adds reminder to MSLane.
     * 
     * @param id Unique id.
     * @param lane Lane where detector woks on.
     * @param begin Startposition of detector.
     * @param length Detectorlength.
     * @param deleteDataAfterSeconds Dismiss time for collected data. 
     */
    MSLaneState( std::string    id,
                 MSLane*        lane,
                 double         begin,
                 double         length,
                 MSNet::Time    deleteDataAfterSeconds = 900 );

    /** 
     * Destructor. Clears containers. Deletes created reminder.
     * 
     */
    ~MSLaneState();

    /**
     * Calculates the meanValue of the waiting-queue length during the
     * lastNTimesteps. Vehicles in a waiting-queue have a gap <= vehLength.
     *
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]
     *
     * @return mean waiting-queue length
     */
    double getNumberOfWaiting( MSNet::Time lastNTimesteps );

    /**
     * Get the waitingQueueLength.
     * Vehicles in a waiting-queue have a gap <= vehLength.
     * If called before the vehicles are
     * moved, the value of the previous timestep is returned, if called
     * after move, the value of the current timestep is returned. Currently
     * the junctions do their job before the move, so if you call the method
     * by a junction, you will get the value of the previous timestep.
     *
     * @return waitingQueueLength of the current or previous timestep
     */
    int getCurrentNumberOfWaiting( void );

    /** 
     * Calculates the mean speed averaged over the lastNTimesteps.
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]
     * 
     * @return Mean speed averaged over the lastNTimesteps
     */
    double getMeanSpeed( MSNet::Time lastNTimesteps );

    /** 
     * Calculates the mean speed of the current timestep.
     * 
     * 
     * @return Current mean speed.
     */
    double getCurrentMeanSpeed( void );

    /** 
     * Calculates the mean speed square averaged over the lastNTimesteps.
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]
     * 
     * @return Mean speed square averaged over the lastNTimesteps
     */
    double getMeanSpeedSquare( MSNet::Time lastNTimesteps );

    /** 
     * Calculates the mean speed square of the current timestep.
     * 
     * 
     * @return Current mean speed square.
     */
    double getCurrentMeanSpeedSquare( void );

    /** 
     * Calculates the mean density averaged over the lastNTimesteps.
     * 
     * @param lastNTimesteps  take data out of the intervall
     * (now-lastNTimesteps, now]
     * 
     * @return Mean density averaged over the lastNTimesteps.
     */
    double getMeanDensity( MSNet::Time lastNTimesteps );
    
    /** 
     * Calculates the density of the current timestep.
     * 
     * 
     * @return Current density.
     */
    double getCurrentDensity( void );

    /** 
     * Calculates the mean traveltime averaged over the lastNTimesteps. Only
     * vehicles that passed the entire detector contribute. These vehicles
     * may have entered the detector during a previous intervall.
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]
     * 
     * @return Mean traveltime averaged over the lastNTimesteps.
     */
    double getMeanTraveltime( MSNet::Time lastNTimesteps );

    /** 
     * Counts the vehicles that added data through addMoveData() over
     * the lastNTimesteps. These vehicles contribute to
     * getMeanDensity(), getMeanSpeed() and
     * getCurrentMeanSpeedSquare().
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now] 
     * 
     * @return Number of vehicles that added data over the lastNTimesteps.
     */
    int getNVehContributed( MSNet::Time lastNTimesteps );

    /** 
     * Counts the vehicles that entered the detector by move, emit or
     * lanechange over the lastNTimesteps.
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]  
     * 
     * @return Number of vehicles that antered the detector over the
     * lastNTimesteps.
     */
    int getNVehEnteredDetector( MSNet::Time lastNTimesteps );

    /** 
     * Counts the vehicles that left the detector by move over the
     * lastNTimesteps. Vehicles that left by lanechange or that reached their
     * destination do not contribute.
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]  
     * 
     * @return Number of vehilces that left the detector by move over the
     * lastNTimesteps.
     */
    int getNVehLeftDetectorByMove( MSNet::Time lastNTimesteps );

    /** 
     * Counts the vehicles that left the detector over the lastNTimesteps
     * and passed the entire detector without interruption. These vehicles
     * may have entered the detector before "now - lastNTimesteps". Only
     * those contribute to getMeanTraveltime().
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now] 
     * 
     * @return Numner of vehicles that left the detector over the
     * lastNTimesteps and passed the entire detector.
     */
    int getNVehPassedEntireDetector( MSNet::Time lastNTimesteps );

    /** 
     * Creates a XML-header explaining the data written by getXMLOutput().
     * 
     * @return XML-header and comment.
     */
    static std::string& getXMLHeader( void );

    /** 
     * Creates XML-output for the interval (now-lastNTimesteps, now] containing
     * getMeanTraveltime(), getMeanSpeed(), getMeanSpeedSquare(),
     * getNVehContributed(), getNVehPassedEntireDetector(),
     * getNVehEnteredDetector() and getNVehLeftDetectorByMove().
     * 
     * @param lastNTimesteps take data out of the intervall
     * (now-lastNTimesteps, now]
     * 
     * @return XML-output of calculated data for the intervall
     * (now-lastNTimesteps, now] 
     */
    std::string getXMLOutput( MSNet::Time lastNTimesteps );

    /**
     * @name Reminder methods.
     *
     * Methods in this group are called by MSLaneStateReminder
     * only. They add the data to calculate the get* values.
     *
     * @see MSLaneStateReminder
     */
    //@{
    
    /** 
     * Add data from moving vehicles. Each moving vehicle on the
     * detector has to call this method in every timestep via the reminder
     * mechanism. 
     *
     * @see MSLaneStateReminder
     * @param veh A reference to the vehicle that adds data.
     * @param newSpeed The speed with which the vehicle will move in this
     * timestep.
     * @param timestepFraction Fraction of the timestep the
     * vehicle is on the detector.
     */
    void addMoveData( MSVehicle& veh, double newSpeed,
                      double timestepFraction );

    /** 
     * Introduces a vehicle to the detector that enters it by move. Called
     * by MSLaneStateReminder only.
     *
     * @see MSLaneStateReminder
     * @param veh The entering vehicle.
     * @param enterTimestepFraction Fraction of the timestep the
     * vehicle is on the detector.
     */
    void enterDetectorByMove( MSVehicle& veh, double enterTimestepFraction );

    /** 
     * Introduces a vehicle to the detetctor that enters it by
     * emission or lanechange. Called by MSLaneStateReminder only.
     *
     * @see MSLaneStateReminder
     * @param veh The entering vehocle.
     */
    void enterDetectorByEmitOrLaneChange( MSVehicle& veh );

    /** 
     * Removes a vehicle from the detector that leaves by move. Vehicles that
     * reached their destination are also handled by this method. Called by
     * MSLaneStateReminder only.
     *
     * @see MSLaneStateReminder
     * @param veh The leaving vehicle.
     * @param leaveTimestepFraction Fraction of the timestep the
     * vehicle is on the detector.
     */
    void leaveDetectorByMove( MSVehicle& veh, double leaveTimestepFraction );

    /** 
     * Removes a vehicle from the detector that leaves by lanechange. Called by
     * MSLaneStateReminder only.
     *
     * @see MSLaneStateReminder
     * @param veh The leaving vehicle.
     */
    void leaveDetectorByLaneChange( MSVehicle& veh );

    //@}

    /**
     * @name Simulationloop methods.
     *
     * Call the following methods once in every timestep in the
     * simulation loop. Neccessary for proper operation of the
     * detector.
     * 
     */
    //@{
    /** 
     * Calls for all MSLaneState objects actionBeforeMoveAndEmit() during the
     * simulation loop. Call it before move and emit is activated.
     * 
     */
    static void actionsBeforeMoveAndEmit( void );

    /** 
     * Calls for all MSLaneState objects actionAfterMoveAndEmit() during the
     * simulation loop. Call it after move and emit.
     * 
     */
    static void actionsAfterMoveAndEmit( void );
    //@}

    /** 
     * Returns a string indentifiing the class. Used for filenames in
     * MSTravelcostDetector
     * 
     * @see MSTravelcostDetector
     * @return String "MSLaneState"
     */
    static std::string getNamePrefix( void );

    // forward declarations
    struct TimestepData;
    struct WaitingQueueElem;
    struct VehicleData;
    
    /// Type of the container where all TimestepData are stored.
    typedef std::deque< TimestepData >           TimestepDataCont;
    /// Type of the container where all WaitingQueueElem are stored.
    typedef std::vector< WaitingQueueElem >      WaitingQueueElemCont;
    /**
     * Type of the map where VehicleData of vehicles on the detector
     * are stored.
     */
    typedef std::map< std::string, VehicleData > VehicleDataMap;
    /**
     * Type of the container where VehicleData of vehicles that left the
     * detector are stored. 
     */
    typedef std::deque< VehicleData >            VehicleDataCont;

    /**
     * Collection of data for each timestep.
     */
    struct TimestepData
    {
        /// Sole ctor.
        TimestepData( MSNet::Time timestep ) :
            timestepM( timestep ),
            speedSumM(0),
            speedSquareSumM(0),
            contTimestepSumM(0),
            timestepSumM(0),
            queueLengthM(-1),
            nVehEnteredDetectorM(0)
            {}

        MSNet::Time timestepM;  /**< The timestep the data belongs to. */
        double speedSumM;       /**< Sum of the speeds the vehicles on the
                                 * detector added via addMoveData() during
                                 * timestep. */
        double speedSquareSumM; /**< Sum of the speedsquares the
                                 * vehicles on the detector added via
                                 * addMoveData() during timestep. */
        double contTimestepSumM; /**< Continuous sum of timesteps the
                                  * vehicles spend on the
                                  * detector. Continuous because
                                  * vehicles enter/leave the detector
                                  * and contribute only a fraction of
                                  * a timestep.*/
        double timestepSumM;    /**< Discrete sum of timesteps the
                                  * vehicles spend on the
                                  * detector. Every vehicle
                                  * contributes 1 even if it spend
                                  * only a fraction of a timestep. */
        int queueLengthM;       /**< Length of the waiting queue
                                 * calculated by
                                 * actionsAfterMoveAndEmit() after
                                 * move and emit of all vehicles. */
        int nVehEnteredDetectorM; /**< Number of vehicles that entered the
                                   * detector either by move or lanechange or
                                   * emit.*/
    };

    /**
     * Collection of data for each vehicle that is or has been on the detector.
     */
    struct VehicleData
    {
        /// Sole ctor.
        VehicleData( double entryContTimestep,
                     bool enteredDetectorByMove ) :
            entryContTimestepM ( entryContTimestep ),
            leaveContTimestepM ( -1 ),
            passedEntireDetectorM ( enteredDetectorByMove ),
            leftDetectorByMoveM ( false )
            {}
        
        double entryContTimestepM; /**< The vehicles entry timestep
                                    * (continuous) */
        double leaveContTimestepM; /**< The vehicles leave timestep
                                    * (continuous). */
        bool passedEntireDetectorM; /**< Indicator if vehicle passed
                                     * the entire dtetctor without
                                     * interruption. */
        bool leftDetectorByMoveM; /**< Indicator if vehicle left the
                                   * detector by move. */
    };

    /**
     * Collection of data for calculation of the
     * waiting-queue-length. Updated every timestep.
     */
    struct WaitingQueueElem
    {
        /// Sole ctor.
        WaitingQueueElem( double pos, double vehLength ) :
            posM( pos ), vehLengthM( vehLength )
            {}

        double posM;            /**< Current position of vehicle on
                                 * the detector. */
        double vehLengthM;      /**< Length of the vehicle.  */
    };

    /**
     * @name Predicate classes.
     * 
     * Predicate classes working on the nested structs VehicleData and
     * WaitingQueueElem. They are included in these structs because of
     * MSVC++ compile-problems.
     * 
     */
    //@{
    /**
     * Returns true if passed VehicleData's entryContTimestepM is lesser
     * than the passed entryTimestepBound.
     * 
     */
    struct entryTimestepLesser :
        public std::binary_function< VehicleData, double, bool >
    {
        bool operator()( const VehicleData& data,
                         double entryTimestepBound ) const
        {
            return data.entryContTimestepM < entryTimestepBound;
        }
    };

    /** 
     * Returns true if passed VehicleDataMap's entryContTimestepM is lesser
     * than the passed entryTimestepBound.
     * 
     */
    struct entryTimestepLesserMap :
            public std::binary_function< VehicleDataMap::value_type,
                double, bool >
    {
        bool operator()( const VehicleDataMap::value_type& data,
                         double entryTimestepBound ) const
        {
            return data.second.entryContTimestepM < entryTimestepBound;
        }
    };

    /**
     * Returns true if passed VehicleData's leaveContTimestepM is lesser 
     * than the passed leaveTimestepBound.
     * 
     */
    struct leaveTimestepLesser :
            public std::binary_function< VehicleData, double, bool >
    {
        bool operator()( const VehicleData& data,
                         double leaveTimestepBound ) const
        {
            return data.leaveContTimestepM < leaveTimestepBound;
        }
    };

    /**
     * Returns true if first WaitingQueueElem's posM is greater than
     * the second WaitingQueueElem's posM.
     * 
     */
    struct PosGreater : public std::binary_function<
        const WaitingQueueElem, const WaitingQueueElem, bool >
    {
        // Sort criterion for std::vector< WaitingQueueElem >
        // We sort in descending order
        bool operator() ( const WaitingQueueElem p1,
                          const WaitingQueueElem p2 ) const {
            return p1.posM > p2.posM;
        }
    };
    //@}


protected:
    /** 
     * Creates a new TimestepData mand adds it to timestepDataM.
     * 
     * @return true. MSVC++ requires a return value, but there is no
     * need for it.
     */
    bool actionBeforeMoveAndEmit( void );

    /** 
     * Currently just calls calcWaitingQueueLength().
     * 
     * @return true. MSVC++ requires a return value, but there is no
     * need for it.
     */
    bool actionAfterMoveAndEmit( void );

    /** 
     * Deletes data from timestepDataM and vehLeftDetectorM if 
     * deleteDataAfterSecondsM is over. Is called via MSEventControl.
     * 
     * @return deleteDataAfterSecondsM to recur the event. 
     */
    MSNet::Time deleteOldData( void );

    /** 
     * Calculates the waiting-queue-length every timestep from
     * waitingQueueElemsM and stores value in timestepDataM.
     * 
     */
    void calcWaitingQueueLength( void );

    /** 
     * Calculates a valid timestep out of [0, now] from where accumulation
     * in the get* methods will start.
     * 
     * @param lastNTimesteps Number of timesteps we want to look back.
     * @see getStartIterator()
     * @return max( now - lastNTimesteps, 0 )
     */
    double getStartTimestep( MSNet::Time lastNTimesteps );

    /** 
     * Indicator if stored value nVehContributedM is still valid.
     * nVehContributedM needs to be recalculated if one of the Reminder methods
     * has been called since the last calculation or if the requested interval
     * has changed.
     * 
     * @param lastNTimesteps Interval that is requested.
     * @see Reminder methods
     * @return True if new calculation is neccessary, false otherwise.
     */
    bool needsNewCalculation( MSNet::Time lastNTimesteps );

    /** 
     * Calculates a valid interator to container from where accumulation
     * in the get* methods will start.
     * 
     * @param lastNTimesteps Number of timesteps we want to look back.
     * @param container Reference to the container we are interesed in.
     * @see getStartTimestep()
     * @return An interator out of [container.begin(), container.end() ).
     */
    template< class T >
    typename T::iterator getStartIterator( MSNet::Time lastNTimesteps,
                                           T& container )
        {
            typename T::iterator start = container.begin();
            if ( container.size() > lastNTimesteps ) {
                start =  container.end() - lastNTimesteps;
            }
            return start;
        }


private:
    std::string idM;            /**< Unique id  of the detector. */

    TimestepDataCont     timestepDataM; /**< Container for
                                         * TimestepData. There is one
                                         * entry for each
                                         * timestep. This entry
                                         * accumulates the data from
                                         * all vehicles on the
                                         * detector at this
                                         * timestep.  */
    
    VehicleDataMap       vehOnDetectorM; /**< Container for VehicleData of
                                          * vehicles that are currently on
                                          * the detector. */
    WaitingQueueElemCont waitingQueueElemsM; /**< Container for
                                              * WaitingQueueElem. Cleared every
                                              * timestep after
                                              * calcWaitingQueueLength(). */
    
    VehicleDataCont      vehLeftDetectorM; /**< Container for VehicleData of
                                            * vehicles that already left
                                            * the detector. */

    MSLane* laneM;              /**< Lane where detector works on. */

    double posM;                /**< Start-position of the detector. */

    double lengthM;             /**< Length on the detector. */

    MSMoveReminder* reminderM;  /**< Reminder created in ctor and passed to
                                 * laneM. */

    const MSNet::Time deleteDataAfterSecondsM; /**< Keep at least data from
                                                * the last
                                                * deleteDataAfterSecondsM. */

    bool modifiedSinceLastLookupM; /**< Indicator if container have been
                                    * modified since last
                                    * getNVehContributed() call. */

    MSNet::Time lookedUpLastNTimestepsM; /**< Interval of last
                                          * getNVehContributed() call. */

    int nVehContributedM;       /**< Returned value of last
                                 * getNVehContributed() call. */

    /**< Container that holds all MSLaneState objects. Used by
     * actionsBeforeMoveAndEmit() and actionsAfterMoveAndEmit(). */
    static std::vector< MSLaneState* > laneStateDetectorsM; 

    static std::string xmlHeaderM; /**< String containing a xml-comment. */

    /// Hidden constructor.
    MSLaneState();

    /// Hidden copy constructor.
    MSLaneState( const MSLaneState& );

    /// Hidden assignment operator.
    MSLaneState& operator=( const MSLaneState& );
};


namespace
{
    /**
     * @name Binary-functions to use with std::accumulate.
     * 
     */
    //@{
    /// Adds up TimestepData::speedSumM.
    inline double speedSum( double sumSoFar,
                            const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.speedSumM;
    }

    /// Adds up TimestepData::speedSquareSumM.
    inline double speedSquareSum( double sumSoFar,
                                  const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.speedSquareSumM;
    }

    /// Adds up TimestepData::contTimestepSumM.
    inline double contTimestepSum( double sumSoFar,
                                   const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.contTimestepSumM;
    }

    /// Adds up TimestepData::timestepSumM.
    inline double timestepSum( double sumSoFar,
                               const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.timestepSumM;
    }

    /// Adds up TimestepData::queueLengthM.
    inline double waitingQueueSum( double sumSoFar,
                                   const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.queueLengthM;
    }

    /// Adds up traveltimes if vehicles has passed the entire detetctor.
    inline double traveltimeSum( double sumSoFar,
                                 const MSLaneState::VehicleData& data )
    {
        if ( data.passedEntireDetectorM ) {
            return sumSoFar + data.leaveContTimestepM -
                data.entryContTimestepM;
        }
        return sumSoFar;
    }

    /// Adds up VehicleData::leftDetectorByMoveM.
    inline int leftByMoveSum( int sumSoFar,
                              const MSLaneState::VehicleData& data )
    {
        return sumSoFar + data.leftDetectorByMoveM;
    }

    /// Adds up VehicleData::passedEntireDetectorM.
    inline int passedEntireSum( int sumSoFar,
                                const MSLaneState::VehicleData& data )
    {
        return sumSoFar + data.passedEntireDetectorM;
    }

    /// Adds up TimestepData::nVehEnteredDetectorM.
    inline int nVehEnteredSum( int sumSoFar,
                               const MSLaneState::TimestepData& data )
    {
        return sumSoFar + data.nVehEnteredDetectorM;
        //@}
    }
}

#endif

// Local Variables:
// mode:C++
// End:
