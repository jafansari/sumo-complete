/****************************************************************************/
/// @file    TraCITestClient.cpp
/// @author  Friedemann Wesner <wesner@itm.uni-luebeck.de>
/// @date    2008/04/07
/// @version $Id$
///
/// A dummy client to simulate communication to a TraCI server
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


// ===========================================================================
// included modules
// ===========================================================================
#include "TraCITestClient.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <traci-server/TraCIConstants.h>

#define BUILD_TCPIP
#include <foreign/tcpip/storage.h>
#include <foreign/tcpip/socket.h>


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;
using namespace tcpip;
using namespace testclient;


// ===========================================================================
// method definitions
// ===========================================================================

TraCITestClient::TraCITestClient(std::string outputFileName)
        :socket(NULL),
        outputFileName(outputFileName),
        answerLog("")
{}


TraCITestClient::~TraCITestClient()
{
    writeResult();
}


void
TraCITestClient::writeResult()
{
    time_t seconds;
    tm* locTime;

    std::ofstream outFile(outputFileName.c_str());
    if (!outFile) {
        cerr << "Unable to write result file" << endl;
    }
    time(&seconds);
    locTime = localtime(&seconds);
    outFile << "TraCITestClient output file. Date: " << asctime(locTime) << endl;
    outFile << answerLog.str();
    outFile.close();
}


void
TraCITestClient::errorMsg(std::stringstream& msg)
{
    cerr << msg.str() << endl;
    answerLog << "----" << endl << msg.str() << endl;
}


bool
TraCITestClient::connect(int port, std::string host)
{
    std::stringstream msg;
    socket = new tcpip::Socket(host, port);

    //socket->set_blocking(true);

    try {
        socket->connect();
    } catch (SocketException e) {
        msg << "#Error while connecting: " << e.what();
        errorMsg(msg);
        return false;
    }

    return true;
}


bool
TraCITestClient::close()
{
    if (socket != NULL) {
        socket->close();
    }
    return true;
}


bool
TraCITestClient::run(std::string fileName, int port, std::string host)
{
    std::ifstream defFile;
    std::string fileContentStr;
    std::stringstream fileContent;
    std::string lineCommand;
    std::stringstream msg;
    int repNo = 1;
    bool commentRead = false;

    if (!connect(port, host)) {
        return false;
    }

    // read definition file and trigger commands according to it
    defFile.open(fileName.c_str());

    if (!defFile) {
        msg << "Can not open definition file " << fileName << endl;
        errorMsg(msg);
        return false;
    }

    while (defFile >> lineCommand) {
        repNo = 1;
        if (lineCommand.compare("%") == 0) {
            // a comment was read
            commentRead = 1 - commentRead;
            continue;
        }
        if (commentRead) {
            // wait until end of comment is reached
            continue;
        }
        if (lineCommand.compare("repeat") == 0) {
            defFile >> repNo;
        } else if (lineCommand.compare("simstep") == 0) {
            // read parameter for command simulation step and trigger command
            double time;
            int posFormat;

            defFile >> time;
            defFile >> posFormat;
            for (int i=0; i < repNo; i++) {
                commandSimulationStep(time, posFormat);
            }
        } else if (lineCommand.compare("simstep_repeat") == 0) {
            // read parameter for command simulation step and trigger command
            // repeat the command with increasing target times
            double time;
            double time_last;
            double increment;
            int posFormat;

            defFile >> time;
            defFile >> time_last;
            defFile >> increment;
            defFile >> posFormat;
            for (int i=0; i < repNo; i++) {
                while (time <= time_last) {
                    commandSimulationStep(time, posFormat);
                    time += increment;
                }
            }
        } else if (lineCommand.compare("setmaxspeed") == 0) {
            // trigger command SetMaximumSpeed
            int nodeId;
            float maxSpeed;

            defFile >> nodeId;
            defFile >> maxSpeed;
            for (int i=0; i < repNo; i++) {
                commandSetMaximumSpeed(nodeId, maxSpeed);
            }
        } else if (lineCommand.compare("stopnode2d") == 0) {
            // trigger command StopNode with 2d position
            int nodeId;
            testclient::Position2D pos;
            float radius;
            double waitTime;

            defFile >> nodeId;
            defFile >> pos.x;
            defFile >> pos.y;
            defFile >> radius;
            defFile >> waitTime;
            commandStopNode(nodeId, pos, radius, waitTime);
        } else if (lineCommand.compare("stopnode3d") == 0) {
            // trigger command StopNode with 3d position
            int nodeId;
            testclient::Position3D pos;
            float radius;
            double waitTime;

            defFile >> nodeId;
            defFile >> pos.x;
            defFile >> pos.y;
            defFile >> pos.z;
            defFile >> radius;
            defFile >> waitTime;
            commandStopNode(nodeId, pos, radius, waitTime);
        } else if (lineCommand.compare("stopnode_roadpos") == 0) {
            // trigger command StopNode with road map position
            int nodeId;
            testclient::PositionRoadMap pos;
            float radius;
            double waitTime;

            defFile >> nodeId;
            defFile >> pos.roadId;
            defFile >> pos.pos;
            defFile >> pos.laneId;
            defFile >> radius;
            defFile >> waitTime;
            commandStopNode(nodeId, pos, radius, waitTime);
        } else if (lineCommand.compare("slowdown") == 0) {
            // trigger command slowDown
            int nodeId;
            float speed;
            double timeInterval;

            defFile >> nodeId;
            defFile >> speed;
            defFile >> timeInterval;
            commandSlowDown(nodeId, speed, timeInterval);
        } else if (lineCommand.compare("changelane") == 0) {
            // trigger command ChangeLane
            int nodeId;
            int laneId;
            float fixTime;

            defFile >> nodeId;
            defFile >> laneId;
            defFile >> fixTime;
            commandChangeLane(nodeId, laneId, fixTime);
        } else if (lineCommand.compare("changetarget") == 0) {
            // trigger command ChangeTarget
            int nodeId;
            std::string roadId;

            defFile >> nodeId;
            defFile >> roadId;
            commandChangeTarget(nodeId, roadId);
        } else if (lineCommand.compare("changeroute") == 0) {
            // trigger command ChangeRoute
            int nodeId;
            std::string roadId;
            double travelTime;

            defFile >> nodeId;
            defFile >> roadId;
            defFile >> travelTime;
            commandChangeRoute(nodeId, roadId, travelTime);
        } else if (lineCommand.compare("posconversion2d") == 0) {
            // trigger command PositionConversion for a 2d position
            testclient::Position2D pos;
            int destFormat;

            defFile >> pos.x;
            defFile >> pos.y;
            defFile >> destFormat;
            commandPositionConversion(pos, destFormat);
        } else if (lineCommand.compare("posconversion3d") == 0) {
            // trigger command PositionConversion for a 3d position
            testclient::Position3D pos;
            int destFormat;

            defFile >> pos.x;
            defFile >> pos.y;
            defFile >> pos.z;
            defFile >> destFormat;
            commandPositionConversion(pos, destFormat);
        } else if (lineCommand.compare("posconversion_roadpos") == 0) {
            // trigger command PositionConversion for a road map position
            testclient::PositionRoadMap pos;
            int destFormat;

            defFile >> pos.roadId;
            defFile >> pos.pos;
            defFile >> pos.laneId;
            defFile >> destFormat;
            commandPositionConversion(pos, destFormat);
        } else if (lineCommand.compare("distancerequest_3d_3d") == 0) {
            // trigger command DistanceRequest for 2 3D positions
            testclient::Position3D pos1;
            testclient::Position3D pos2;
            int flag;

            defFile >> pos1.x;
            defFile >> pos1.y;
            defFile >> pos1.z;
            defFile >> pos2.x;
            defFile >> pos2.y;
            defFile >> pos2.z;
            defFile >> flag;
            commandDistanceRequest(pos1, pos2, flag);
        } else if (lineCommand.compare("distancerequest_3d_roadpos") == 0) {
            // trigger command DistanceRequest for 3D and road map position
            testclient::Position3D pos1;
            testclient::PositionRoadMap pos2;
            int flag;

            defFile >> pos1.x;
            defFile >> pos1.y;
            defFile >> pos1.z;
            defFile >> pos2.roadId;
            defFile >> pos2.pos;
            defFile >> pos2.laneId;
            defFile >> flag;
            commandDistanceRequest(pos1, pos2, flag);
        } else if (lineCommand.compare("distancerequest_roadpos_3d") == 0) {
            // trigger command DistanceRequest for road map and 3D position
            testclient::PositionRoadMap pos1;
            testclient::Position3D pos2;
            int flag;

            defFile >> pos1.roadId;
            defFile >> pos1.pos;
            defFile >> pos1.laneId;
            defFile >> pos2.x;
            defFile >> pos2.y;
            defFile >> pos2.z;
            defFile >> flag;
            commandDistanceRequest(pos1, pos2, flag);
        } else if (lineCommand.compare("distancerequest_roadpos_roadpos") == 0) {
            // trigger command DistanceRequest for 2 road map positions
            testclient::PositionRoadMap pos1;
            testclient::PositionRoadMap pos2;
            int flag;

            defFile >> pos1.roadId;
            defFile >> pos1.pos;
            defFile >> pos1.laneId;
            defFile >> pos2.roadId;
            defFile >> pos2.pos;
            defFile >> pos2.laneId;
            defFile >> flag;
            commandDistanceRequest(pos1, pos2, flag);
        } else if (lineCommand.compare("scenario_novalue") == 0) {
            // trigger command Scenario, not using field "value"
            int flag;
            int domain;
            int domainId;
            int variable;
            int valueDataType;

            defFile >> flag;
            defFile >> domain;
            defFile >> domainId;
            defFile >> variable;
            defFile >> valueDataType;
            commandScenario(flag, domain, domainId, variable, valueDataType);
        } else if (lineCommand.compare("scenario_string") == 0) {
            // trigger command Scenario, giving a string value
            int flag;
            int domain;
            int domainId;
            int variable;
            std::string stringVal;

            defFile >> flag;
            defFile >> domain;
            defFile >> domainId;
            defFile >> variable;
            defFile >> stringVal;
            commandScenario(flag, domain, domainId, variable, stringVal);
        } else if (lineCommand.compare("scenario_pos3d") == 0) {
            // trigger command Scenario, giving a 3d position value
            int flag;
            int domain;
            int domainId;
            int variable;
            testclient::Position3D pos3dVal;

            defFile >> flag;
            defFile >> domain;
            defFile >> domainId;
            defFile >> variable;
            defFile >> pos3dVal.x;
            defFile >> pos3dVal.y;
            defFile >> pos3dVal.z;
            commandScenario(flag, domain, domainId, variable, pos3dVal);
        } else if (lineCommand.compare("scenario_roadpos") == 0) {
            // trigger command Scenario, giving a road map position value
            int flag;
            int domain;
            int domainId;
            int variable;
            testclient::PositionRoadMap roadPosVal;

            defFile >> flag;
            defFile >> domain;
            defFile >> domainId;
            defFile >> variable;
            defFile >> roadPosVal.roadId;
            defFile >> roadPosVal.pos;
            defFile >> roadPosVal.laneId;
            commandScenario(flag, domain, domainId, variable, roadPosVal);
        } else if (lineCommand.compare("gettlstatus") == 0) {
            // trigger command GetTrafficLightStatus
            int tlId;
            double intervalStart;
            double intervalEnd;

            defFile >> tlId;
            defFile >> intervalStart;
            defFile >> intervalEnd;
            commandGetTLStatus(tlId, intervalStart, intervalEnd);
        } else if (lineCommand.compare("getvariable") == 0) {
            // trigger command GetTrafficLightStatus
            int domID, varID;
            string objID;

            defFile >> domID;
            defFile >> varID;
            defFile >> objID;
            commandGetVariable(domID, varID, objID);
        } else {
            msg << "Error in definition file: " << lineCommand
            << " is not a valid command";
            errorMsg(msg);
            commandClose();
            close();
            return false;
        }
    }

    defFile.close();

    commandClose();

    close();

    return true;
}


bool
TraCITestClient::reportResultState(tcpip::Storage& inMsg, int command)
{
    int cmdLength;
    int cmdId;
    int resultType;
    int cmdStart;
    std::string msg;

    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        cmdId = inMsg.readUnsignedByte();
        if (cmdId != command) {
            answerLog << "#Error: received status response to command: " << cmdId
            << " but expected: " << command << endl;
            return false;
        }
        resultType = inMsg.readUnsignedByte();
        msg = inMsg.readString();
    } catch (std::invalid_argument e) {
        answerLog << "#Error: an exception was thrown while reading result state message" << endl;
        return false;
    }
    switch (resultType) {
    case RTYPE_ERR:
        answerLog << ".. Answered with error to command (" << cmdId << "), [description: " << msg << "]" << endl;
        return false;
    case RTYPE_NOTIMPLEMENTED:
        answerLog << ".. Sent command is not implemented (" << cmdId << "), [description: " << msg << "]" << endl;
        return false;
    case RTYPE_OK:
        answerLog << ".. Command acknowledged (" << cmdId << "), [description: " << msg << "]" << endl;
        break;
    default:
        answerLog << ".. Answered with unknown result code(" << resultType << ") to command(" << cmdId
        << "), [description: " << msg << "]" << endl;
        return false;
    }
    if ((cmdStart + cmdLength) != inMsg.position()) {
        answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
        return false;
    }

    return true;
}


