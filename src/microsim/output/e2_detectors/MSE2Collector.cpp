///
/// @file    MSE2Collector.cpp
/// @author  Christian Roessel <christian.roessel@dlr.de>
/// @date    Started Tue Dec 02 2003 22:13 CET
/// @version $Id$
///
/// @brief
///
///

// Copyright (C) 2003 by German Aerospace Center (http://www.dlr.de)

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.8  2006/10/25 12:22:40  dkrajzew
// updated
//
// Revision 1.7  2006/04/11 10:59:06  dkrajzew
// all structures now return their id via getID()
//
// Revision 1.6  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.5  2005/09/15 11:08:20  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/07/15 07:18:39  dkrajzew
// code style applied
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
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include "MSE2Collector.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG

/* =========================================================================
 * variable declarations
 * ======================================================================= */

std::string MSE2Collector::xmlHeaderM(
"<?xml version=\"1.0\" standalone=\"yes\"?>\n\n"
"<!--\n"
"- densityMean [veh/km]\n"
"- maxJamLengthInVehiclesMean [veh]\n"
"- maxJamLengthInMetersMean [m]\n"
"- jamLengthSumInVehiclesMean [veh]\n"
"- jamLengthSumInMetersMean [m]\n"
"- queueLengthAheadOfTrafficLightsInVehiclesMean [veh]\n"
"- queueLengthAheadOfTrafficLightsInMetersMean [m]\n"
"- nE2VehiclesMean [veh]\n"
"- occupancyDegreeMean [0,1]\n"
"- spaceMeanSpeedMean [m/s]\n"
"- currentHaltingDurationSumPerVehicleMean [s]\n"
"- nStartedHalts [n]\n"
//"- haltingDurationSum [s]\n"
"- haltingDurationMean [s]\n"
"-->\n\n");


std::string MSE2Collector::infoEndM = "</detector>";

namespace E2
{
    DetType& operator++( DetType& det )
    {
        return det = ( ALL == det ) ? DENSITY : DetType( det + 1 );
    }

    Containers& operator++( Containers& cont )
    {
        return cont =
            ( HALTINGS == cont ) ? COUNTER : Containers( cont + 1 );
    }
}
/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
