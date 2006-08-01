#ifndef MSE3NVEHICLES_H
#define MSE3NVEHICLES_H

///
/// @file    MSE3NVehicles.h
/// @author  Christian Roessel <christian.roessel@dlr.de>
/// @date    Started Thu Nov 27 2003 20:24 CET
/// @version
///
/// @brief
///
///

/* Copyright (C) 2003 by German Aerospace Center (http://www.dlr.de) */

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//

/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <microsim/output/MSDetectorContainerWrapper.h>
#include <string>


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 *
 */
class MSE3NVehicles
{
protected:
    typedef SUMOReal DetectorAggregate;
    typedef DetectorContainer::VehicleMap Container;
    typedef Container::InnerContainer VehiclesMap;

    MSE3NVehicles( const Container& container )
    : containerM( container )
    {
    }

    virtual ~MSE3NVehicles( void ) {}

    bool hasVehicle( MSVehicle& veh ) const {
        return containerM.hasVehicle( &veh );
    }

    DetectorAggregate getValue( MSVehicle& veh ) {
        return 1;
    }

    static std::string getDetectorName( void ) {
        return "nE3Vehicles";
    }

private:
    const Container& containerM;

    MSE3NVehicles();
    MSE3NVehicles( const MSE3NVehicles& );
    MSE3NVehicles& operator=( const MSE3NVehicles& );

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/



// Local Variables:
// mode:C++
// End:

#endif // MSE3NVEHICLES_H
