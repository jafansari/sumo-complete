/****************************************************************************/
/// @file    TraCIServerAPI_Simulation.h
/// @author  Daniel Krajzewicz
/// @date    07.05.2009
/// @version $Id: TraCIServerAPI_Simulation.h 6907 2009-03-13 12:13:38Z dkrajzew $
///
// APIs for getting/setting edge values via TraCI
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef TraCIServerAPI_Simulation_h
#define TraCIServerAPI_Simulation_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
#include "TraCIException.h"
#include <foreign/tcpip/storage.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TraCIServerAPI_Simulation
 * @brief APIs for getting/setting simulation values via TraCI
 */
class TraCIServerAPI_Simulation {
public:
    /** @brief Processes a get value command (Command 0xaa: Get Edge Variable)
     *
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     * @param[in] withStatus Whether the status message shall be written (not in subscription)
     */
    static bool processGet(tcpip::Storage &inputStorage, tcpip::Storage &outputStorage,
                           const std::map<MSNet::VehicleState, std::vector<std::string> > &info,
                           bool withStatus=true) throw(traci::TraCIException);

private:
    /// @brief invalidated copy constructor
    TraCIServerAPI_Simulation(const TraCIServerAPI_Simulation &s);

    /// @brief invalidated assignment operator
    TraCIServerAPI_Simulation &operator=(const TraCIServerAPI_Simulation &s);


};


#endif

/****************************************************************************/

