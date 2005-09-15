/**
 * @file   MSDensity.cpp
 * @author Christian Roessel
 * @date   Started Thu Sep 11 13:35:55 2003
 * @version
 * @brief
 *
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

//

//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)

/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include "MSDensity.h"
#include <microsim/MSUnit.h>
#include <microsim/MSLane.h>
#include <cassert>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG

/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;

/* =========================================================================
 * member method definitions
 * ======================================================================= */
MSDensity::MSDensity( double lengthInMeters,
                      const DetectorContainer::Count& counter ) :
    detectorLengthM( lengthInMeters / 1000.0 ),
    counterM( counter )
{}


MSDensity::DetectorAggregate
MSDensity::getDetectorAggregate( void ) // [veh/km]
{
    double nVehOnDet = counterM.vehicleCountM -
        counterM.occupancyCorrectionM->getOccupancyEntryCorrection() -
        counterM.occupancyCorrectionM->getOccupancyLeaveCorrection();
    return nVehOnDet / detectorLengthM;
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/


// Local Variables:
// mode:C++
// End:
