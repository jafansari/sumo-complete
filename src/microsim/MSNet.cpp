/****************************************************************************/
/// @file    MSNet.cpp
/// @author  Christian Roessel
/// @date    Tue, 06 Mar 2001
/// @version $Id$
///
// The simulated network and simulation perfomer
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

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <sstream>
#include <typeinfo>
#include <algorithm>
#include <cassert>
#include <vector>
#include <sstream>
#include <utils/common/UtilExceptions.h>
#include "MSNet.h"
#include "MSEdgeControl.h"
#include "MSJunctionControl.h"
#include "MSEmitControl.h"
#include "MSEventControl.h"
#include "MSEdge.h"
#include "MSJunction.h"
#include "MSJunctionLogic.h"
#include "MSLane.h"
#include "MSVehicleTransfer.h"
#include "MSRoute.h"
#include "MSRouteLoaderControl.h"
#include "traffic_lights/MSTLLogicControl.h"
#include "MSVehicleControl.h"
#include "trigger/MSTrigger.h"
#include "MSCORN.h"
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/MSVehicleTransfer.h>
#include "traffic_lights/MSTrafficLightLogic.h"
#include <utils/shapes/Polygon2D.h>
#include <utils/shapes/ShapeContainer.h>
#include "output/MSXMLRawOut.h"
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/SysUtils.h>
#include <utils/common/WrappingCommand.h>
#include <utils/options/OptionsCont.h>
#include "MSGlobals.h"
#include "MSRouteHandler.h"
#include "MSRouteLoader.h"
#include <utils/geom/GeoConvHelper.h>
#include <ctime>
#include "MSPerson.h"
#include "MSEdgeWeightsStorage.h"


#ifdef _MESSAGES
#include "MSMessageEmitter.h"
#endif

#ifdef HAVE_MESOSIM
#include <mesosim/MELoop.h>
#endif

#ifndef NO_TRACI
#include <traci-server/TraCIServer.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// static member definitions
// ===========================================================================
MSNet* MSNet::myInstance = 0;


// ===========================================================================
// member method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// MSNet::EdgeWeightsProxi - methods
// ---------------------------------------------------------------------------
SUMOReal
MSNet::EdgeWeightsProxi::getEffort(const MSEdge * const e,
                                   const SUMOVehicle * const v,
                                   SUMOTime t) const {
    SUMOReal value;
    if (myVehicleKnowledge.retrieveExistingEffort(e, v, t, value)) {
        return value;
    }
    if (myNetKnowledge.retrieveExistingEffort(e, v, t, value)) {
        return value;
    }
    return 0;
}


SUMOReal
MSNet::EdgeWeightsProxi::getTravelTime(const MSEdge * const e,
                                       const SUMOVehicle * const v,
                                       SUMOTime t) const {
    SUMOReal value;
    if (myVehicleKnowledge.retrieveExistingTravelTime(e, v, t, value)) {
        return value;
    }
    if (myNetKnowledge.retrieveExistingTravelTime(e, v, t, value)) {
        return value;
    }
    const MSLane * const l = e->getLanes()[0];
    return l->getLength() / l->getMaxSpeed();
}



// ---------------------------------------------------------------------------
// MSNet - methods
// ---------------------------------------------------------------------------
MSNet*
MSNet::getInstance(void) throw(ProcessError) {
    if (myInstance != 0) {
        return myInstance;
    }
    throw ProcessError("A network was not yet constructed.");
}


MSNet::MSNet(MSVehicleControl *vc, MSEventControl *beginOfTimestepEvents,
             MSEventControl *endOfTimestepEvents, MSEventControl *emissionEvents,
             ShapeContainer *shapeCont) throw(ProcessError) {
    if (myInstance!=0) {
        throw ProcessError("A network was already constructed.");
    }
    MSCORN::init();
    OptionsCont &oc = OptionsCont::getOptions();
    myStep = (SUMOTime) oc.getInt("begin"); // !!! SUMOTime-option
    myLogExecutionTime = !oc.getBool("no-duration-log");
    myLogStepNumber = !oc.getBool("no-step-log");
    myTooManyVehicles = oc.getInt("too-many-vehicles");
    myEmitter = new MSEmitControl(*vc, (SUMOTime) oc.getInt("max-depart-delay"), oc.getBool("sloppy-emit"));// !!! SUMOTime-option
    myVehicleControl = vc;
    myDetectorControl = new MSDetectorControl();
    myEdges = 0;
    myJunctions = 0;
    myRouteLoaders = 0;
    myLogics = 0;
    myPersonControl = 0;
    myEdgeWeights = 0;
    myShapeContainer = shapeCont==0 ? new ShapeContainer() : shapeCont;

    myBeginOfTimestepEvents = beginOfTimestepEvents;
    myEndOfTimestepEvents = endOfTimestepEvents;
    myEmissionEvents = emissionEvents;

#ifdef HAVE_MESOSIM
    if (MSGlobals::gUseMesoSim) {
        MSGlobals::gMesoNet = new MELoop();
    }
#endif
    myInstance = this;
}




void
MSNet::closeBuilding(MSEdgeControl *edges, MSJunctionControl *junctions,
                     MSRouteLoaderControl *routeLoaders,
                     MSTLLogicControl *tlc,
                     std::vector<int> stateDumpTimes,
                     std::string stateDumpFiles) throw() {
    myEdges = edges;
    myJunctions = junctions;
    myRouteLoaders = routeLoaders;
    myLogics = tlc;
    // save the time the network state shall be saved at
    myStateDumpTimes = stateDumpTimes;
    myStateDumpFiles = stateDumpFiles;

    // set requests/responses
    myJunctions->postloadInitContainer();
    // initialise outputs
    if (OptionsCont::getOptions().isSet("emissions-output")) {
        MSCORN::setWished(MSCORN::CORN_OUT_EMISSIONS);
    }
    if (OptionsCont::getOptions().isSet("tripinfo-output")) {
        MSCORN::setWished(MSCORN::CORN_OUT_TRIPDURATIONS);
    }
    if (OptionsCont::getOptions().isSet("vehroute-output")) {
        MSCORN::setWished(MSCORN::CORN_OUT_VEHROUTES);
    }

    // initialise performance computation
    if (myLogExecutionTime) {
        mySimBeginMillis = SysUtils::getCurrentMillis();
    }
}


MSNet::~MSNet() throw() {
    // delete controls
    delete myJunctions;
    delete myDetectorControl;
    // delete mean data
    delete myEdges;
    delete myEmitter;
    delete myLogics;
    delete myRouteLoaders;
    delete myVehicleControl;
    delete myShapeContainer;
#ifdef _MESSAGES
#ifdef _DEBUG
    cout << "MSNet: clearing myMsgEmitter" << endl;
#endif
    myMsgEmitter.clear();
#ifdef _DEBUG
    cout << "MSNet: clearing msgEmitVec" << endl;
#endif
    msgEmitVec.clear();
#endif
#ifdef HAVE_MESOSIM
    if (MSGlobals::gUseMesoSim) {
        delete MSGlobals::gMesoNet;
    }
#endif
    delete myBeginOfTimestepEvents;
    delete myEndOfTimestepEvents;
    delete myEmissionEvents;
    delete myEdgeWeights;
    clearAll();
    GeoConvHelper::close();
    myInstance = 0;
}


int
MSNet::simulate(SUMOTime start, SUMOTime stop) {
    // the simulation loop
    std::string quitMessage = "";
    myStep = start;
    do {
        if (myLogStepNumber) {
            preSimStepOutput();
        }
        simulationStep();
        if (myLogStepNumber) {
            postSimStepOutput();
        }
        if (myTooManyVehicles>0&&(int) myVehicleControl->getRunningVehicleNo()>myTooManyVehicles) {
            quitMessage = "Simulation End: Too many vehicles.";
        }
#ifndef NO_TRACI
        if (traci::TraCIServer::wasClosed()) {
            quitMessage = "Simulation End: TraCI requested termination.";
        }
        if (stop == INT_MAX && OptionsCont::getOptions().getInt("remote-port") == 0 && myVehicleControl->haveAllVehiclesQuit() && !myEmitter->hasPendingFlows()) {
#else
        if (stop == INT_MAX && myVehicleControl->haveAllVehiclesQuit() && !myEmitter->hasPendingFlows()) {
#endif
            if (myEmissionEvents->isEmpty()) {
                quitMessage = "Simulation End: All vehicles have left the simulation.";
            }
        }
        if (myStep >= stop) {
            quitMessage = "Simulation End: The final simulation step has been reached.";
        }
    }
    while (quitMessage=="");
    WRITE_MESSAGE(quitMessage);
    // exit simulation loop
    closeSimulation(start);
    return 0;
}


void
MSNet::closeSimulation(SUMOTime start) {
    if (myLogExecutionTime) {
        long duration = SysUtils::getCurrentMillis() - mySimBeginMillis;
        std::ostringstream msg;
        msg << "Performance: " << "\n" << " Duration: " << duration << " ms" << "\n";
        if (duration!=0) {
            msg << " Real time factor: " << ((SUMOReal)(myStep-start)*1000./(SUMOReal)duration) << "\n";
            msg.setf(std::ios::fixed , std::ios::floatfield);    // use decimal format
            msg.setf(std::ios::showpoint);    // print decimal point
            msg << " UPS: " << ((SUMOReal) myVehiclesMoved * 1000. / (SUMOReal) duration) << "\n";
        }
        msg << "Vehicles: " << "\n"
        << " Emitted: " << myVehicleControl->getEmittedVehicleNo() << "\n"
        << " Running: " << myVehicleControl->getRunningVehicleNo() << "\n"
        << " Waiting: " << myEmitter->getWaitingVehicleNo() << "\n";
        WRITE_MESSAGE(msg.str());
    }
    myDetectorControl->close(myStep);
#ifndef NO_TRACI
    traci::TraCIServer::close();
#endif
}


void
MSNet::simulationStep() {
#ifndef NO_TRACI
    traci::TraCIServer::processCommandsUntilSimStep(myStep);
#endif
    // execute beginOfTimestepEvents
    if (myLogExecutionTime) {
        mySimStepBegin = SysUtils::getCurrentMillis();
    }
#ifdef HAVE_MESOSIM
    // netstate output
    if (find(myStateDumpTimes.begin(), myStateDumpTimes.end(), myStep)!=myStateDumpTimes.end()) {
        std::string name = myStateDumpFiles + '_' + toString(myStep) + ".bin";
        std::ofstream strm(name.c_str(), std::fstream::out|std::fstream::binary);
        saveState(strm);
    }
#endif
    myBeginOfTimestepEvents->execute(myStep);
    if (MSGlobals::gCheck4Accidents) {
        myEdges->detectCollisions(myStep);
    }
#ifdef HAVE_MESOSIM
    if (MSGlobals::gUseMesoSim) {
        myJunctions->setAllowed();
        MSGlobals::gMesoNet->simulate(myStep);
    } else {
#endif
        myJunctions->resetRequests();
        // check whether the tls shall be switched
        myLogics->check2Switch(myStep);
        // set information about which vehicles may drive at all
        myLogics->setTrafficLightSignals();


        // assure all lanes with vehicles are 'active'
        myEdges->patchActiveLanes();

        // move vehicles
        //  precompute possible positions for vehicles that do interact with
        //   their lane's end
        myEdges->moveCritical(myStep);

        // check the right-of-way for all junctions
        myJunctions->setAllowed();

        // move vehicles which do interact with their lane's end
        //  (it is now known whether they may drive
        myEdges->moveFirst(myStep);
        if (MSGlobals::gCheck4Accidents) {
            myEdges->detectCollisions(myStep);
        }

        // Vehicles change Lanes (maybe)
        myEdges->changeLanes(myStep);

        if (MSGlobals::gCheck4Accidents) {
            myEdges->detectCollisions(myStep);
        }
#ifdef HAVE_MESOSIM
    }
#endif
    // load routes
    myRouteLoaders->loadNext(myStep, myEmitter);
    // emit Vehicles
    myEmitter->emitVehicles(myStep);
    if (MSGlobals::gCheck4Accidents) {
        myEdges->detectCollisions(myStep);
    }
    MSVehicleTransfer::getInstance()->checkEmissions(myStep);
    myEmissionEvents->execute(myStep);

    // persons
    if (myPersonControl!=0) {
        if (myPersonControl->hasWaitingPersons(myStep)) {
            const MSPersonControl::PersonVector &persons = myPersonControl->getWaitingPersons(myStep);
            for (MSPersonControl::PersonVector::const_iterator i=persons.begin(); i!=persons.end(); ++i) {
                MSPerson *person = *i;
                if (person->endReached()) {
                    delete person;
                } else {
                    person->proceed(this, myStep);
                }
            }
        }
    }

    // execute endOfTimestepEvents
    myEndOfTimestepEvents->execute(myStep);

    // update and write (if needed) detector values
    writeOutput();

    if (myLogExecutionTime) {
        mySimStepEnd = SysUtils::getCurrentMillis();
        mySimStepDuration = mySimStepEnd - mySimStepBegin;
        myVehiclesMoved += myVehicleControl->getRunningVehicleNo();
    }
    myStep += DELTA_T;
}


void
MSNet::clearAll() {
    // clear container
    MSEdge::clear();
    MSLane::clear();
    MSRoute::clear();
    delete MSVehicleTransfer::getInstance();
}


SUMOTime
MSNet::getCurrentTimeStep() const {
    return myStep;
}


void
MSNet::writeOutput() {
    // update detector values
    myDetectorControl->updateDetectors(myStep);
    // check state dumps
    if (OptionsCont::getOptions().isSet("netstate-dump")) {
        MSXMLRawOut::write(OutputDevice::getDeviceByOption("netstate-dump"), *myEdges, myStep);
    }
    // emission output
    if (OptionsCont::getOptions().isSet("emissions-output")) {
        OutputDevice::getDeviceByOption("emissions-output")
        << "    <emission-state time=\"" << myStep << "\" "
        << "loaded=\"" << myVehicleControl->getLoadedVehicleNo() << "\" "
        << "emitted=\"" << myVehicleControl->getEmittedVehicleNo() << "\" "
        << "running=\"" << myVehicleControl->getRunningVehicleNo() << "\" "
        << "waiting=\"" << myEmitter->getWaitingVehicleNo() << "\" "
        << "ended=\"" << myVehicleControl->getEndedVehicleNo() << "\" "
        << "meanWaitingTime=\"" << myVehicleControl->getMeanWaitingTime() << "\" "
        << "meanTravelTime=\"" << myVehicleControl->getMeanTravelTime() << "\" ";
        if (myLogExecutionTime) {
            OutputDevice::getDeviceByOption("emissions-output")
            << "duration=\"" << mySimStepDuration << "\" ";
        }
        OutputDevice::getDeviceByOption("emissions-output") << "/>\n";
    }
    // write detector values
    myDetectorControl->writeOutput(myStep + DELTA_T, false);
}


bool
MSNet::logSimulationDuration() const throw() {
    return myLogExecutionTime;
}


#ifdef HAVE_MESOSIM
void
MSNet::saveState(std::ostream &os) throw() {
    FileHelpers::writeString(os, VERSION_STRING);
    FileHelpers::writeUInt(os, sizeof(size_t));
    FileHelpers::writeUInt(os, sizeof(SUMOReal));
    FileHelpers::writeUInt(os, MSEdge::dictSize());
    FileHelpers::writeUInt(os, myStep);
    myVehicleControl->saveState(os);
    if (MSGlobals::gUseMesoSim) {
        MSGlobals::gMesoNet->saveState(os);
    }
}


unsigned int
MSNet::loadState(BinaryInputDevice &bis) throw() {
    std::string version;
    unsigned int sizeT, fpSize, numEdges, step;
    bis >> version;
    bis >> sizeT;
    bis >> fpSize;
    bis >> numEdges;
    bis >> step;
    if (version != VERSION_STRING) {
        WRITE_WARNING("State was written with sumo version " + version + " (present: " + VERSION_STRING +")!");
    }
    if (sizeT != sizeof(size_t)) {
        WRITE_WARNING("State was written on a different platform (32bit vs. 64bit)!");
    }
    if (fpSize != sizeof(SUMOReal)) {
        WRITE_WARNING("State was written with a different precision for SUMOReal!");
    }
    if (numEdges != MSEdge::dictSize()) {
        WRITE_WARNING("State was written for a different net!");
    }
    myVehicleControl->loadState(bis);
    if (MSGlobals::gUseMesoSim) {
        MSGlobals::gMesoNet->loadState(bis, *myVehicleControl);
    }
    return step;
}
#endif


MSPersonControl &
MSNet::getPersonControl() throw() {
    if (myPersonControl==0) {
        myPersonControl = new MSPersonControl();
    }
    return *myPersonControl;
}


MSEdgeWeightsStorage &
MSNet::getWeightsStorage() throw() {
    if (myEdgeWeights==0) {
        myEdgeWeights = new MSEdgeWeightsStorage();
    }
    return *myEdgeWeights;
}


void
MSNet::preSimStepOutput() const throw() {
    std::cout << std::setprecision(OUTPUT_ACCURACY);
    std::cout << "Step #" << myStep;
}


void
MSNet::postSimStepOutput() const throw() {
    if (myLogExecutionTime) {
        std::string msg;
        std::ostringstream oss;
        oss.setf(std::ios::fixed , std::ios::floatfield);    // use decimal format
        oss.setf(std::ios::showpoint);    // print decimal point
        oss << std::setprecision(OUTPUT_ACCURACY);
        if (mySimStepDuration!=0) {
            oss << " (" << mySimStepDuration << "ms ~= "
            << (1000./ (SUMOReal) mySimStepDuration) << "*RT, ~"
            << ((SUMOReal) myVehicleControl->getRunningVehicleNo()/(SUMOReal) mySimStepDuration*1000.);
        } else {
            oss << " (0ms ?*RT. ?";
        }
        oss << "UPS, vehicles"
        << " TOT " << myVehicleControl->getEmittedVehicleNo()
        << " ACT " << myVehicleControl->getRunningVehicleNo()
        << ")                                              ";
        msg = oss.str();
        std::string prev = "Step #" + toString(myStep-DELTA_T);
        msg = msg.substr(0, 78 - prev.length());
        std::cout << msg;
    }
    std::cout << (char) 13;
}


void
MSNet::addVehicleStateListener(VehicleStateListener *listener) throw() {
    if (find(myVehicleStateListeners.begin(), myVehicleStateListeners.end(), listener)==myVehicleStateListeners.end()) {
        myVehicleStateListeners.push_back(listener);
    }
}


void
MSNet::removeVehicleStateListener(VehicleStateListener *listener) throw() {
    std::vector<VehicleStateListener*>::iterator i= find(myVehicleStateListeners.begin(), myVehicleStateListeners.end(), listener);
    if (i!=myVehicleStateListeners.end()) {
        myVehicleStateListeners.erase(i);
    }
}


void
MSNet::informVehicleStateListener(const MSVehicle * const vehicle, VehicleState to) throw() {
    for (std::vector<VehicleStateListener*>::iterator i=myVehicleStateListeners.begin(); i!=myVehicleStateListeners.end(); ++i) {
        (*i)->vehicleStateChanged(vehicle, to);
    }
}



#ifdef _MESSAGES
MSMessageEmitter*
MSNet::getMsgEmitter(const std::string& whatemit) {
    msgEmitVec.clear();
    msgEmitVec = myMsgEmitter.buildAndGetStaticVector();
    MSMessageEmitter *msgEmitter = 0;
    for (int i = 0; i < msgEmitVec.size(); ++i) {
        if (msgEmitVec.at(i)->getEventsEnabled(whatemit)) {
            msgEmitter = msgEmitVec.at(i);
            break;
        }
    }
    // returns 0 if the requested MessageEmitter is not in the map
    return msgEmitter;
}


void
MSNet::createMsgEmitter(std::string& id,
                        std::string& file,
                        const std::string& base,
                        std::string& whatemit,
                        bool reverse,
                        bool table,
                        bool xy,
                        SUMOReal step) {
    MSMessageEmitter *msgEmitter = new MSMessageEmitter(file, base, whatemit, reverse, table, xy, step);
    myMsgEmitter.add(id, msgEmitter);
}
#endif


/****************************************************************************/

