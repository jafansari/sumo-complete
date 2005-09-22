#ifndef MSUNIT_H
#define MSUNIT_H

/**
 * @file   MSUnit.h
 * @author Christian Roessel
 * @date   Thu Aug  7 14:53:08 2003
 * @version $Id$
 * @brief  Declaraition and Definition of class MSUnit
 *
 */

/* Copyright (C) 2003 by German Aerospace Center (http://www.dlr.de) */

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//

// $Id$


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <cassert>
#include <cmath>


// conversion class from metric units (meters, seconds, etc.)
// to simulation units (cells, steps)
class MSUnit
{
public:
    typedef SUMOReal Meters;
    typedef SUMOReal Seconds;
    typedef SUMOReal Cells;
    typedef SUMOReal Steps;
    typedef unsigned int IntSteps;
    typedef SUMOReal MetersPerSecond;
    typedef SUMOReal CellsPerStep;
    typedef SUMOReal MetersPerCell;
    typedef SUMOReal SecondsPerStep;
    typedef SUMOReal VehPerKm;
    typedef SUMOReal VehPerCell;
    typedef SUMOReal VehPerHour;
    typedef SUMOReal VehPerStep;

    static void create( MetersPerCell metersPerCell,
                        SecondsPerStep secondsPerStep )
        {
            assert( instanceM == 0 );
            instanceM = new MSUnit( metersPerCell, secondsPerStep );
        }

    static MSUnit* getInstance( void )
        {
            return instanceM;
        }

    ~MSUnit( void )
        {
            instanceM = 0;
        }

    Seconds getSeconds( Steps steps ) const
        {
            return static_cast<Seconds>( steps * secondsPerStepM );
        }

    IntSteps getIntegerSteps( Seconds seconds ) const
        {
            return static_cast<IntSteps>( floor( seconds / secondsPerStepM ) );
        }

    Steps getSteps( Seconds seconds ) const
        {
            return static_cast<Steps>( seconds / secondsPerStepM );
        }

    Meters getMeters( Cells cells ) const
        {
            return static_cast<Meters>( cells * metersPerCellM );
        }

    Cells getIntegerCells( Meters meters ) const
        {
            return static_cast<Cells>( floor( meters / metersPerCellM ) );
        }

    Cells getCells( Meters meters ) const
        {
            return static_cast<Cells>( meters / metersPerCellM );
        }

    MetersPerSecond getMetersPerSecond( CellsPerStep cellsPerStep ) const
        {
            return static_cast<MetersPerSecond>(
                cellsPerStep * metersPerCellM / secondsPerStepM );
        }

    CellsPerStep getIntegerCellsPerStep(MetersPerSecond metersPerSecond) const
        {
            return static_cast<CellsPerStep>(
                floor( metersPerSecond * secondsPerStepM / metersPerCellM ) );
        }

    CellsPerStep getCellsPerStep( MetersPerSecond metersPerSecond ) const
        {
            return static_cast<CellsPerStep>(
                metersPerSecond * secondsPerStepM / metersPerCellM );
        }

    VehPerKm getVehPerKm( VehPerCell vehPerCell ) const
        {
            return static_cast< VehPerKm >(
                vehPerCell * oneKM / metersPerCellM );
        }

    VehPerCell getVehPerCell( VehPerKm vehPerKm ) const
        {
            return static_cast< VehPerCell >(
                vehPerKm * metersPerCellM / oneKM );
        }

    VehPerHour getVehPerHour( VehPerStep vehPerStep ) const
        {
            return static_cast< VehPerHour >(
             vehPerStep * oneHour / secondsPerStepM );
        }

    VehPerStep getVehPerStep( VehPerHour vehPerHour )const
        {
            return static_cast< VehPerStep >(
             vehPerHour * secondsPerStepM / oneHour );
        }


private:

    MSUnit( MetersPerCell metersPerCell, SecondsPerStep secondsPerStep )
        : metersPerCellM( metersPerCell ),
          secondsPerStepM( secondsPerStep )
        {
            assert( metersPerCellM > 0 );
            assert( secondsPerStepM > 0 );
        }

    const MetersPerCell metersPerCellM;
    const SecondsPerStep secondsPerStepM;
    static MSUnit* instanceM;
    static const Meters oneKM;
    static const Seconds oneHour;
};


#endif // MSUNIT_H

// Local Variables:
// mode:C++
// End:
