/****************************************************************************/
/// @file    TraCIServerAPI_Simulation.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// APIs for getting/setting edge values via TraCI
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/common/StdDefs.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include "TraCIConstants.h"
#include "TraCIServerAPIHelper.h"
#include "TraCIServerAPI_Simulation.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;
using namespace traci;
using namespace tcpip;


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Simulation::processGet(tcpip::Storage &inputStorage,
                                      tcpip::Storage &outputStorage,
                                      const std::map<MSNet::VehicleState, std::vector<std::string> > &infos,
                                      bool withStatus) throw(TraCIException) {
    std::string warning = ""; // additional description for response
    // variable & id
    int variable = inputStorage.readUnsignedByte();
    std::string id = inputStorage.readString();
    // check variable
    if (variable!=VAR_TIME_STEP
            &&variable!=VAR_LOADED_VEHICLES_NUMBER&&variable!=VAR_LOADED_VEHICLES_IDS
            &&variable!=VAR_DEPARTED_VEHICLES_NUMBER&&variable!=VAR_DEPARTED_VEHICLES_IDS
            &&variable!=VAR_TELEPORT_STARTING_VEHICLES_NUMBER&&variable!=VAR_TELEPORT_STARTING_VEHICLES_IDS
            &&variable!=VAR_TELEPORT_ENDING_VEHICLES_NUMBER&&variable!=VAR_TELEPORT_ENDING_VEHICLES_IDS
            &&variable!=VAR_ARRIVED_VEHICLES_NUMBER&&variable!=VAR_ARRIVED_VEHICLES_IDS
       ) {
        TraCIServerAPIHelper::writeStatusCmd(CMD_GET_SIM_VARIABLE, RTYPE_ERR, "Get Simulation Variable: unsupported variable specified", outputStorage);
        return false;
    }
    // begin response building
    Storage tempMsg;
    //  response-code, variableID, objectID
    tempMsg.writeUnsignedByte(RESPONSE_GET_SIM_VARIABLE);
    tempMsg.writeUnsignedByte(variable);
    tempMsg.writeString(id);
    // process request
    switch (variable) {
    case VAR_TIME_STEP:
        tempMsg.writeUnsignedByte(TYPE_DOUBLE);
        tempMsg.writeDouble(MSNet::getInstance()->getCurrentTimeStep());
        break;
    case VAR_LOADED_VEHICLES_NUMBER: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_BUILT)->second;
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    }
    break;
    case VAR_LOADED_VEHICLES_IDS: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_BUILT)->second;
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    }
    break;
    case VAR_DEPARTED_VEHICLES_NUMBER: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_DEPARTED)->second;
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    }
    break;
    case VAR_DEPARTED_VEHICLES_IDS: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_DEPARTED)->second;
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    }
    break;
    case VAR_TELEPORT_STARTING_VEHICLES_NUMBER: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_STARTING_TELEPORT)->second;
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    }
    break;
    case VAR_TELEPORT_STARTING_VEHICLES_IDS: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_STARTING_TELEPORT)->second;
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    }
    break;
    case VAR_TELEPORT_ENDING_VEHICLES_NUMBER: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_ENDING_TELEPORT)->second;
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    }
    break;
    case VAR_TELEPORT_ENDING_VEHICLES_IDS: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_ENDING_TELEPORT)->second;
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    }
    break;
    case VAR_ARRIVED_VEHICLES_NUMBER: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_ARRIVED)->second;
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    }
    break;
    case VAR_ARRIVED_VEHICLES_IDS: {
        const std::vector<std::string> &ids = infos.find(MSNet::VEHICLE_STATE_ARRIVED)->second;
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    }
    break;
    default:
        break;
    }
    if (withStatus) {
        TraCIServerAPIHelper::writeStatusCmd(CMD_GET_SIM_VARIABLE, RTYPE_OK, warning, outputStorage);
    }
    // send response
    outputStorage.writeUnsignedByte(0); // command length -> extended
    outputStorage.writeInt(1 + 4 + tempMsg.size());
    outputStorage.writeStorage(tempMsg);
    return true;
}


/****************************************************************************/

