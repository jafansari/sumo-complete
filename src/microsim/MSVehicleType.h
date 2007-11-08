/****************************************************************************/
/// @file    MSVehicleType.h
/// @author  Christian Roessel
/// @date    Mon, 12 Mar 2001
/// @version $Id$
///
// parameters.
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
#ifndef MSVehicleType_h
#define MSVehicleType_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <cassert>
#include <map>
#include <string>
#include <utils/common/SUMOTime.h>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/RandHelper.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class BinaryInputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicleType
 * @brief Storage for parameters of vehicles of the same abstract type
 *
 * The vehicle type stores the parameter of a single vehicle type.
 * It is assumed that within the simulation many vehicles are using the same
 *  vehicle type, quite common is using only one vehicle type for all vehicles.
 * You can think of it like of having a vehicle type for each VW Golf or
 *  Ford Mustang in your simulation while the car instances just refer to it.
 */
class MSVehicleType
{
public:
    /// Constructor.
    MSVehicleType(const std::string &id, SUMOReal length, SUMOReal maxSpeed,
                  SUMOReal accel, SUMOReal decel, SUMOReal dawdle,
                  SUMOReal tau,
                  SUMOVehicleClass vclass);

    /// Destructor.
    virtual ~MSVehicleType();









    /**  */
    virtual SUMOReal brakeGap(SUMOReal speed) const {
        return speed * speed * myInverseTwoDecel + speed * myTau;
    }

    virtual SUMOReal approachingBrakeGap(SUMOReal speed) const {
        return speed * speed * myInverseTwoDecel;
    }

    /** */
    virtual SUMOReal interactionGap(SUMOReal vF, SUMOReal laneMaxSpeed, SUMOReal vL) const {
        // Resolve the vsafe equation to gap. Assume predecessor has
        // speed != 0 and that vsafe will be the current speed plus acceleration,
        // i.e that with this gap there will be no interaction.
        SUMOReal vNext = MIN2(maxNextSpeed(vF), laneMaxSpeed);
        SUMOReal gap = (vNext - vL) *
                       ((vF + vL) * myInverseTwoDecel + myTau) +
                       vL * myTau;

        // Don't allow timeHeadWay < deltaT situations.
        return MAX2(gap, timeHeadWayGap(vNext));
    }

    /**  */
    bool hasSafeGap(SUMOReal speed, SUMOReal gap, SUMOReal predSpeed, SUMOReal laneMaxSpeed) const {
        SUMOReal vSafe = ffeV(speed, gap, predSpeed);
        SUMOReal vNext = MIN3(maxNextSpeed(speed), laneMaxSpeed, vSafe);
        return (vNext>=getSpeedAfterMaxDecel(speed)
                &&
                gap   >= timeHeadWayGap(predSpeed));
    }

    /** Returns the minimum gap between this driving vehicle and a
     * possibly emitted vehicle with speed 0. */
    SUMOReal safeEmitGap(SUMOReal speed) const {
        SUMOReal vNextMin = getSpeedAfterMaxDecel(speed); // ok, minimum next speed
        SUMOReal safeGap  = vNextMin * (speed * myInverseTwoDecel + myTau);
        return MAX2(safeGap, timeHeadWayGap(speed)) + ACCEL2DIST(getMaxAccel(speed));
    }


    /** Dawdle according the vehicles dawdle parameter. Return value >= 0 */
    SUMOReal dawdle(SUMOReal speed) const {
        // generate random number out of [0,1]
        SUMOReal random = RandHelper::rand();
        // Dawdle.
        if (speed<getMaxAccel(0)) {
            // we should not prevent vehicles from driving just due to dawdling
            //  if someone is starting, he should definitely start
            // (but what about slow-to-start?)!!!
            speed -= ACCEL2SPEED(myDawdle * speed * random);
        } else {
            speed -= ACCEL2SPEED(myDawdle * getMaxAccel(speed) * random);
        }
        return MAX2(SUMOReal(0), speed);
    }

    SUMOReal getSpeedAfterMaxDecel(SUMOReal v) const {
        return MAX2((SUMOReal) 0, v - ACCEL2SPEED(myDecel));
    }

    SUMOReal maxNextSpeed(SUMOReal v) const {
        return MIN2(v+ACCEL2SPEED(getMaxAccel(v)), myMaxSpeed);
    }