void
TraCITestClient::commandSimulationStep(double time, int posFormat)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server";
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 8 + 1);
    // command id
    outMsg.writeUnsignedByte(CMD_SIMSTEP);
    // simulation time
    outMsg.writeDouble(time);
    // position result format
    outMsg.writeUnsignedByte(posFormat);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <SimulationStep>:" << endl << "  TargetTime=" << time
    << " PosFormat=" << posFormat << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_SIMSTEP)) {
        return;
    }

    // validate answer message
    validateSimulationStep(inMsg);
}


void
TraCITestClient::commandSetMaximumSpeed(int nodeId, float speed)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + 4);
    // command id
    outMsg.writeUnsignedByte(CMD_SETMAXSPEED);
    // node id
    outMsg.writeInt(nodeId);
    // max speed
    outMsg.writeFloat(speed);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <SetMaximumSpeed>:" << endl << "  NodeId=" << nodeId
    << " MaxSpeed=" << speed << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_SETMAXSPEED)) {
        return;
    }
}


void
TraCITestClient::commandStopNode(int nodeId, testclient::Position2D pos, float radius, double waitTime)
{
    commandStopNode(nodeId, &pos, NULL, NULL, radius, waitTime);
}


void
TraCITestClient::commandStopNode(int nodeId, testclient::Position3D pos, float radius, double waitTime)
{
    commandStopNode(nodeId, NULL, &pos, NULL, radius, waitTime);
}


void
TraCITestClient::commandStopNode(int nodeId, testclient::PositionRoadMap pos, float radius, double waitTime)
{
    commandStopNode(nodeId, NULL, NULL, &pos, radius, waitTime);
}


void
TraCITestClient::commandStopNode(int nodeId, testclient::Position2D* pos2D,
                                 testclient::Position3D* pos3D,
                                 testclient::PositionRoadMap* posRoad,
                                 float radius, double waitTime)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    tcpip::Storage tempMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command id
    tempMsg.writeUnsignedByte(CMD_STOP);
    // node id
    tempMsg.writeInt(nodeId);
    // position
    if (pos2D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_2D);
        tempMsg.writeFloat(pos2D->x);
        tempMsg.writeFloat(pos2D->y);
    } else if (pos3D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_3D);
        tempMsg.writeFloat(pos3D->x);
        tempMsg.writeFloat(pos3D->y);
        tempMsg.writeFloat(pos3D->z);
    } else if (posRoad != NULL) {
        tempMsg.writeUnsignedByte(POSITION_ROADMAP);
        tempMsg.writeString(posRoad->roadId);
        tempMsg.writeFloat(posRoad->pos);
        tempMsg.writeUnsignedByte(posRoad->laneId);
    } else {
        cerr << "Error in method commandStopNode: position is NULL" << endl;
        return;
    }
    // radius
    tempMsg.writeFloat(radius);
    // waittime
    tempMsg.writeDouble(waitTime);
    // command length
    outMsg.writeUnsignedByte(1 + tempMsg.size());
    outMsg.writeStorage(tempMsg);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <StopNode>:" << endl << "  NodeId=" << nodeId;
    if (pos2D != NULL) {
        answerLog << " Position-2D: x=" << pos2D->x << " y=" << pos2D->y ;
    } else if (pos3D != NULL) {
        answerLog << " Position-3D: x=" << pos3D->x << " y=" << pos3D->y << " z=" << pos3D->z;
    } else if (posRoad != NULL) {
        answerLog << " Position-RoadMap: roadId=" << posRoad->roadId << " pos=" << posRoad->pos << " laneId=" << (int)posRoad->laneId ;
    }
    answerLog << " radius=" << radius << " waitTime=" << waitTime << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_STOP)) {
        return;
    }

    // validate answer message
    validateStopNode(inMsg);
}


void
TraCITestClient::commandChangeLane(int nodeId, int laneId, float fixTime)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + 1 + 4);
    // command id
    outMsg.writeUnsignedByte(CMD_CHANGELANE);
    // node id
    outMsg.writeInt(nodeId);
    // lane id
    outMsg.writeUnsignedByte(laneId);
    // fix time
    outMsg.writeFloat(fixTime);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <ChangeLane>:" << endl << "  NodeId=" << nodeId
    << " LaneId=" << laneId << " fixTime=" << fixTime << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_CHANGELANE)) {
        return;
    }
}


void
TraCITestClient::commandSlowDown(int nodeId, float minSpeed, double timeInterval)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + 4 + 8);
    // command id
    outMsg.writeUnsignedByte(CMD_SLOWDOWN);
    // node id
    outMsg.writeInt(nodeId);
    // min speed
    outMsg.writeFloat(minSpeed);
    // time interval
    outMsg.writeDouble(timeInterval);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <SlowDown>:" << endl << "  NodeId=" << nodeId
    << " MinSpeed=" << minSpeed << " timeInterval" << timeInterval << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_SLOWDOWN)) {
        return;
    }
}


void
TraCITestClient::commandChangeRoute(int nodeId, std::string roadId, double travelTime)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + (4+roadId.length()) + 8);
    // command id
    outMsg.writeUnsignedByte(CMD_CHANGEROUTE);
    // node id
    outMsg.writeInt(nodeId);
    // road id
    outMsg.writeString(roadId);
    // travel time
    outMsg.writeDouble(travelTime);

    answerLog << endl << "-> Command sent: <ChangeRoute>:" << endl << "  NodeId=" << nodeId
    << " RoadId=" << roadId << " travelTime=" << travelTime << endl;

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_CHANGEROUTE)) {
        return;
    }
}


void
TraCITestClient::commandChangeTarget(int nodeId, std::string roadId)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + (4+roadId.length()));
    // command id
    outMsg.writeUnsignedByte(CMD_CHANGETARGET);
    // node id
    outMsg.writeInt(nodeId);
    // road id
    outMsg.writeString(roadId);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <ChangeTarget>:" << endl << "  NodeId=" << nodeId
    << " RoadId=" << roadId << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_CHANGETARGET)) {
        return;
    }
}


void
TraCITestClient::commandPositionConversion(testclient::Position2D pos, int posId)
{
    commandPositionConversion(&pos, NULL, NULL, posId);
}


void
TraCITestClient::commandPositionConversion(testclient::Position3D pos, int posId)
{
    commandPositionConversion(NULL, &pos, NULL, posId);
}


void
TraCITestClient::commandPositionConversion(testclient::PositionRoadMap pos, int posId)
{
    commandPositionConversion(NULL, NULL, &pos, posId);
}


void
TraCITestClient::commandPositionConversion(testclient::Position2D* pos2D,
        testclient::Position3D* pos3D,
        testclient::PositionRoadMap* posRoad,
        int posId)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    tcpip::Storage tempMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command id
    tempMsg.writeUnsignedByte(CMD_POSITIONCONVERSION);
    // position
    if (pos2D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_2D);
        tempMsg.writeFloat(pos2D->x);
        tempMsg.writeFloat(pos2D->y);
    } else if (pos3D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_3D);
        tempMsg.writeFloat(pos3D->x);
        tempMsg.writeFloat(pos3D->y);
        tempMsg.writeFloat(pos3D->z);
    } else if (posRoad != NULL) {
        tempMsg.writeUnsignedByte(POSITION_ROADMAP);
        tempMsg.writeString(posRoad->roadId);
        tempMsg.writeFloat(posRoad->pos);
        tempMsg.writeUnsignedByte(posRoad->laneId);
    } else {
        cerr << "Error in method commandPositionConversion: position is NULL" << endl;
        return;
    }
    // destination position id
    tempMsg.writeUnsignedByte(posId);
    // command length
    outMsg.writeUnsignedByte(1 + tempMsg.size());
    outMsg.writeStorage(tempMsg);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <PositionConversion>:" << endl;
    if (pos2D != NULL) {
        answerLog << " DestPosition-2D: x=" << pos2D->x << " y=" << pos2D->y ;
    } else if (pos3D != NULL) {
        answerLog << " DestPosition-3D: x=" << pos3D->x << " y=" << pos3D->y << " z=" << pos3D->z;
    } else if (posRoad != NULL) {
        answerLog << " DestPosition-RoadMap: roadId=" << posRoad->roadId << " pos=" << posRoad->pos << " laneId=" << (int)posRoad->laneId ;
    }
    answerLog << " posId=" << posId << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_POSITIONCONVERSION)) {
        return;
    }

    // validate answer message
    validatePositionConversion(inMsg);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position2D pos1, testclient::Position2D pos2, int flag)
{
    commandDistanceRequest(&pos1, NULL, NULL, &pos2, NULL, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position2D pos1, testclient::Position3D pos2, int flag)
{
    commandDistanceRequest(&pos1, NULL, NULL, NULL, &pos2, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position3D pos1, testclient::Position3D pos2, int flag)
{
    commandDistanceRequest(NULL, &pos1, NULL, NULL, &pos2, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position3D pos1, testclient::Position2D pos2, int flag)
{
    commandDistanceRequest(NULL, &pos1, NULL, &pos2, NULL, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::PositionRoadMap pos1, testclient::Position2D pos2, int flag)
{
    commandDistanceRequest(NULL, NULL, &pos1, &pos2, NULL, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::PositionRoadMap pos1, testclient::Position3D pos2, int flag)
{
    commandDistanceRequest(NULL, NULL, &pos1, NULL, &pos2, NULL, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::PositionRoadMap pos1, testclient::PositionRoadMap pos2, int flag)
{
    commandDistanceRequest(NULL, NULL, &pos1, NULL, NULL, &pos2, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position2D pos1, testclient::PositionRoadMap pos2, int flag)
{
    commandDistanceRequest(&pos1, NULL, NULL, NULL, NULL, &pos2, flag);
}


void
TraCITestClient::commandDistanceRequest(testclient::Position3D pos1, testclient::PositionRoadMap pos2, int flag)
{
    commandDistanceRequest(NULL, &pos1, NULL, NULL, NULL, &pos2, flag);
}



void
TraCITestClient::commandDistanceRequest(testclient::Position2D* pos1_2D,
                                        testclient::Position3D* pos1_3D,
                                        testclient::PositionRoadMap* pos1_Road,
                                        testclient::Position2D* pos2_2D,
                                        testclient::Position3D* pos2_3D,
                                        testclient::PositionRoadMap* pos2_Road,
                                        int flag)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    tcpip::Storage tempMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command id
    tempMsg.writeUnsignedByte(CMD_DISTANCEREQUEST);
    // position1
    if (pos1_2D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_2D);
        tempMsg.writeFloat(pos1_2D->x);
        tempMsg.writeFloat(pos1_2D->y);
    } else if (pos1_3D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_3D);
        tempMsg.writeFloat(pos1_3D->x);
        tempMsg.writeFloat(pos1_3D->y);
        tempMsg.writeFloat(pos1_3D->z);
    } else if (pos1_Road != NULL) {
        tempMsg.writeUnsignedByte(POSITION_ROADMAP);
        tempMsg.writeString(pos1_Road->roadId);
        tempMsg.writeFloat(pos1_Road->pos);
        tempMsg.writeUnsignedByte(pos1_Road->laneId);
    } else {
        cerr << "Error in method commandDistanceRequest: position1 is NULL" << endl;
        return;
    }
    // position2
    if (pos2_2D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_2D);
        tempMsg.writeFloat(pos2_2D->x);
        tempMsg.writeFloat(pos2_2D->y);
    } else if (pos2_3D != NULL) {
        tempMsg.writeUnsignedByte(POSITION_3D);
        tempMsg.writeFloat(pos2_3D->x);
        tempMsg.writeFloat(pos2_3D->y);
        tempMsg.writeFloat(pos2_3D->z);
    } else if (pos2_Road != NULL) {
        tempMsg.writeUnsignedByte(POSITION_ROADMAP);
        tempMsg.writeString(pos2_Road->roadId);
        tempMsg.writeFloat(pos2_Road->pos);
        tempMsg.writeUnsignedByte(pos2_Road->laneId);
    } else {
        cerr << "Error in method commandDistanceRequest: position2 is NULL" << endl;
        return;
    }
    // flag
    tempMsg.writeUnsignedByte(flag);
    // command length
    outMsg.writeUnsignedByte(1 + tempMsg.size());
    outMsg.writeStorage(tempMsg);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <DistanceRequest>:" << endl;
    if (pos1_2D != NULL) {
        answerLog << " FirstPosition-2D: x=" << pos1_2D->x << " y=" << pos1_2D->y ;
    } else if (pos1_3D != NULL) {
        answerLog << " FirstPosition-3D: x=" << pos1_3D->x << " y=" << pos1_3D->y << " z=" << pos1_3D->z;
    } else if (pos1_Road != NULL) {
        answerLog << " FirstPosition-RoadMap: roadId=" << pos1_Road->roadId << " pos=" << pos1_Road->pos << " laneId=" << (int)pos1_Road->laneId ;
    }
    if (pos2_2D != NULL) {
        answerLog << " SecondPosition-2D: x=" << pos2_2D->x << " y=" << pos2_2D->y ;
    } else if (pos2_3D != NULL) {
        answerLog << " SecondPosition-3D: x=" << pos2_3D->x << " y=" << pos2_3D->y << " z=" << pos2_3D->z;
    } else if (pos2_Road != NULL) {
        answerLog << " SecondPosition-RoadMap: roadId=" << pos2_Road->roadId << " pos=" << pos2_Road->pos << " laneId=" << (int)pos2_Road->laneId ;
    }
    answerLog << " Flag=" << flag << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_DISTANCEREQUEST)) {
        return;
    }

    // validate answer message
    validateDistanceRequest(inMsg);
}


void
TraCITestClient::commandGetTLStatus(int tlId, double intervalStart, double intervalEnd)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4 + 8 + 8);
    // command id
    outMsg.writeUnsignedByte(CMD_GETTLSTATUS);
    // tl id
    outMsg.writeInt(tlId);
    // interval start
    outMsg.writeDouble(intervalStart);
    // interval end
    outMsg.writeDouble(intervalEnd);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <GetTLStatus>:" << endl
    << "  TLId=" << tlId << " IntervalStart=" << intervalStart
    << " IntervalEnd=" << intervalEnd << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_GETTLSTATUS)) {
        return;
    }

    // validate response
    validateGetTLStatus(inMsg);
}



void 
TraCITestClient::commandGetVariable(int domID, int varID, const std::string &objID)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;
    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }
    // command length
    outMsg.writeUnsignedByte(1 + 1 + 1 + 4 + objID.length());
    // command id
    outMsg.writeUnsignedByte(domID);
    // variable id
    outMsg.writeUnsignedByte(varID);
    // object id
    outMsg.writeString(objID);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }
    answerLog << endl << "-> Command sent: <GetVariable>:" << endl
    << "  domID=" << domID << " varID=" << varID
    << " objID=" << objID << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
        if (!reportResultState(inMsg, domID)) {
            return;
        }
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }
    // validate result state
    try {
        int respStart = inMsg.position();
        int extLength = inMsg.readUnsignedByte();
        int respLength = inMsg.readInt();
        int cmdId = inMsg.readUnsignedByte();
        if (cmdId != (domID+0x10)) {
            answerLog << "#Error: received response with command id: " << cmdId
                << "but expected: " << (int)(domID+0x10) << endl;
            return;
        }
        answerLog << "  CommandID=" << cmdId;
        answerLog << "  VariableID=" << inMsg.readUnsignedByte();
        answerLog << "  ObjectID=" << inMsg.readString();
        int valueDataType = inMsg.readUnsignedByte();
        answerLog << " valueDataType=" << valueDataType;
        readAndReportTypeDependent(inMsg, valueDataType);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    /*
    answerLog << "  ObjectID=" << inMsg.readString();
    /*
    // read value
    int valueDataType = inMsg.readUnsignedByte();
    answerLog << " valueDataType=" << valueDataType;
    readAndReportTypeDependent(inMsg, valueDataType);
    */
}


void
TraCITestClient::commandClose()
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command length
    outMsg.writeUnsignedByte(1 + 1);
    // command id
    outMsg.writeUnsignedByte(CMD_CLOSE);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <Close>:" << endl;

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_CLOSE)) {
        return;
    }
}


void
TraCITestClient::commandScenario(int flag, int domain, int domainId, int variable, int valueDataType)
{
    commandScenario(flag, domain, domainId, variable, valueDataType, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL, NULL, NULL);
}


void
TraCITestClient::commandScenario(int flag, int domain, int domainId, int variable, std::string stringVal)
{
    commandScenario(flag, domain, domainId, variable, 0, NULL, NULL, NULL, NULL, NULL, &stringVal,
                    NULL, NULL, NULL, NULL, NULL);
}


void
TraCITestClient::commandScenario(int flag, int domain, int domainId, int variable, testclient::Position3D pos3dVal)
{
    commandScenario(flag, domain, domainId, variable, 0, NULL, NULL, NULL, NULL, NULL, NULL,
                    &pos3dVal, NULL, NULL, NULL, NULL);
}


void
TraCITestClient::commandScenario(int flag, int domain, int domainId, int variable, testclient::PositionRoadMap roadPosVal)
{
    commandScenario(flag, domain, domainId, variable, 0, NULL, NULL, NULL, NULL, NULL, NULL,
                    NULL, &roadPosVal, NULL, NULL, NULL);
}


void
TraCITestClient::commandScenario(int flag, int domain, int domainId, int variable, int valueDataType,
                                 int* intVal, int* byteVal, int* ubyteVal,
                                 float* floatVal, double* doubleVal, std::string* stringVal,
                                 testclient::Position3D* pos3dVal, testclient::PositionRoadMap* roadPosVal,
                                 testclient::BoundingBox* boxVal, testclient::Polygon* polyVal,
                                 testclient::TLPhaseList* tlphaseVal)
{
    tcpip::Storage outMsg;
    tcpip::Storage inMsg;
    tcpip::Storage tempMsg;
    std::stringstream valueString;
    std::stringstream msg;

    if (socket == NULL) {
        msg << "#Error while sending command: no connection to server" ;
        errorMsg(msg);
        return;
    }

    // command id
    tempMsg.writeUnsignedByte(CMD_SCENARIO);
    // flag
    tempMsg.writeUnsignedByte(flag);
    // domain
    tempMsg.writeUnsignedByte(domain);
    // domain id
    tempMsg.writeInt(domainId);
    // variable
    tempMsg.writeUnsignedByte(variable);
    // value
    if (intVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt(*intVal);
        valueString << " IntegerValue=" << *intVal;
    } else if (byteVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_BYTE);
        tempMsg.writeByte(*byteVal);
        valueString << " ByteValue=" << *byteVal;
    } else if (ubyteVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_UBYTE);
        tempMsg.writeUnsignedByte(*ubyteVal);
        valueString << " UByteValue=" << *ubyteVal;
    } else if (floatVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_FLOAT);
        tempMsg.writeFloat(*floatVal);
        valueString << " FloatValue=" << *floatVal;
    } else if (doubleVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_DOUBLE);
        tempMsg.writeDouble(*doubleVal);
        valueString << " DoubleValue=" << *doubleVal;
    } else if (stringVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_STRING);
        tempMsg.writeString(*stringVal);
        valueString << " StringValue=" << *stringVal;
    } else if (pos3dVal != NULL) {
        tempMsg.writeUnsignedByte(POSITION_3D);
        tempMsg.writeFloat(pos3dVal->x);
        tempMsg.writeFloat(pos3dVal->y);
        tempMsg.writeFloat(pos3dVal->z);
        valueString << std::endl << " Position3DValue: x="
        << pos3dVal->x << " y=" << pos3dVal->y << " z=" << pos3dVal->z;
    } else if (roadPosVal != NULL) {
        tempMsg.writeUnsignedByte(POSITION_ROADMAP);
        tempMsg.writeString(roadPosVal->roadId);
        tempMsg.writeFloat(roadPosVal->pos);
        tempMsg.writeUnsignedByte(roadPosVal->laneId);
        valueString << std::endl << " RoadMapPositionValue: roadId=" << roadPosVal->roadId
        << " pos=" << roadPosVal->pos << " laneId=" << (int)roadPosVal->laneId;
    } else if (boxVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_BOUNDINGBOX);
        tempMsg.writeFloat(boxVal->lowerLeft.x);
        tempMsg.writeFloat(boxVal->lowerLeft.y);
        tempMsg.writeFloat(boxVal->upperRight.x);
        tempMsg.writeFloat(boxVal->lowerLeft.y);
        valueString << std::endl << " BoundaryBoxValue: lowerleft x=" << boxVal->lowerLeft.x << " y=" << boxVal->lowerLeft.y
        << "upperRight x=" << boxVal->upperRight.x << " y=" << boxVal->upperRight.y;
    } else if (polyVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_POLYGON);
        tempMsg.writeUnsignedByte(polyVal->size());
        valueString << std::endl << " PolygonValue: " << std::endl;
        for (testclient::Polygon::iterator it = polyVal->begin(); it != polyVal->end(); it++) {
            tempMsg.writeFloat(it->x);
            tempMsg.writeFloat(it->y);
            valueString << " (" << it->x << "," << it->y << "), " << std::endl;
        }
    } else if (tlphaseVal != NULL) {
        tempMsg.writeUnsignedByte(TYPE_TLPHASELIST);
        tempMsg.writeUnsignedByte(tlphaseVal->size());
        valueString << std::endl << " TLPhaseListValue: " << std::endl;
        for (testclient::TLPhaseList::iterator it = tlphaseVal->begin(); it != tlphaseVal->end(); it++) {
            tempMsg.writeString(it->precRoadId);
            tempMsg.writeString(it->succRoadId);
            tempMsg.writeUnsignedByte(it->phase);
            valueString << " (precRoad=" << it->precRoadId << " succRoad=" << it->succRoadId
            << " phase=" << it->phase << ")" << std::endl;
        }
    } else {
        // value data type (with out value)
        tempMsg.writeUnsignedByte(valueDataType);
        valueString << " [no value sent]";
    }

    // command length
    outMsg.writeUnsignedByte(1 + tempMsg.size());
    // whole message
    outMsg.writeStorage(tempMsg);

    // send request message
    try {
        socket->sendExact(outMsg);
    } catch (SocketException e) {
        msg << "Error while sending command: " << e.what();
        errorMsg(msg);
        return;
    }

    answerLog << endl << "-> Command sent: <Scenario>:" << endl << "  flag=" << flag
    << " domain=" << domain << " domainId=" << domainId
    << " variable=" << variable << " valueDataType=" << valueDataType
    << valueString.str() << endl;
    writeResult();

    // receive answer message
    try {
        socket->receiveExact(inMsg);
    } catch (SocketException e) {
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }

    // validate result state
    if (!reportResultState(inMsg, CMD_SCENARIO)) {
        return;
    }

    // validate response
    validateScenario(inMsg);
}


bool
TraCITestClient::validateSimulationStep(tcpip::Storage &inMsg)
{
    int cmdId;
    int cmdLength;
    int nodeId;
    double targetTime;
    int posType;
    int cmdStart;
    testclient::Position2D pos2D;
    testclient::Position3D pos3D;
    testclient::PositionRoadMap roadPos;

    while (inMsg.valid_pos()) {
        try {
            cmdStart = inMsg.position();
            cmdLength = inMsg.readUnsignedByte();
            cmdId = inMsg.readUnsignedByte();
            if (cmdId != CMD_MOVENODE) {
                answerLog << "#Error: received response with command id: " << cmdId
                << "but expected: " << (int)CMD_MOVENODE << endl;
                return false;
            }
            answerLog << ".. Received Response <MoveNode>:" << endl;
            nodeId = inMsg.readInt();
            answerLog << "  nodeId=" << nodeId << " ";
            targetTime = inMsg.readDouble();
            answerLog << "targetTime=" << targetTime << " ";
            posType = inMsg.readUnsignedByte();
            switch (posType) {
            case POSITION_2D:
                pos2D.x = inMsg.readFloat();
                pos2D.y = inMsg.readFloat();
                answerLog << "2D-Position: x=" << pos2D.x << " y=" << pos2D.y << endl;
                break;
            case POSITION_3D:
            case POSITION_2_5D:
                if (posType == POSITION_2_5D) {
                    answerLog << "2.5D-Position: ";
                } else {
                    answerLog << "3D-Position: ";
                }
                pos3D.x = inMsg.readFloat();
                pos3D.y = inMsg.readFloat();
                pos3D.z = inMsg.readFloat();
                answerLog << "x=" << pos3D.x << " y=" << pos3D.y << " z=" << pos3D.z << endl;
                break;
            case POSITION_ROADMAP:
                roadPos.roadId = inMsg.readString();
                roadPos.pos = inMsg.readFloat();
                roadPos.laneId = inMsg.readUnsignedByte();
                answerLog << "RoadMap-Position: roadId=" << roadPos.roadId << " pos=" << roadPos.pos
                << " laneId=" << (int)roadPos.laneId << endl;
                break;
            default:
                answerLog << "#Error: received unknown position format" << endl;
                return false;
            }
            if ((cmdStart + cmdLength) != inMsg.position()) {
                answerLog << "#Warning: command at position " << cmdStart << " has wrong length" << endl;
                //return false;
            }
        } catch (std::invalid_argument e) {
            answerLog << "#Error while reading message:" << e.what() << endl;
            return false;
        }
    }

    return true;
}


bool
TraCITestClient::validateStopNode(tcpip::Storage &inMsg)
{
    int cmdId;
    int cmdLength;
    int rNodeId;
    float rRadius;
    double rWaitTime;
    int rPosType;
    int cmdStart;
    testclient::PositionRoadMap rRoadPos;

    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        // read command id
        cmdId = inMsg.readUnsignedByte();
        if (cmdId != CMD_STOP) {
            answerLog << "#Error: received response with command id: " << cmdId
            << "but expected: " << (int)CMD_STOP << endl;
            return false;
        }
        answerLog << ".. Received Response <StopNode>:" << endl;
        // read nodeID
        rNodeId = inMsg.readInt();
        answerLog << "  nodeId=" << rNodeId << " ";
        // read stop position
        rPosType = inMsg.readUnsignedByte();
        if (rPosType != POSITION_ROADMAP) {
            answerLog << "#Error: received position was not in road map format" << endl;
            return false;
        }
        rRoadPos.roadId = inMsg.readString();
        rRoadPos.pos = inMsg.readFloat();
        rRoadPos.laneId = inMsg.readUnsignedByte();
        answerLog << "Stop position: roadId=" << rRoadPos.roadId
        << " pos=" << rRoadPos.pos << " laneId=" << (int)rRoadPos.laneId;
        // read radius
        rRadius = inMsg.readFloat();
        answerLog << " radius=" << rRadius;
        // read wait time
        rWaitTime = inMsg.readDouble();
        answerLog << " wait time=" << rWaitTime << endl;
        // check command length
        if ((cmdStart + cmdLength) != inMsg.position()) {
            answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
            return false;
        }
    } catch (std::invalid_argument e) {
        answerLog << "#Error while reading message:" << e.what() << endl;
        return false;
    }

    return true;
}


bool
TraCITestClient::validatePositionConversion(tcpip::Storage &inMsg)
{
    int cmdId;
    int cmdLength;
    int posType;
    int reqPosType;
    int cmdStart;
    testclient::PositionRoadMap roadPos;
    testclient::Position2D pos2D;
    testclient::Position3D pos3D;

    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        // read command id
        cmdId = inMsg.readUnsignedByte();
        if (cmdId != CMD_POSITIONCONVERSION) {
            answerLog << "#Error: received response with command id: " << cmdId
            << "but expected: " << (int)CMD_POSITIONCONVERSION << endl;
            return false;
        }
        answerLog << ".. Received Response <PositionConversion>:" << endl;
        // read converted position
        posType = inMsg.readUnsignedByte();
        switch (posType) {
        case POSITION_2D:
            pos2D.x = inMsg.readFloat();
            pos2D.y = inMsg.readFloat();
            answerLog << "2D-Position: x=" << pos2D.x << " y=" << pos2D.y << endl;
            break;
        case POSITION_3D:
        case POSITION_2_5D:
            if (posType == POSITION_2_5D) {
                answerLog << "2.5D-Position: ";
            } else {
                answerLog << "3D-Position: ";
            }
            pos3D.x = inMsg.readFloat();
            pos3D.y = inMsg.readFloat();
            pos3D.z = inMsg.readFloat();
            answerLog << "x=" << pos3D.x << " y=" << pos3D.y << " z=" << pos3D.z << endl;
            break;
        case POSITION_ROADMAP:
            roadPos.roadId = inMsg.readString();
            roadPos.pos = inMsg.readFloat();
            roadPos.laneId = inMsg.readUnsignedByte();
            answerLog << "RoadMap-Position: roadId=" << roadPos.roadId << " pos=" << roadPos.pos
            << " laneId=" << (int)roadPos.laneId << endl;
            break;
        default:
            answerLog << "#Error: received unknown position format" << endl;
            return false;
        }
        // read requested position type
        reqPosType = inMsg.readUnsignedByte();
        if (reqPosType != posType) {
            answerLog << "#Warning: requested position type (" << reqPosType
            << ") and received position type (" << posType << ") do not match" << endl;
        }
        // check command length
        if ((cmdStart + cmdLength) != inMsg.position()) {
            answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
            return false;
        }
    } catch (std::invalid_argument e) {
        answerLog << "#Error while reading message:" << e.what() << endl;
        return false;
    }

    return true;
}


bool
TraCITestClient::validateDistanceRequest(tcpip::Storage& inMsg)
{
    int cmdId;
    int cmdLength;
    int flag;
    int cmdStart;
    float distance;
    /*testclient::PositionRoadMap roadPos;
    testclient::Position2D pos2D;
    testclient::Position3D pos3D;*/

    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        // read command id
        cmdId = inMsg.readUnsignedByte();
        if (cmdId != CMD_DISTANCEREQUEST) {
            answerLog << "#Error: received response with command id: " << cmdId
            << "but expected: " << (int)CMD_DISTANCEREQUEST << endl;
            return false;
        }
        answerLog << ".. Received Response <DistanceRequest>:" << endl;
        // read flag
        flag = inMsg.readUnsignedByte();
        answerLog << " flag=" << flag;
        // read computed distance
        distance = inMsg.readFloat();
        answerLog << " distance=" << distance << endl;
        //// read computed position
        //posType = inMsg.readUnsignedByte();
        //switch (posType) {
        //case POSITION_2D:
        //	pos2D.x = inMsg.readFloat();
        //	pos2D.y = inMsg.readFloat();
        //	answerLog << "2D-Position: x=" << pos2D.x << " y=" << pos2D.y;
        //	break;
        //case POSITION_3D:
        //case POSITION_2_5D:
        //	if (posType == POSITION_2_5D) {
        //		answerLog << "2.5D-Position: ";
        //	} else {
        //		answerLog << "3D-Position: ";
        //	}
        //	pos3D.x = inMsg.readFloat();
        //	pos3D.y = inMsg.readFloat();
        //	pos3D.z = inMsg.readFloat();
        //	answerLog << "x=" << pos3D.x << " y=" << pos3D.y << " z=" << pos3D.z;
        //	break;
        //case POSITION_ROADMAP:
        //	roadPos.roadId = inMsg.readString();
        //	roadPos.pos = inMsg.readFloat();
        //	roadPos.laneId = inMsg.readUnsignedByte();
        //	answerLog << "RoadMap-Position: roadId=" << roadPos.roadId << " pos=" << roadPos.pos
        //		<< " laneId=" << (int)roadPos.laneId;
        //	break;
        //default:
        //	answerLog << "#Error: received unknown position format: " << posType << endl;
        //	return false;
        //}
        // check command length
        if ((cmdStart + cmdLength) != inMsg.position()) {
            answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
            return false;
        }
    } catch (std::invalid_argument e) {
        answerLog << "#Error while reading message:" << e.what() << endl;
        return false;
    }

    return true;
}