    SUMOReal ffeV(SUMOReal speed, SUMOReal gap2pred, SUMOReal predSpeed) const {
        return MIN2(_vsafe(speed, myDecel, gap2pred, predSpeed), maxNextSpeed(speed));
    }

    SUMOReal ffeS(SUMOReal speed, SUMOReal gap2pred) const {
        return MIN2(_vsafe(speed, myDecel, gap2pred, 0), maxNextSpeed(speed));
    }

    SUMOReal getSecureGap(SUMOReal speed, SUMOReal predSpeed, SUMOReal predLength) const {
        SUMOReal safeSpace2 = brakeGap(speed);
        SUMOReal vSafe = ffeV(0, 0, predSpeed);
        SUMOReal safeSpace3 =
            ((vSafe - predSpeed)
             * (SUMOReal)((vSafe+predSpeed) / 2.0 / (2.0 * MSVehicleType::getMinVehicleDecel()) + myTau))
            + predSpeed * myTau;
        SUMOReal safeSpace = safeSpace2 > safeSpace3 ? safeSpace2 : safeSpace3;
        safeSpace = safeSpace > ACCEL2SPEED(myDecel) ? safeSpace : ACCEL2SPEED(myDecel);
        safeSpace += predLength;
        safeSpace += ACCEL2SPEED(getMaxAccel(speed));
        return safeSpace;
    }

    SUMOReal decelAbility() const {
        return ACCEL2SPEED(myDecel); // !!! really the speed?
    }


    /** */
    SUMOReal timeHeadWayGap(SUMOReal speed) const {
        assert(speed >= 0);
        return SPEED2DIST(speed);
    }












    /// Get vehicle's length [m].
    SUMOReal getLength() const {
        return myLength;
    }

    /// Get vehicle's maximum speed [m/s].
    SUMOReal getMaxSpeed() const {
        return myMaxSpeed;
    }

    /// Get the vehicle's maximum acceleration [m/s^2]
    inline SUMOReal getMaxAccel(SUMOReal v) const {
        return (SUMOReal)(myAccel *(1.0 - (v/myMaxSpeed)));
    }

    /// Get the vehicle's maximum deceleration [m/s^2]
    SUMOReal getMaxDecel() const {
        return myDecel;
    }

    SUMOReal getTau() const {
        return myTau;
    }


    /// returns the name of the vehicle type
    const std::string &getID() const;


    /// Returns the minimum deceleration-ability of all vehicle-types.
    static SUMOReal getMinVehicleDecel() {
        return myMinDecel;
    }


    /// Saves the states of a vehicle
    void saveState(std::ostream &os);

    SUMOVehicleClass getVehicleClass() const {
        return myVehicleClass;
    }


protected:
    /** Returns the SK-vsafe. */
    SUMOReal _vsafe(SUMOReal currentSpeed, SUMOReal /*decelAbility*/,
                    SUMOReal gap2pred, SUMOReal predSpeed) const {
        if (predSpeed==0&&gap2pred<0.01) {
            return 0;
        }
        assert(currentSpeed     >= SUMOReal(0));
        assert(gap2pred  >= SUMOReal(0));
        assert(predSpeed >= SUMOReal(0));
        SUMOReal vsafe = predSpeed +
                         ((gap2pred - predSpeed * myTau) /
                          (((predSpeed + currentSpeed) * myInverseTwoDecel) + myTau));
        assert(vsafe >= 0);
        return vsafe;
    }


private:
    /// Unique ID.
    std::string myID;

    /// Vehicle's length [m].
    SUMOReal myLength;

    /// Vehicle's maximum speed [m/s].
    SUMOReal myMaxSpeed;

    /// The vehicle's maximum acceleration [m/s^2]
    SUMOReal myAccel;

    /// The vehicle's maximum deceleration [m/s^2]
    SUMOReal myDecel;

    /// The vehicle's dawdle-parameter. 0 for no dawdling, 1 for max.
    SUMOReal myDawdle;

    /// The precomputed value for 1/(2*d)
    SUMOReal myInverseTwoDecel;



    SUMOReal myTau;
    SUMOVehicleClass myVehicleClass;

    /// Minimum deceleration-ability of all vehicle-types.
    static SUMOReal myMinDecel;

    /// Maximum length of all vehicle-types.
    static SUMOReal myMaxLength;

    /// Invalidated copy constructor
    MSVehicleType(const MSVehicleType&);

    /// Invalidated assignment operator
    MSVehicleType& operator=(const MSVehicleType&);

};


#endif

/****************************************************************************/