bool
TraCITestClient::validateScenario(tcpip::Storage &inMsg)
{
    int cmdId;
    int cmdLength;
    int flag;
    int domain;
    int domainId;
    int variable;
    int valueDataType;
    int cmdStart;
    testclient::PositionRoadMap rRoadPos;

    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        // read command id
        cmdId = inMsg.readUnsignedByte();
        if (cmdId != CMD_SCENARIO) {
            answerLog << "#Error: received response with command id: " << cmdId
            << "but expected: " << (int)CMD_SCENARIO << endl;
            return false;
        }
        answerLog << ".. Received Response <Scenario>:" << endl;
        // read flag
        flag = inMsg.readUnsignedByte();
        answerLog << "  flag=" << flag << " ";
        // read domain
        domain = inMsg.readUnsignedByte();
        answerLog << " domain=" << domain;
        // read domain id
        domainId = inMsg.readInt();
        answerLog << " domainId=" << domainId;
        // read variable
        variable = inMsg.readUnsignedByte();
        answerLog << " variable=" << variable;
        // value data type
        valueDataType = inMsg.readUnsignedByte();
        answerLog << " valueDataType=" << valueDataType;
        // read value
        if(!readAndReportTypeDependent(inMsg, valueDataType)) {
            return false;
        }
        // check command length
        if ((cmdStart + cmdLength) != inMsg.position()) {
            answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
            return false;
        }
    } catch (std::invalid_argument e) {
        answerLog << "#Error while reading message:" << e.what() << endl;
        return false;
    }
    return true;
}

bool
TraCITestClient::readAndReportTypeDependent(tcpip::Storage &inMsg, int valueDataType)
{
    if (valueDataType == TYPE_UBYTE) {
        int ubyte = inMsg.readUnsignedByte();
        answerLog << " Unsigned Byte Value: " << ubyte << endl;
    } else if (valueDataType == TYPE_BYTE) {
        int byte = inMsg.readByte();
        answerLog << " Byte value: " << byte << endl;
    } else if (valueDataType == TYPE_INTEGER) {
        int integer = inMsg.readInt();
        answerLog << " Int value: " << integer << endl;
    } else if (valueDataType == TYPE_FLOAT) {
        float floatv = inMsg.readFloat();
        answerLog << " float value: " << floatv << endl;
    } else if (valueDataType == TYPE_DOUBLE) {
        double doublev = inMsg.readDouble();
        answerLog << " Double value: " << doublev << endl;
    } else if (valueDataType == TYPE_BOUNDINGBOX) {
        testclient::BoundingBox box;
        box.lowerLeft.x = inMsg.readFloat();
        box.lowerLeft.y = inMsg.readFloat();
        box.upperRight.x = inMsg.readFloat();
        box.upperRight.y = inMsg.readFloat();
        answerLog << " BoundaryBoxValue: lowerLeft x="<< box.lowerLeft.x
            << " y=" << box.lowerLeft.y << " upperRight x=" << box.upperRight.x
            << " y=" << box.upperRight.y << endl;
    } else if (valueDataType == TYPE_POLYGON) {
        int length = inMsg.readUnsignedByte();
        answerLog << " PolygonValue: ";
        for (int i=0; i < length; i++) {
            float x = inMsg.readFloat();
            float y = inMsg.readFloat();
            answerLog << "(" << x << "," << y << ") ";
        }
        answerLog << endl;
    } else if (valueDataType == POSITION_3D) {
        float x = inMsg.readFloat();
        float y = inMsg.readFloat();
        float z = inMsg.readFloat();
        answerLog << " Position3DValue: " << std::endl;
        answerLog << " x: " << x << " y: " << y
            << " z: " << z << std::endl;
    } else if (valueDataType == POSITION_ROADMAP) {
        std::string roadId = inMsg.readString();
        float pos = inMsg.readFloat();
        int laneId = inMsg.readUnsignedByte();
        answerLog << " RoadMapPositionValue: roadId=" << roadId
            << " pos=" << pos
            << " laneId=" << laneId << std::endl;
    } else if (valueDataType == TYPE_TLPHASELIST) {
        int length = inMsg.readUnsignedByte();
        answerLog << " TLPhaseListValue: length=" << length << endl;
        for (int i=0; i< length; i++) {
            string pred = inMsg.readString();
            string succ = inMsg.readString();
            int phase = inMsg.readUnsignedByte();
            answerLog << " precRoad=" << pred << " succRoad=" << succ
                << " phase=";
            switch (phase) {
            case TLPHASE_RED:
                answerLog << "red" << endl;
                break;
            case TLPHASE_YELLOW:
                answerLog << "yellow" << endl;
                break;
            case TLPHASE_GREEN:
                answerLog << "green" << endl;
                break;
            default:
                answerLog << "#Error: unknown phase value" << (int)phase << endl;
                return false;
            }
        }
    } else if (valueDataType == TYPE_STRING) {
        string s = inMsg.readString();
        answerLog << " string value: " << s << endl;
    } else if (valueDataType == TYPE_STRINGLIST) {
        vector<string> s = inMsg.readStringList();
        answerLog << " string list value: [ " << endl;
        for(vector<string>::iterator i=s.begin(); i!=s.end(); ++i) {
            if(i!=s.begin()) {
                answerLog << ", ";
            }
            answerLog << '"' << *i << '"';
        }
        answerLog << " ]" << endl;
    } else {
        answerLog << "#Error: unknown valueDataType!" << endl;
        return false;
    }
    return true;
}


bool
TraCITestClient::validateGetTLStatus(tcpip::Storage &inMsg)
{
    int cmdId;
    int cmdLength;
    int cmdStart;
    double switchTime;
    std::string precEdge;
    std::string succEdge;
    float posOnEdge;
    int newPhase;
    double yellowTime;

    while (inMsg.valid_pos()) {
        try {
            cmdStart = inMsg.position();
            cmdLength = inMsg.readUnsignedByte();
            // read command id
            cmdId = inMsg.readUnsignedByte();
            if (cmdId != CMD_TLSWITCH) {
                answerLog << "#Error: received response with command id: " << cmdId
                << "but expected: " << (int)CMD_TLSWITCH << endl;
                return false;
            }
            answerLog << ".. Received Response <TrafficLightSwitch>:" << endl;
            // read switch time
            switchTime = inMsg.readDouble();
            answerLog << "  SwitchTime=" << switchTime;
            // read preceeding edge id
            precEdge = inMsg.readString();
            answerLog << " PrecEdge=" << precEdge;
            // read position on preceeding edge
            posOnEdge = inMsg.readFloat();
            answerLog << " PosOnPrecEdge=" << posOnEdge;
            // read succeeding edge id
            succEdge = inMsg.readString();
            answerLog << " SuccEdge=" << succEdge;
            // read new phase
            newPhase = inMsg.readUnsignedByte();
            answerLog << " NewPhase=" << newPhase;
            // read yellow time
            yellowTime = inMsg.readDouble();
            answerLog << " YellowTime=" << yellowTime << endl;
            // check command length
            if ((cmdStart + cmdLength) != inMsg.position()) {
                answerLog << "#Error: command at position " << cmdStart << " has wrong length" << endl;
                return false;
            }
        } catch (std::invalid_argument e) {
            answerLog << "#Error while reading message:" << e.what() << endl;
            return false;
        }
    }

    return true;
}
