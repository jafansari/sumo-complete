/****************************************************************************/
/// @file    MSVehicle.cpp
/// @author  Christian Roessel
/// @date    Mon, 05 Mar 2001
/// @version $Id$
///
// Representation of a vehicle in the micro simulation
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

#include "MSLane.h"
#include "MSVehicle.h"
#include "MSEdge.h"
#include "MSVehicleType.h"
#include "MSNet.h"
#include "MSRoute.h"
#include "MSLinkCont.h"
#include "MSVehicleQuitReminded.h"
#include <utils/common/StringUtils.h>
#include <utils/common/StdDefs.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSGlobals.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <map>
#include "MSMoveReminder.h"
#include <utils/options/OptionsCont.h>
#include "MSLCM_DK2004.h"
#include <utils/common/ToString.h>
#include <utils/common/FileHelpers.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/iodevices/BinaryInputDevice.h>
#include "trigger/MSBusStop.h"
#include <utils/common/DijkstraRouterTT.h>
#include "MSPerson.h"
#include "MSPersonControl.h"
#include <utils/common/RandHelper.h>
#include "devices/MSDevice_Routing.h"
#include <microsim/devices/MSDevice_HBEFA.h>
#include "MSEdgeWeightsStorage.h"
#include <utils/common/HelpersHBEFA.h>
#include <utils/common/HelpersHarmonoise.h>

#ifdef _MESSAGES
#include "MSMessageEmitter.h"
#endif

#ifdef HAVE_MESOSIM
#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>
#include "MSGlobals.h"
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS

//#define DEBUG_VEHICLE_GUI_SELECTION 1
#ifdef DEBUG_VEHICLE_GUI_SELECTION
#include <utils/gui/div/GUIGlobalSelection.h>
#include <guisim/GUIVehicle.h>
#include <guisim/GUILane.h>
#endif

#define BUS_STOP_OFFSET 0.5


// ===========================================================================
// static value definitions
// ===========================================================================
std::vector<MSLane*> MSVehicle::myEmptyLaneVector;


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * methods of MSVehicle::State
 * ----------------------------------------------------------------------- */
MSVehicle::State::State(const State& state) {
    myPos = state.myPos;
    mySpeed = state.mySpeed;
}


MSVehicle::State&
MSVehicle::State::operator=(const State& state) {
    myPos   = state.myPos;
    mySpeed = state.mySpeed;
    return *this;
}


bool
MSVehicle::State::operator!=(const State& state) {
    return (myPos   != state.myPos ||
            mySpeed != state.mySpeed);
}


SUMOReal
MSVehicle::State::pos() const {
    return myPos;
}


MSVehicle::State::State(SUMOReal pos, SUMOReal speed) :
        myPos(pos), mySpeed(speed) {}


/* -------------------------------------------------------------------------
 * MSVehicle-methods
 * ----------------------------------------------------------------------- */
MSVehicle::~MSVehicle() throw() {
    // remove move reminder
    for (QuitRemindedVector::iterator i=myQuitReminded.begin(); i!=myQuitReminded.end(); ++i) {
        (*i)->removeOnTripEnd(this);
    }
    // delete the route
    myRoute->release();
    // delete values in CORN
    if (myPointerCORNMap.find(MSCORN::CORN_P_VEH_OLDROUTE)!=myPointerCORNMap.end()) {
        ReplacedRoutesVector *v = (ReplacedRoutesVector*) myPointerCORNMap[MSCORN::CORN_P_VEH_OLDROUTE];
        for (ReplacedRoutesVector::iterator i=v->begin(); i!=v->end(); ++i) {
            (*i).route->release();
        }
        delete v;
    }
    if (myPointerCORNMap.find(MSCORN::CORN_P_VEH_DEPART_INFO)!=myPointerCORNMap.end()) {
        DepartArrivalInformation *i = (DepartArrivalInformation*) myPointerCORNMap[MSCORN::CORN_P_VEH_DEPART_INFO];
        delete i;
    }
    if (myPointerCORNMap.find(MSCORN::CORN_P_VEH_ARRIVAL_INFO)!=myPointerCORNMap.end()) {
        DepartArrivalInformation *i = (DepartArrivalInformation*) myPointerCORNMap[MSCORN::CORN_P_VEH_ARRIVAL_INFO];
        delete i;
    }
    //
    delete myParameter;
    delete myLaneChangeModel;
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        delete(*dev);
    }
    myDevices.clear();
    // persons
    if (hasCORNPointerValue(MSCORN::CORN_P_VEH_PASSENGER)) {
        std::vector<MSPerson*> *persons = (std::vector<MSPerson*>*) myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER];
        for (std::vector<MSPerson*>::iterator i=persons->begin(); i!=persons->end(); ++i) {
            (*i)->proceed(MSNet::getInstance(), MSNet::getInstance()->getCurrentTimeStep());
        }
        delete persons;
    }
    // other
    delete myEdgeWeights;
    for (std::vector<MSLane*>::iterator i=myFurtherLanes.begin(); i!=myFurtherLanes.end(); ++i) {
        (*i)->resetPartialOccupation(this);
    }
    for (DriveItemVector::iterator i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end(); ++i) {
        if ((*i).myLink!=0) {
            (*i).myLink->removeApproaching(this);
        }
    }
    myFurtherLanes.clear();
}


MSVehicle::MSVehicle(SUMOVehicleParameter* pars,
                     const MSRoute* route,
                     const MSVehicleType* type,
                     int vehicleIndex) throw(ProcessError) :
        myLastLaneChangeOffset(0),
        myTarget(0),
        myWaitingTime(0),
        myParameter(pars),
        myRoute(route),
        myState(0, 0), //
        myIndividualMaxSpeed(0.0),
        myHasIndividualMaxSpeed(false),
        myReferenceSpeed(-1.0),
        myLane(0),
        myType(type),
        myLastBestLanesEdge(0),
        myCurrEdge(myRoute->begin()),
        myMoveReminders(0),
        myOldLaneMoveReminders(0),
        myOldLaneMoveReminderOffsets(0),
        myArrivalPos(pars->arrivalPos),
        myPreDawdleAcceleration(0),
        myEdgeWeights(0),
		mySignals(0),
        myLastGreenTime(86400*1000)
#ifndef NO_TRACI
        ,adaptingSpeed(false),
        isLastAdaption(false),
        speedBeforeAdaption(0),
        speedWithoutTraciInfluence(0),
        timeBeforeAdaption(0),
        speedReduction(0),
        adaptDuration(0),
        timeBeforeLaneChange(0),
        laneChangeStickyTime(0),
        laneChangeConstraintActive(false),
		myTraCISpeed(-1),
        myDestinationLane(0)
#endif
{
    for (std::vector<SUMOVehicleParameter::Stop>::iterator i=pars->stops.begin(); i!=pars->stops.end(); ++i) {
        if (!addStop(*i)) {
            throw ProcessError("Stop for vehicle '" + pars->id +
                               "' on lane '" + i->lane + "' is not downstream the current route.");
        }
    }
    for (std::vector<SUMOVehicleParameter::Stop>::const_iterator i=route->getStops().begin(); i!=route->getStops().end(); ++i) {
        if (!addStop(*i)) {
            throw ProcessError("Stop for vehicle '" + pars->id +
                               "' on lane '" + i->lane + "' is not downstream the current route.");
        }
    }
#ifdef _MESSAGES
    myLCMsgEmitter = MSNet::getInstance()->getMsgEmitter("lanechange");
    myBMsgEmitter = MSNet::getInstance()->getMsgEmitter("break");
    myHBMsgEmitter = MSNet::getInstance()->getMsgEmitter("heartbeat");
#endif
    // build arrival definition
    SUMOReal lastLaneLength = (myRoute->getLastEdge()->getLanes())[0]->getLength();
    if (myArrivalPos < 0) {
        myArrivalPos += lastLaneLength; // !!! validate!
    }
    if (myArrivalPos<0) {
        myArrivalPos = 0;
    }
    if (myArrivalPos>lastLaneLength) {
        myArrivalPos = lastLaneLength;
    }
    MSDevice_Routing::buildVehicleDevices(*this, myDevices);
    myLaneChangeModel = new MSLCM_DK2004(*this);
    // init devices
    MSDevice_HBEFA::buildVehicleDevices(*this, myDevices);
    // init CORN containers
    if (MSCORN::wished(MSCORN::CORN_VEH_WAITINGTIME)) {
        myIntCORNMap[MSCORN::CORN_VEH_WAITINGTIME] = 0;
    }
    if ((*myCurrEdge)->getDepartLane(*this) == 0) {
        delete myLaneChangeModel;
        throw ProcessError("Invalid departlane definition for vehicle '" + pars->id + "'");
    }
    myRoute->addReference();
}


// ------------ interaction with the route
void
MSVehicle::onTryEmit() throw() {
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->onTryEmit();
    }
}


void
MSVehicle::onDepart() throw() {
    // check whether the vehicle's departure time shall be saved
    if (MSCORN::wished(MSCORN::CORN_VEH_DEPART_TIME)) {
        myIntCORNMap[MSCORN::CORN_VEH_DEPART_TIME] = (int) MSNet::getInstance()->getCurrentTimeStep();
    }
    // check whether the vehicle's verbose departure information shall be saved
    if (MSCORN::wished(MSCORN::CORN_VEH_DEPART_INFO)) {
        DepartArrivalInformation *i = new DepartArrivalInformation();
        i->time = MSNet::getInstance()->getCurrentTimeStep();
        i->lane = myLane;
        i->pos = myState.pos();
        i->speed = myState.speed();
        myPointerCORNMap[MSCORN::CORN_P_VEH_DEPART_INFO] = (void*) i;
    }
    if (hasCORNPointerValue(MSCORN::CORN_P_VEH_PASSENGER)) {
        std::vector<MSPerson*> *persons = (std::vector<MSPerson*>*) myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER];
        for (std::vector<MSPerson*>::iterator i=persons->begin(); i!=persons->end(); ++i) {
            (*i)->setDeparted(MSNet::getInstance()->getCurrentTimeStep());
        }
    }
    // inform the vehicle control
    MSNet::getInstance()->getVehicleControl().vehicleEmitted(*this);
}


void
MSVehicle::onRemovalFromNet(bool forTeleporting) throw() {
    // check whether the vehicle's verbose arrival information shall be saved
    if (!forTeleporting && MSCORN::wished(MSCORN::CORN_VEH_ARRIVAL_INFO)) {
        DepartArrivalInformation *i = new DepartArrivalInformation();
        i->time = MSNet::getInstance()->getCurrentTimeStep();
        i->lane = myLane;
        i->pos = myState.pos();
        i->speed = myState.speed();
        myPointerCORNMap[MSCORN::CORN_P_VEH_ARRIVAL_INFO] = (void*) i;
    }
    SUMOReal pspeed = myState.mySpeed;
    SUMOReal pos = myState.myPos;
    SUMOReal oldPos = pos - SPEED2DIST(pspeed);
    // process reminder
    workOnMoveReminders(oldPos, pos, pspeed);
    // remove from structures to be informed about it
    for (QuitRemindedVector::iterator i=myQuitReminded.begin(); i!=myQuitReminded.end(); ++i) {
        (*i)->removeOnTripEnd(this);
    }
    myQuitReminded.clear();
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->onRemovalFromNet();
    }
    for (DriveItemVector::iterator i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end(); ++i) {
        if ((*i).myLink!=0) {
            (*i).myLink->removeApproaching(this);
        }
    }
    leaveLane(true);
}


// ------------ interaction with the route
const MSEdge*
MSVehicle::succEdge(unsigned int nSuccs) const throw() {
    if (hasSuccEdge(nSuccs)) {
        return *(myCurrEdge + nSuccs);
    } else {
        return 0;
    }
}


bool
MSVehicle::ends() const throw() {
    return myCurrEdge==myRoute->end()-1 && myState.myPos > myArrivalPos - POSITION_EPS;
}


const MSRoute &
MSVehicle::getRoute(int index) const throw() {
    if (index==0) {
        return *myRoute;
    }
    --index; // only prior routes are stored
    std::map<MSCORN::Pointer, void*>::const_iterator i = myPointerCORNMap.find(MSCORN::CORN_P_VEH_OLDROUTE);
    assert(i!=myPointerCORNMap.end());
    const ReplacedRoutesVector * const v = (const ReplacedRoutesVector * const)(*i).second;
    assert((int) v->size()>index);
    return *((*v)[index].route);
}


bool
MSVehicle::replaceRoute(const MSEdgeVector &edges, SUMOTime simTime, bool onInit) throw() {
    // build a new id, first
    std::string id = getID();
    if (id[0]!='!') {
        id = "!" + id;
    }
    if (myRoute->getID().find("!var#")!=std::string::npos) {
        id = myRoute->getID().substr(0, myRoute->getID().rfind("!var#")+4) + toString(myIntCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] + 1);
    } else {
        id = id + "!var#1";
    }
    MSRoute *newRoute = new MSRoute(id, edges, 0, myRoute->getColor(), myRoute->getStops());
    if (!MSRoute::dictionary(id, newRoute)) {
        delete newRoute;
        return false;
    }
    if (!replaceRoute(newRoute, simTime, onInit)) {
        newRoute->addReference();
        newRoute->release();
        return false;
    }
    return true;
}


bool
MSVehicle::replaceRoute(const MSRoute* newRoute, SUMOTime simTime, bool onInit) throw() {
    const MSEdgeVector &edges = newRoute->getEdges();
    // assert the vehicle may continue (must not be "teleported" or whatever to another position)
    MSRouteIterator currEdge2 = find(edges.begin(), edges.end(), *myCurrEdge);
    if (!onInit && currEdge2==edges.end()) {
        return false;
    }
    // check whether the new route is the same as the prior
    MSRouteIterator ri = myCurrEdge;
    while (ri!=myRoute->end()&&currEdge2!=edges.end()&&*ri==*currEdge2) {
        ++ri;
        ++currEdge2;
    }
    if (ri==myRoute->end()&&currEdge2==edges.end()) {
        return true;
    }

    // ... maybe the route information shall be saved for output?
    if (MSCORN::wished(MSCORN::CORN_VEH_SAVEREROUTING)) {
        myRoute->addReference();
        RouteReplaceInfo rri(*myCurrEdge, simTime, myRoute);
        if (myPointerCORNMap.find(MSCORN::CORN_P_VEH_OLDROUTE)==myPointerCORNMap.end()) {
            myPointerCORNMap[MSCORN::CORN_P_VEH_OLDROUTE] = new ReplacedRoutesVector();
        }
        ((ReplacedRoutesVector*) myPointerCORNMap[MSCORN::CORN_P_VEH_OLDROUTE])->push_back(rri);
    }

    // rebuild in-vehicle route information
    if (onInit) {
        myCurrEdge = newRoute->begin();
    } else {
        myCurrEdge = newRoute->find(*myCurrEdge);
    }
    // check whether the old route may be deleted (is not used by anyone else)
    myRoute->release();
    newRoute->addReference();
    // assign new route
    myRoute = newRoute;
    myLastBestLanesEdge = 0;
    // update arrival definition
    myArrivalPos = myParameter->arrivalPos;
    SUMOReal lastLaneLength = (myRoute->getLastEdge()->getLanes())[0]->getLength();
    if (myArrivalPos < 0) {
        myArrivalPos += lastLaneLength; // !!! validate!
    }
    if (myArrivalPos<0) {
        myArrivalPos = 0;
    }
    if (myArrivalPos>lastLaneLength) {
        myArrivalPos = lastLaneLength;
    }
    // save information that the vehicle was rerouted
    //  !!! refactor the CORN-stuff
    myIntCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] = 0;
    myIntCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] = myIntCORNMap[MSCORN::CORN_VEH_NUMBERROUTE] + 1;
    // recheck stops
    for (std::list<Stop>::iterator iter = myStops.begin(); iter != myStops.end();) {
        if (find(edges.begin(), edges.end(), &iter->lane->getEdge())==edges.end()) {
            iter = myStops.erase(iter);
        } else {
            ++iter;
        }
    }
    return true;
}


bool
MSVehicle::willPass(const MSEdge * const edge) const throw() {
    return find(myCurrEdge, myRoute->end(), edge)!=myRoute->end();
}


void
MSVehicle::reroute(SUMOTime t, SUMOAbstractRouter<MSEdge, SUMOVehicle> &router, bool withTaz) throw() {
    // check whether to reroute
    std::vector<const MSEdge*> edges;
    if (withTaz && MSEdge::dictionary(myParameter->fromTaz+"-source") && MSEdge::dictionary(myParameter->toTaz+"-sink")) {
        router.compute(MSEdge::dictionary(myParameter->fromTaz+"-source"), MSEdge::dictionary(myParameter->toTaz+"-sink"), (const MSVehicle * const) this, t, edges);
        if (edges.size() >= 2) {
            edges.erase(edges.begin());
            edges.pop_back();
        }
    } else {
        router.compute(*myCurrEdge, myRoute->getLastEdge(), (const MSVehicle * const) this, t, edges);
    }
    if (edges.empty()) {
        WRITE_WARNING("No route for vehicle '" + getID() + "' found.");
        return;
    }
    replaceRoute(edges, MSNet::getInstance()->getCurrentTimeStep(), withTaz);
}


MSEdgeWeightsStorage &
MSVehicle::getWeightsStorage() throw() {
    if (myEdgeWeights==0) {
        myEdgeWeights = new MSEdgeWeightsStorage();
    }
    return *myEdgeWeights;
}


bool
MSVehicle::hasValidRoute(std::string &msg) const throw() {
    MSRouteIterator last = myRoute->end() - 1;
    // check connectivity, first
    for (MSRouteIterator e=myCurrEdge; e!=last; ++e) {
        if ((*e)->allowedLanes(**(e+1), myType->getVehicleClass())==0) {
            msg = "No connection between '" + (*e)->getID() + "' and '" + (*(e+1))->getID() + "'.";
            return false;
        }
    }
    last = myRoute->end();
    // check usable lanes, then
    for (MSRouteIterator e=myCurrEdge; e!=last; ++e) {
        if ((*e)->prohibits(this)) {
            msg = "Edge '" + (*e)->getID() + "' prohibits.";
            return false;
        }
    }
    return true;
}


// ------------ Retrieval of CORN values
int
MSVehicle::getCORNIntValue(MSCORN::Function f) const throw() {
    return myIntCORNMap.find(f)->second;
}


void *
MSVehicle::getCORNPointerValue(MSCORN::Pointer p) const throw() {
    assert(myPointerCORNMap.find(p)!=myPointerCORNMap.end());
    return myPointerCORNMap.find(p)->second;
}


bool
MSVehicle::hasCORNIntValue(MSCORN::Function f) const throw() {
    return myIntCORNMap.find(f)!=myIntCORNMap.end();
}


bool
MSVehicle::hasCORNPointerValue(MSCORN::Pointer p) const throw() {
    return myPointerCORNMap.find(p)!=myPointerCORNMap.end();
}




// ------------ Interaction with move reminders
SUMOReal
MSVehicle::getPositionOnActiveMoveReminderLane(const MSLane * const searchedLane) const throw() {
    if (searchedLane==myLane) {
        return myState.myPos;
    }
    std::vector< MSMoveReminder* >::const_iterator rem = myOldLaneMoveReminders.begin();
    std::vector<SUMOReal>::const_iterator off = myOldLaneMoveReminderOffsets.begin();
    for (; rem!=myOldLaneMoveReminders.end()&&off!=myOldLaneMoveReminderOffsets.end(); ++rem, ++off) {
        if ((*rem)->getLane()==searchedLane) {
            return (*off) + myState.myPos;
        }
    }
    return -1;
}


void
MSVehicle::workOnMoveReminders(SUMOReal oldPos, SUMOReal newPos, SUMOReal newSpeed) throw() {
    // This erasure-idiom works for all stl-sequence-containers
    // See Meyers: Effective STL, Item 9
    for (std::vector< MSMoveReminder* >::iterator rem=myMoveReminders.begin(); rem!=myMoveReminders.end();) {
        if (!(*rem)->isStillActive(*this, oldPos, newPos, newSpeed)) {
            rem = myMoveReminders.erase(rem);
        } else {
            ++rem;
        }
    }
    OffsetVector::iterator off=myOldLaneMoveReminderOffsets.begin();
    for (std::vector< MSMoveReminder* >::iterator rem=myOldLaneMoveReminders.begin(); rem!=myOldLaneMoveReminders.end();) {
        SUMOReal oldLaneLength = *off;
        if (!(*rem)->isStillActive(*this, oldLaneLength+oldPos, oldLaneLength+newPos, newSpeed)) {
            rem = myOldLaneMoveReminders.erase(rem);
            off = myOldLaneMoveReminderOffsets.erase(off);
        } else {
            ++rem;
            ++off;
        }
    }
}


void
MSVehicle::adaptLaneEntering2MoveReminder(const MSLane &enteredLane) throw() {
    // save the old work reminders, patching the position information
    //  add the information about the new offset to the old lane reminders
    SUMOReal oldLaneLength = myLane->getLength();
    OffsetVector::iterator i;
    for (i=myOldLaneMoveReminderOffsets.begin(); i!=myOldLaneMoveReminderOffsets.end(); ++i) {
        (*i) += oldLaneLength;
    }
    for (size_t j=0; j<myMoveReminders.size(); j++) {
        myOldLaneMoveReminderOffsets.push_back(oldLaneLength);
    }
    copy(myMoveReminders.begin(), myMoveReminders.end(), back_inserter(myOldLaneMoveReminders));
    assert(myOldLaneMoveReminders.size()==myOldLaneMoveReminderOffsets.size());
    // get new move reminder
    myMoveReminders = enteredLane.getMoveReminders();
}


void
MSVehicle::activateReminders(bool isEmit, bool isLaneChange) throw() {
    // This erasure-idiom works for all stl-sequence-containers
    // See Meyers: Effective STL, Item 9
    for (std::vector< MSMoveReminder* >::iterator rem=myMoveReminders.begin(); rem!=myMoveReminders.end();) {
        if (!(*rem)->notifyEnter(*this, isEmit, isLaneChange)) {
            rem = myMoveReminders.erase(rem);
        } else {
            ++rem;
        }
    }
}


void
MSVehicle::addReminder(MSMoveReminder* rem) throw() {
    myMoveReminders.push_back(rem);
}


// ------------
bool
MSVehicle::addStop(const SUMOVehicleParameter::Stop &stopPar, SUMOTime untilOffset) throw() {
    Stop stop;
    stop.lane = MSLane::dictionary(stopPar.lane);
    stop.busstop = MSNet::getInstance()->getBusStop(stopPar.busstop);
    stop.startPos = stopPar.startPos;
    stop.endPos = stopPar.endPos;
    stop.duration = stopPar.duration;
    stop.until = stopPar.until;
    if (stop.until != -1) {
        stop.until += untilOffset;
    }
    stop.triggered = stopPar.triggered;
    stop.parking = stopPar.parking;
    stop.reached = false;
    if (stop.startPos < 0 || stop.endPos > stop.lane->getLength()) {
        return false;
    }
    stop.edge = myRoute->find(&stop.lane->getEdge(), myCurrEdge);
    MSRouteIterator prevStopEdge = myCurrEdge;
    SUMOReal prevStopPos = myState.myPos;
    // where to insert the stop
    std::list<Stop>::iterator iter = myStops.begin();
    if (stopPar.index == STOP_INDEX_END || stopPar.index >= static_cast<int>(myStops.size())) {
        if (myStops.size() > 0) {
            prevStopEdge = myStops.back().edge;
            prevStopPos = myStops.back().endPos;
            iter = myStops.end();
            stop.edge = myRoute->find(&stop.lane->getEdge(), prevStopEdge);
        }
    } else {
        if (stopPar.index == STOP_INDEX_FIT) {
            while (iter != myStops.end() && (iter->edge < stop.edge ||
                                             (iter->endPos < stop.endPos && iter->edge == stop.edge))) {
                prevStopEdge = iter->edge;
                prevStopPos = iter->endPos;
                ++iter;
            }
        } else {
            int index = stopPar.index;
            while (index > 0) {
                prevStopEdge = iter->edge;
                prevStopPos = iter->endPos;
                ++iter;
                --index;
            }
            stop.edge = myRoute->find(&stop.lane->getEdge(), prevStopEdge);
        }
    }
    if (stop.edge == myRoute->end() || prevStopEdge > stop.edge || prevStopEdge == stop.edge && prevStopPos > stop.endPos) {
        return false;
    }
    if (myCurrEdge == stop.edge && myState.myPos > stop.endPos - getCarFollowModel().brakeGap(myState.mySpeed)) {
        return false;
    }
    myStops.insert(iter, stop);
    return true;
}


bool
MSVehicle::isStopped() {
    return !myStops.empty() && myStops.begin()->reached && myStops.begin()->duration>0;
}


SUMOReal
MSVehicle::processNextStop(SUMOReal currentVelocity) throw() {
    if (myStops.empty()) {
        // no stops; pass
        return currentVelocity;
    }
    if (myStops.begin()->reached) {
        // ok, we have already reached the next stop
        if (myStops.begin()->duration==0) {
            // ... and have waited as long as needed
            if (myStops.begin()->busstop!=0) {
                // inform bus stop about leaving it
                myStops.begin()->busstop->leaveFrom(this);
            }
            // the current stop is no longer valid
            MSNet::getInstance()->getVehicleControl().removeWaiting(&myLane->getEdge(), this);
            if (hasCORNPointerValue(MSCORN::CORN_P_VEH_PASSENGER)) {
                std::vector<MSPerson*> *persons = (std::vector<MSPerson*>*) myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER];
                for (std::vector<MSPerson*>::iterator i=persons->begin(); i!=persons->end(); ++i) {
                    (*i)->setDeparted(MSNet::getInstance()->getCurrentTimeStep());
                }
            }
            myStops.pop_front();
            // maybe the next stop is on the same edge; let's rebuild best lanes
            getBestLanes(true);
            // continue as wished...
        } else {
            // we have to wait some more time
            myStops.begin()->duration -= DELTA_T;
            return 0;
        }
    } else {
        // is the next stop on the current lane?
        if (myStops.begin()->edge==myCurrEdge && myStops.begin()->lane==myLane) {
            Stop &bstop = myStops.front();
            // get the stopping position
            SUMOReal endPos = bstop.endPos;
//			SUMOReal offset = 0.1;
            bool busStopsMustHaveSpace = true;
            if (bstop.busstop!=0) {
                // on bus stops, we have to wait for free place if they are in use...
//				offset = BUS_STOP_OFFSET;
                endPos = bstop.busstop->getLastFreePos();
                if (endPos-5.<bstop.busstop->getBeginLanePosition()) { // !!! explicite offset
                    busStopsMustHaveSpace = false;
                }
            }
//            if (myState.pos()>=endPos-offset&&busStopsMustHaveSpace) {
            if (myState.pos()>=endPos-BUS_STOP_OFFSET&&busStopsMustHaveSpace) {
                // ok, we may stop (have reached the stop)
                if (MSNet::getInstance()->getPersonControl().checkWaiting(&myLane->getEdge(), this) && bstop.triggered) {
                    bstop.duration = 0;
                }
                MSNet::getInstance()->getVehicleControl().addWaiting(&myLane->getEdge(), this);
                if (hasCORNPointerValue(MSCORN::CORN_P_VEH_PASSENGER)) {
                    std::vector<MSPerson*> *persons = (std::vector<MSPerson*>*) myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER];
                    for (std::vector<MSPerson*>::iterator i=persons->begin(); i!=persons->end();) {
                        if (&(*i)->getDestination() == &myLane->getEdge()) {
                            (*i)->proceed(MSNet::getInstance(), MSNet::getInstance()->getCurrentTimeStep());
                            i = persons->erase(i);
                        } else {
                            ++i;
                        }
                    }
                }
                bstop.reached = true;
                // compute stopping time
                if (bstop.until>=0) {
                    if (bstop.duration==-1) {
                        bstop.duration = bstop.until - MSNet::getInstance()->getCurrentTimeStep();
                    } else {
                        bstop.duration = MAX2(bstop.duration, bstop.until - MSNet::getInstance()->getCurrentTimeStep());
                    }
                }
                if (bstop.busstop!=0) {
                    // let the bus stop know the vehicle
                    bstop.busstop->enter(this, myState.pos(), myState.pos()-myType->getLength());
                }
            }
            // decelerate
            // !!! should not v be 0 when we have reached the stop?
            return getCarFollowModel().ffeS(this, endPos-myState.pos());
        }
    }
    return currentVelocity;
}


bool
MSVehicle::moveRegardingCritical(SUMOTime t, const MSLane* const lane,
                                 const MSVehicle * const pred,
                                 const MSVehicle * const neigh,
                                 SUMOReal lengthsInFront) throw() {
#ifdef _MESSAGES
    if (myHBMsgEmitter != 0) {
        if (isOnRoad()) {
            SUMOReal timeStep = MSNet::getInstance()->getCurrentTimeStep();
            myHBMsgEmitter->writeHeartBeatEvent(myParameter->id, timeStep, myLane, myState.pos(), myState.speed(), getPosition().x(), getPosition().y());
        }
    }
#endif
#ifdef DEBUG_VEHICLE_GUI_SELECTION
    if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
        int bla = 0;
    }
#endif
    myTarget = 0;
    for (DriveItemVector::iterator i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end(); ++i) {
        if ((*i).myLink!=0) {
            (*i).myLink->removeApproaching(this);
        }
    }
    myLFLinkLanes.clear();
    const MSCFModel &cfModel = getCarFollowModel();
    // check whether the vehicle is not on an appropriate lane
    if (!myLane->appropriate(this)) {
        // decelerate to lane end when yes
        SUMOReal seen = myLane->getLength()-myState.myPos;
        SUMOReal vWish = MIN2(cfModel.ffeS(this, seen), myLane->getMaxSpeed());
        if (pred!=0) {
            // interaction with leader if one exists on same lane
            SUMOReal gap = gap2pred(*pred);
            if (MSGlobals::gCheck4Accidents && gap < 0/*-POSITION_EPS*/) {
                // collision occured!
                return true;
            }
            vWish = MIN2(vWish, cfModel.ffeV(this, pred));
        } else {
            // (potential) interaction with a vehicle extending partially into this lane
            MSVehicle *predP = myLane->getPartialOccupator();
            if (predP!=0) {
                SUMOReal gap = myLane->getPartialOccupatorEnd() - myState.myPos;
                if (MSGlobals::gCheck4Accidents && gap < 0/*-POSITION_EPS*/) {
                    // collision occured!
                    return true;
                }
                vWish = MIN2(vWish, cfModel.ffeV(this, gap, predP->getSpeed()));
            }
        }
        // interaction with left-lane leader (do not overtake right)
        cfModel.leftVehicleVsafe(this, neigh, vWish);
        // check whether the vehicle wants to stop somewhere
        if (!myStops.empty()&& &myStops.begin()->lane->getEdge()==&lane->getEdge()) {
            SUMOReal seen = lane->getLength() - myState.pos();
            SUMOReal vsafeStop = cfModel.ffeS(this, seen-(lane->getLength()-myStops.begin()->endPos));
            vWish = MIN2(vWish, vsafeStop);
        }
        vWish = MAX2((SUMOReal) 0, vWish);
        myLFLinkLanes.push_back(DriveProcessItem(0, vWish, vWish, false, 0, 0, myLane->getLength()-myState.myPos));
    } else {
        // compute other values as in move
        SUMOReal vBeg = MIN2(cfModel.maxNextSpeed(myState.mySpeed), lane->getMaxSpeed());//vaccel( myState.mySpeed, lane->maxSpeed() );
        if (pred!=0) {
            SUMOReal gap = gap2pred(*pred);
            if (MSGlobals::gCheck4Accidents && gap < 0/*-POSITION_EPS*/) {
                // collision occured!
                return true;
            }
            SUMOReal vSafe = cfModel.ffeV(this, gap, pred->getSpeed());
            //  the vehicle is bound by the lane speed and must not drive faster
            //  than vsafe to the next vehicle
            vBeg = MIN2(vBeg, vSafe);
        } else {
            // (potential) interaction with a vehicle extending partially into this lane
            MSVehicle *predP = myLane->getPartialOccupator();
            if (predP!=0) {
                SUMOReal gap = myLane->getPartialOccupatorEnd() - myState.myPos;
                if (MSGlobals::gCheck4Accidents && gap < 0/*-POSITION_EPS*/) {
                    // collision occured!
                    return true;
                }
                vBeg = MIN2(vBeg, cfModel.ffeV(this, gap, predP->getSpeed()));
            }
        }
        cfModel.leftVehicleVsafe(this, neigh, vBeg); // from left-lane leader (do not overtake right)
        // check whether the driver wants to let someone in
        // set next links, computing possible speeds
        vsafeCriticalCont(t, vBeg, lengthsInFront);
    }
    //@ to be optimized (move to somewhere else)
    if (hasCORNIntValue(MSCORN::CORN_VEH_LASTREROUTEOFFSET)) {
        myIntCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] = myIntCORNMap[MSCORN::CORN_VEH_LASTREROUTEOFFSET] + 1;
    }
    //@ to be optimized (move to somewhere else)
    checkRewindLinkLanes(lengthsInFront);
    return false;
}


bool
MSVehicle::moveFirstChecked() {
#ifdef DEBUG_VEHICLE_GUI_SELECTION
    if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
        int bla = 0;
    }
#endif
    myTarget = 0;
    // save old v for optional acceleration computation
    SUMOReal oldV = myState.mySpeed;
    // get vsafe
    SUMOReal vSafe = 0;

    assert(myLFLinkLanes.size()!=0);
    DriveItemVector::iterator i;
    bool braking = false;
    bool lastWasGreenCont = false;
    for (i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end(); ++i) {
        MSLink *link = (*i).myLink;
        // the vehicle must change the lane on one of the next lanes
        if (link!=0&&(*i).mySetRequest) {
            const MSLink::LinkState ls = link->getState();
            if(ls==MSLink::LINKSTATE_TL_GREEN_MAJOR||ls==MSLink::LINKSTATE_TL_GREEN_MINOR||ls==MSLink::LINKSTATE_TL_YELLOW_MAJOR||ls==MSLink::LINKSTATE_TL_YELLOW_MINOR) {
                myLastGreenTime = link->getSwitchToGreenTime();
            }
            // vehicles should brake when running onto a yellow light if the distance allows to halt in front
            const bool yellow = ls==MSLink::LINKSTATE_TL_YELLOW_MAJOR||ls==MSLink::LINKSTATE_TL_YELLOW_MINOR;
            if (yellow&&(*i).myDistance>getCarFollowModel().getSpeedAfterMaxDecel(myState.mySpeed)) {
                vSafe = (*i).myVLinkWait;
                braking = true;
                lastWasGreenCont = false;
                break;
            }
            //
            const bool opened = link->opened((*i).myArrivalTime, (*i).myArrivalSpeed, getVehicleType().getLength());
            // vehicles should decelerate when approaching a minor link
            if (opened&&!lastWasGreenCont&&!link->havePriority()&&(*i).myDistance>getCarFollowModel().getMaxDecel()) {
                vSafe = (*i).myVLinkWait;
                braking = true;
                lastWasGreenCont = false;
                break; // could be revalidated
            }
            // have waited; may pass if opened...
            if (opened) {
                vSafe = (*i).myVLinkPass;
                lastWasGreenCont = link->isCont()&&(ls==MSLink::LINKSTATE_TL_GREEN_MAJOR);
            } else {
                vSafe = (*i).myVLinkWait;
                braking = true;
                lastWasGreenCont = false;
                break;
            }
        } else {
            vSafe = (*i).myVLinkWait;
            braking = true;
            break;
        }
    }

    SUMOReal vNext = getCarFollowModel().moveHelper(this, myLane, vSafe);
    vNext = MAX2(vNext, (SUMOReal) 0.);
#ifndef NO_TRACI
	if(myTraCISpeed>=0) {
		vNext = myTraCISpeed;
	}
#endif
    // visit waiting time
    if (vNext<=0.1) {
        myWaitingTime += DELTA_T;
        if (MSCORN::wished(MSCORN::CORN_VEH_WAITINGTIME)) {
            myIntCORNMap[MSCORN::CORN_VEH_WAITINGTIME]++;
        }
        braking = true;
    } else {
        myWaitingTime = 0;
    }
    if (myState.mySpeed<vNext) {
        braking = false;
    }
	if(braking) {
		switchOnSignal(VEH_SIGNAL_BRAKELIGHT);
	} else {
		switchOffSignal(VEH_SIGNAL_BRAKELIGHT);
	}
    // call reminders after vNext is set
    SUMOReal pos = myState.myPos;
#ifndef NO_TRACI
    speedWithoutTraciInfluence = MIN2(vNext, myType->getMaxSpeed());
#endif
    vNext = MIN2(vNext, getMaxSpeed());

#ifdef _MESSAGES
    if (myHBMsgEmitter != 0) {
        if (isOnRoad()) {
            SUMOReal timeStep = MSNet::getInstance()->getCurrentTimeStep();
            myHBMsgEmitter->writeHeartBeatEvent(myParameter->id, timeStep, myLane, myState.pos(), myState.speed(), getPosition().x(), getPosition().y());
        }
    }
    if (myBMsgEmitter!=0) {
        if (vNext < oldV) {
            SUMOReal timeStep = MSNet::getInstance()->getCurrentTimeStep();
            myBMsgEmitter->writeBreakEvent(myParameter->id, timeStep, myLane, myState.pos(), myState.speed(), getPosition().x(), getPosition().y());
        }
    }
#endif
    // update position and speed
    myState.myPos += SPEED2DIST(vNext);
    myState.mySpeed = vNext;
    myTarget = 0;
    std::vector<MSLane*> passedLanes;
    for (std::vector<MSLane*>::reverse_iterator i=myFurtherLanes.rbegin(); i!=myFurtherLanes.rend(); ++i) {
        passedLanes.push_back(*i);
    }
    if (passedLanes.size()==0||passedLanes.back()!=myLane) {
        passedLanes.push_back(myLane);
    }
    // move on lane(s)
    if (myState.myPos<=myLane->getLength()) {
        // we are staying at our lane
        //  there is no need to go over succeeding lanes
        workOnMoveReminders(pos, pos + SPEED2DIST(vNext), vNext);
    } else {
        // we are moving at least to the next lane (maybe pass even more than one)
        MSLane *approachedLane = myLane;
        // move the vehicle forward
        SUMOReal driven = myState.myPos>approachedLane->getLength()
                          ? approachedLane->getLength() - pos
                          : myState.myPos - pos;
        for (i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end() && myState.myPos>approachedLane->getLength(); ++i) {
            if (approachedLane!=myLane) {
                leaveLaneAtMove(driven);
            }
            MSLink *link = (*i).myLink;
            // check whether the vehicle was allowed to enter lane
            //  otherwise it is decelareted and we do not need to test for it's
            //  approach on the following lanes when a lane changing is performed
            assert(approachedLane!=0);
            myState.myPos -= approachedLane->getLength();
            assert(myState.myPos>0);
            if (approachedLane!=myLane) {
                if(enterLaneAtMove(approachedLane, driven)) {
                    myLane = approachedLane;
                    return true;
                }
                driven += approachedLane->getLength();
            }
            // proceed to the next lane
            if (link!=0) {
#ifdef HAVE_INTERNAL_LANES
                approachedLane = link->getViaLane();
                if (approachedLane==0) {
                    approachedLane = link->getLane();
                }
#else
                approachedLane = link->getLane();
#endif
            }
            passedLanes.push_back(approachedLane);
        }
        myTarget = approachedLane;
    }
    // clear previously set information
    for (std::vector<MSLane*>::iterator i=myFurtherLanes.begin(); i!=myFurtherLanes.end(); ++i) {
        (*i)->resetPartialOccupation(this);
    }
    myFurtherLanes.clear();
    if (myState.myPos-getVehicleType().getLength()<0&&passedLanes.size()>0) {
        SUMOReal leftLength = getVehicleType().getLength()-myState.myPos;
        std::vector<MSLane*>::reverse_iterator i=passedLanes.rbegin() + 1;
        while (leftLength>0&&i!=passedLanes.rend()) {
            myFurtherLanes.push_back(*i);
            leftLength -= (*i)->setPartialOccupation(this, leftLength);
            ++i;
        }
    }
    assert(myTarget==0||myTarget->getLength()>=myState.myPos);
    setBlinkerInformation();
    return false;
}


void
MSVehicle::checkRewindLinkLanes(SUMOReal lengthsInFront) throw() {
#ifdef DEBUG_VEHICLE_GUI_SELECTION
    if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
        int bla = 0;
        if (MSNet::getInstance()->getCurrentTimeStep()==152000) {
            bla = 0;
        }
    }
#endif
#ifdef HAVE_INTERNAL_LANES
    if (MSGlobals::gUsingInternalLanes) {
        int removalBegin = -1;
        bool hadVehicle = false;
        SUMOReal seenLanes = 0;
        SUMOReal seenSpace = -lengthsInFront;

        std::vector<SUMOReal> availableSpace;
        std::vector<bool> hadVehicles;

        for (unsigned int i=0; i<myLFLinkLanes.size(); ++i) {
            // skip unset links
            DriveProcessItem &item = myLFLinkLanes[i];
            if (item.myLink==0) {
                availableSpace.push_back(seenSpace);
                hadVehicles.push_back(hadVehicle);
                continue;
            }
            // get the next lane, determine whether it is an internal lane
            MSLane *approachedLane = item.myLink->getViaLane();
            if (approachedLane!=0) {
                if (item.myLink->isCrossing()&&item.myLink->willHaveBlockedFoe()) {
                    seenSpace = seenSpace - approachedLane->getVehLenSum();
                    hadVehicle |= approachedLane->getVehicleNumber()!=0;
                } else {
                    seenSpace = seenSpace - approachedLane->getVehLenSum() + approachedLane->getLength();
                    hadVehicle |= approachedLane->getVehicleNumber()!=0;
                }
                availableSpace.push_back(seenSpace);
                hadVehicles.push_back(hadVehicle);
                continue;
            }
            approachedLane = item.myLink->getLane();
            MSVehicle *last = approachedLane->getLastVehicle();
            if (last==0) {
                last = approachedLane->getPartialOccupator();
                if (last!=0) {
                    SUMOReal m = MAX2(seenSpace, seenSpace + approachedLane->getPartialOccupatorEnd() + last->getCarFollowModel().brakeGap(last->getSpeed()));
                    availableSpace.push_back(m);
                    hadVehicle = true;
                    seenSpace = seenSpace - approachedLane->getVehLenSum() + approachedLane->getLength();
                } else {
                    seenSpace = seenSpace - approachedLane->getVehLenSum() + approachedLane->getLength();
                    availableSpace.push_back(seenSpace);
                }
            } else {
                if (last->signalSet(VEH_SIGNAL_BRAKELIGHT)) {
                    SUMOReal lastBrakeGap = last->getCarFollowModel().brakeGap(approachedLane->getLastVehicle()->getSpeed());
                    SUMOReal lastGap = last->getPositionOnLane() - last->getVehicleType().getLength() + lastBrakeGap - last->getSpeed()*last->getCarFollowModel().getTau();
                    SUMOReal m = MAX2(seenSpace, seenSpace + lastGap);
                    availableSpace.push_back(m);
                    seenSpace = seenSpace - approachedLane->getVehLenSum() + approachedLane->getLength();
                } else {
                    seenSpace = seenSpace - approachedLane->getVehLenSum() + approachedLane->getLength();
                    availableSpace.push_back(seenSpace);
                }
                hadVehicle = true;
            }
            hadVehicles.push_back(hadVehicle);
        }
#ifdef DEBUG_VEHICLE_GUI_SELECTION
        if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
            int bla = 0;
        }
#endif
        SUMOTime t = MSNet::getInstance()->getCurrentTimeStep();
        for (int i= (int)(myLFLinkLanes.size()-1); i>0; --i) {
            DriveProcessItem &item = myLFLinkLanes[i-1];
            if (item.myLink==0||item.myLink->isCont()||item.myLink->opened(t, .1, getVehicleType().getLength())||!hadVehicles[i]) {
                availableSpace[i-1] = availableSpace[i];
            }
        }

        for (unsigned int i=0; hadVehicle&&i<myLFLinkLanes.size()&&removalBegin<0; ++i) {
            // skip unset links
            DriveProcessItem &item = myLFLinkLanes[i];
            if (item.myLink==0) {
                continue;
            }
            if (/*!item.myLink->isCont()
                    &&*/availableSpace[i]-getVehicleType().getLength()<0
                    &&item.myLink->willHaveBlockedFoe()) {
                removalBegin = i;
            }
        }
        if (removalBegin!=-1&&!(removalBegin==0&&myLane->getEdge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL)) {
            while (removalBegin< (int) (myLFLinkLanes.size())) {
                myLFLinkLanes[removalBegin].myVLinkPass = myLFLinkLanes[removalBegin].myVLinkWait;
                myLFLinkLanes[removalBegin].mySetRequest = false;
                ++removalBegin;
            }
        }
    }
#endif
    for (DriveItemVector::iterator i=myLFLinkLanes.begin(); i!=myLFLinkLanes.end(); ++i) {
        if ((*i).myLink!=0) {
            (*i).myLink->setApproaching(this, (*i).myArrivalTime, (*i).myArrivalSpeed, (*i).mySetRequest);
        }
    }
}



void
MSVehicle::vsafeCriticalCont(SUMOTime t, SUMOReal boundVSafe, SUMOReal lengthsInFront) {
#ifdef DEBUG_VEHICLE_GUI_SELECTION
    if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
        int bla = 0;
    }
#endif
    const MSCFModel &cfModel = getCarFollowModel();
    // the vehicle may have just to look into the next lane
    //  compute this information and use it only once in the next loop
    SUMOReal seen = myLane->getLength() - myState.myPos;
    SUMOReal seenNonInternal = 0;//myLane->getEdge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL ? 0 : seen;
    // !!! Why is the brake gap accounted? !!!
    if (this!=myLane->getFirstVehicle() && seen - cfModel.brakeGap(myState.mySpeed) > 0) {
        // not "reaching critical"
        myLFLinkLanes.push_back(DriveProcessItem(0, boundVSafe, boundVSafe, false, 0, 0, seen));
        return;
    }

    MSLane *nextLane = myLane;
    // compute the way the vehicle would drive if it would use the current speed and then
    //  decelerate
    SUMOReal maxV = cfModel.maxNextSpeed(myState.mySpeed);
    SUMOReal dist = SPEED2DIST(maxV) + cfModel.brakeGap(maxV);//myState.mySpeed);
    SUMOReal vLinkPass = boundVSafe;
    SUMOReal vLinkWait = vLinkPass;
    const std::vector<MSLane*> &bestLaneConts = getBestLanesContinuation();
#ifdef HAVE_INTERNAL_LANES
    bool hadNonInternal = false;
#endif

    unsigned int view = 1;
    // loop over following lanes
    while (true) {
        // process stops
        if (!myStops.empty()&& &myStops.begin()->lane->getEdge()==&nextLane->getEdge()) {
            SUMOReal vsafeStop = cfModel.ffeS(this, seen-(nextLane->getLength()-myStops.begin()->endPos));
            vLinkPass = MIN2(vLinkPass, vsafeStop);
            vLinkWait = MIN2(vLinkWait, vsafeStop);
        }

        // get the next link used
        MSLinkCont::const_iterator link = myLane->succLinkSec(*this, view, *nextLane, bestLaneConts);
        // and the length of the currently investigated lane
        SUMOReal laneLength = nextLane->getLength();

        // check whether the lane is a dead end
        //  (should be valid only on further loop iterations
        if (nextLane->isLinkEnd(link)) {
            // the vehicle will not drive further
            SUMOReal laneEndVSafe = cfModel.ffeS(this, seen);
            myLFLinkLanes.push_back(DriveProcessItem(0, MIN2(vLinkPass, laneEndVSafe), MIN2(vLinkPass, laneEndVSafe), false, 0, 0, seen));
            return;
        }
        // the link was passed
        vLinkWait = vLinkPass;


        // needed to let vehicles wait for all overlapping vehicles in front
        const MSLinkCont &lc = nextLane->getLinkCont();

        // get the following lane
#ifdef HAVE_INTERNAL_LANES
        SUMOReal lastLength = nextLane->getLength();
        bool nextInternal = false;
        nextLane = (*link)->getViaLane();
        if (nextLane==0) {
            nextLane = (*link)->getLane();
            hadNonInternal = true;
        } else {
            nextInternal = true;
        }
#else
        nextLane = (*link)->getLane();
#endif

        // compute the velocity to use when the link is not blocked by oter vehicles
        //  the vehicle shall be not faster when reaching the next lane than allowed
        SUMOReal vmaxNextLane = MAX2(cfModel.ffeV(this, seen, nextLane->getMaxSpeed()), nextLane->getMaxSpeed());

        // the vehicle shall keep a secure distance to its predecessor
        //  (or approach the lane end if the predeccessor is too near)
        SUMOReal vsafePredNextLane = 100000;
        std::pair<MSVehicle*, SUMOReal> lastOnNext = nextLane->getLastVehicleInformation();
        if (lastOnNext.first!=0) {
            if (seen+lastOnNext.second>=0) {
                vsafePredNextLane = cfModel.ffeV(this, seen+lastOnNext.second, lastOnNext.first->getSpeed());
            } else {
                vsafePredNextLane = cfModel.ffeS(this, seen);
            }
        }
#ifdef DEBUG_VEHICLE_GUI_SELECTION
        if (gSelected.isSelected(GLO_VEHICLE, static_cast<const GUIVehicle*>(this)->getGlID())) {
            int bla = 0;
        }
#endif
        // compute the velocity to use when the link may be used
        vLinkPass = MIN3(vLinkPass, vmaxNextLane, vsafePredNextLane);

        // if the link may not be used (is blocked by another vehicle) then let the
        //  vehicle decelerate until the end of the street
        vLinkWait = MIN3(vLinkPass, vLinkWait, cfModel.ffeS(this, seen));

        // behaviour in front of not priorised intersections (waiting for priorised foe vehicles)
        bool setRequest = false;
        // process stops
        if (!myStops.empty()&& &myStops.begin()->lane->getEdge()==&nextLane->getEdge()) {
            SUMOReal vsafeStop = cfModel.ffeS(this, seen+myStops.begin()->endPos);
            vLinkPass = MIN2(vLinkPass, vsafeStop);
            vLinkWait = MIN2(vLinkWait, vsafeStop);
        }
        setRequest |= ((*link)->getState()!=MSLink::LINKSTATE_TL_RED&&(vLinkPass>0&&dist-seen>0));
        bool yellow = (*link)->getState()==MSLink::LINKSTATE_TL_YELLOW_MAJOR||(*link)->getState()==MSLink::LINKSTATE_TL_YELLOW_MINOR;
        bool red = (*link)->getState()==MSLink::LINKSTATE_TL_RED;
        if ((yellow||red)&&seen>cfModel.brakeGap(myState.mySpeed)-SPEED2DIST(myState.mySpeed)*cfModel.getTau()) { // !!! we should reuse brakeGap with no reaction time...
            vLinkPass = vLinkWait;
            setRequest = false;
            myLFLinkLanes.push_back(DriveProcessItem(*link, vLinkWait, vLinkWait, false, t+TIME2STEPS(seen / vLinkPass), vLinkPass, seen));
        }
        // the next condition matches the previously one used for determining the difference
        //  between critical/non-critical vehicles. Though, one should assume that a vehicle
        //  should want to move over an intersection even though it could brake before it!?
        setRequest &= dist-seen>0;
        myLFLinkLanes.push_back(DriveProcessItem(*link, vLinkPass, vLinkWait, setRequest, t + TIME2STEPS(seen / vLinkPass), vLinkPass, seen));
        seen += nextLane->getLength();
        seenNonInternal += nextLane->getEdge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL ? 0 : nextLane->getLength();
        if ((vLinkPass<=0||seen>dist)&&hadNonInternal&&seenNonInternal>50) {
            return;
        }
#ifdef HAVE_INTERNAL_LANES
        if (!nextInternal) {
            view++;
        }
#else
        view++;
#endif
    }
}


Position2D
MSVehicle::getPosition() const {
    if (myLane==0) {
        return Position2D(-1000, -1000);
    }
    return myLane->getShape().positionAtLengthPosition(myState.pos());
}


const std::string &
MSVehicle::getID() const throw() {
    return myParameter->id;
}


bool
MSVehicle::enterLaneAtMove(MSLane* enteredLane, SUMOReal driven, bool onTeleporting) {
    // vaporizing edge?
    if (enteredLane->getEdge().isVaporizing()) {
        // yep, let's do the vaporization...
        setWasVaporized(false);
        myLane = enteredLane;
        return true;
    }
    if(!onTeleporting) {
        // move mover reminder one lane further
        adaptLaneEntering2MoveReminder(*enteredLane);
        // set the entered lane as the current lane
        myLane = enteredLane;
    }

    // internal edges are not a part of the route...
    if (enteredLane->getEdge().getPurpose()!=MSEdge::EDGEFUNCTION_INTERNAL) {
        if (MSCORN::wished(MSCORN::CORN_VEH_SAVE_EDGE_EXIT)) {
            if (myPointerCORNMap.find(MSCORN::CORN_P_VEH_EXIT_TIMES)==myPointerCORNMap.end()) {
                myPointerCORNMap[MSCORN::CORN_P_VEH_EXIT_TIMES] = new std::vector<SUMOTime>();
            }
            ((std::vector<SUMOTime>*) myPointerCORNMap[MSCORN::CORN_P_VEH_EXIT_TIMES])->push_back(MSNet::getInstance()->getCurrentTimeStep());
        }
        ++myCurrEdge;
    }
    if(!onTeleporting) {
        // may be optimized: compute only, if the current or the next have more than one lane...!!!
        getBestLanes(true);
        activateReminders(false, false);
        for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
            (*dev)->enterLaneAtMove(enteredLane, driven);
        }

#ifndef NO_TRACI
        checkForLaneChanges();
#endif
    }
    MSRouteIterator destination = myRoute->end() - 1;
    return myCurrEdge == destination && getPositionOnLane() > myArrivalPos - POSITION_EPS;
}


void
MSVehicle::enterLaneAtLaneChange(MSLane* enteredLane) {
#ifdef _MESSAGES
    if (myLCMsgEmitter!=0) {
        SUMOReal timeStep = MSNet::getInstance()->getCurrentTimeStep();
        myLCMsgEmitter->writeLaneChangeEvent(myParameter->id, timeStep, myLane, myState.pos(), myState.speed(), enteredLane, getPosition().x(), getPosition().y());
    }
#endif
    MSLane *myPriorLane = myLane;
    myLane = enteredLane;
    // switch to and activate the new lane's reminders
    // keep OldLaneReminders
    myMoveReminders = enteredLane->getMoveReminders();
    activateReminders(false, true);
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->enterLaneAtLaneChange(enteredLane);
    }
    SUMOReal leftLength = myState.myPos-getVehicleType().getLength();
    if (leftLength<0) {
        // we have to rebuild "further lanes"
        const MSRoute &route = getRoute();
        MSRouteIterator i = myCurrEdge;
        MSLane *lane = myLane;
        while (i!=route.begin()&&leftLength>0) {
            const MSEdge * const prev = *(--i);
            const std::vector<MSLane::IncomingLaneInfo> &incomingLanes = lane->getIncomingLanes();
            for (std::vector<MSLane::IncomingLaneInfo>::const_iterator j=incomingLanes.begin(); j!=incomingLanes.end(); ++j) {
                if (&(*j).lane->getEdge()==prev) {
#ifdef HAVE_INTERNAL_LANES
                    (*j).lane->setPartialOccupation(this, leftLength);
#else
                    leftLength -= (*j).length;
                    (*j).lane->setPartialOccupation(this, leftLength);
#endif
                    leftLength -= (*j).lane->getLength();
                    break;
                }
            }
        }
    }
#ifndef NO_TRACI
    // check if further changes are necessary
    checkForLaneChanges();
#endif
}


void
MSVehicle::enterLaneAtEmit(MSLane* enteredLane, SUMOReal pos, SUMOReal speed) {
    myState = State(pos, speed);
    assert(myState.myPos >= 0);
    assert(myState.mySpeed >= 0);
    myWaitingTime = 0;
    myLane = enteredLane;
    // set and activate the new lane's reminders
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->enterLaneAtEmit();
    }
    std::string msg;
    if (!hasValidRoute(msg)) {
        throw ProcessError("Vehicle '" + getID() + "' has no valid route. " + msg);
    }
    myMoveReminders = enteredLane->getMoveReminders();
    activateReminders(true, false);
    // build the list of lanes the vehicle is lapping into
    SUMOReal leftLength = myType->getLength() - pos;
    MSLane *clane = enteredLane;
    while (leftLength>0) {
        clane = clane->getLogicalPredecessorLane();
        if(clane==0) {
            break;
        }
        myFurtherLanes.push_back(clane);
        leftLength -= (clane)->setPartialOccupation(this, leftLength);
    }
}


void
MSVehicle::leaveLaneAtMove(SUMOReal driven) {
    std::vector< MSMoveReminder* >::iterator rem;
    for (rem=myMoveReminders.begin(); rem != myMoveReminders.end(); ++rem) {
        (*rem)->notifyLeave(*this, false, false);
    }
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->leaveLaneAtMove(driven);
    }
}


void
MSVehicle::leaveLane(bool isArrival) {
    for (std::vector< MSDevice* >::iterator dev=myDevices.begin(); dev != myDevices.end(); ++dev) {
        (*dev)->leaveLane();
    }
    // dismiss the old lane's reminders
    SUMOReal savePos = myState.myPos; // have to do this due to SUMOReal-precision errors
    std::vector< MSMoveReminder* >::iterator rem;
    for (rem=myMoveReminders.begin(); rem != myMoveReminders.end(); ++rem) {
        (*rem)->notifyLeave(*this, isArrival, !isArrival);
    }
    std::vector<SUMOReal>::iterator off = myOldLaneMoveReminderOffsets.begin();
    for (rem=myOldLaneMoveReminders.begin(); rem!=myOldLaneMoveReminders.end(); ++rem, ++off) {
        myState.myPos += (*off);
        (*rem)->notifyLeave(*this, isArrival, !isArrival);
        myState.myPos -= (*off);
    }
    myState.myPos = savePos; // have to do this due to SUMOReal-precision errors
    myMoveReminders.clear();
    myOldLaneMoveReminders.clear();
    myOldLaneMoveReminderOffsets.clear();
    for (std::vector<MSLane*>::iterator i=myFurtherLanes.begin(); i!=myFurtherLanes.end(); ++i) {
        (*i)->resetPartialOccupation(this);
    }
    myFurtherLanes.clear();
}


const MSEdge * const
    MSVehicle::getEdge() const {
    return *myCurrEdge;
}


MSLane *
MSVehicle::getTargetLane() const {
    return myTarget;
}


MSAbstractLaneChangeModel &
MSVehicle::getLaneChangeModel() {
    return *myLaneChangeModel;
}


const MSAbstractLaneChangeModel &
MSVehicle::getLaneChangeModel() const {
    return *myLaneChangeModel;
}


void
MSVehicle::quitRemindedEntered(MSVehicleQuitReminded *r) {
    myQuitReminded.push_back(r);
}


void
MSVehicle::quitRemindedLeft(MSVehicleQuitReminded *r) {
    QuitRemindedVector::iterator i = find(myQuitReminded.begin(), myQuitReminded.end(), r);
    if (i!=myQuitReminded.end()) {
        myQuitReminded.erase(i);
    }
}


void
MSVehicle::rebuildContinuationsFor(LaneQ &oq, MSLane *l, MSRouteIterator ce, int seen) const {
    // check whether the end of iteration was reached
    ++ce;
    // we end if one of the following cases is true:
    // a) we have examined the next edges for 3000m (foresight distance)
    //     but only if we have at least examined the next edge
    // b) if we have examined 8 edges in front (!!! this may be shorted)
    // c) if the route does not continue after the seen edges
    if ((seen>4 && oq.length+l->getLength()>3000) || seen>8 || ce==myRoute->end()) {
        // ok, we have rebuilt this so far... do not have to go any further
        return;
    }
    // we must go further...
    // get the list of allowed lanes
    const std::vector<MSLane*> *allowed = 0;
    if (ce!=myRoute->end()&&ce+1!=myRoute->end()) {
        allowed = (*ce)->allowedLanes(**(ce+1), myType->getVehicleClass());
    }
    // determined recursively what the best lane is
    //  save the best lane for later usage
    LaneQ best;
    best.length = 0;
    const std::vector<MSLane*> &lanes = (*ce)->getLanes();
    const MSLinkCont &lc = l->getLinkCont();
    bool gotOne = false;
    // we go through all connections of the lane to examine
    for (MSLinkCont::const_iterator k=lc.begin(); k!=lc.end(); ++k) {
        // prese values
        LaneQ q;
        MSLane *qqq = (*k)->getLane();
        if (qqq==0) {
            q.occupation = 0;
            q.length = 0;
            continue;
        }
        q.occupation = qqq->getVehLenSum();
        q.length = qqq->getLength();
        q.bestContinuations.push_back(qqq);


        if (!myStops.empty()&& &(myStops.front().lane->getEdge())==&qqq->getEdge()) {
            if (myStops.front().lane==qqq) {
                gotOne = true;
                if (allowed==0||find(allowed->begin(), allowed->end(), (*k)->getLane())!=allowed->end()) {
                    rebuildContinuationsFor(q, qqq, ce, seen+1);
                }
            } else {
                q.occupation = qqq->getVehLenSum();
                const Stop &s = myStops.front();
                SUMOReal endPos = s.endPos;
                if (s.busstop!=0) {
                    // on bus stops, we have to wait for free place if they are in use...
                    endPos = s.busstop->getLastFreePos();
                }
                q.length = endPos;
            }
        } else {
            // check whether the lane is allowed for route continuation (has a link to the next
            //  edge in route)
            if (allowed==0||find(allowed->begin(), allowed->end(), (*k)->getLane())!=allowed->end()) {
                // yes -> compute the best lane combination for consecutive lanes
                gotOne = true;
                rebuildContinuationsFor(q, qqq, ce, seen+1);
            } else {
                // no -> if the lane belongs to an edge not in our route,
                //  reset values to zero (otherwise the lane but not its continuations)
                //  will still be regarded
                if (&(*k)->getLane()->getEdge()!=*ce) {
                    q.occupation = 0;
                    q.length = 0;
                }
            }
        }
        // set best lane information
        if (q.length>best.length) {
            best = q;
        }
    }
    // check whether we need to change the lane on this edge in any case
    if (!gotOne) {
        // yes, that's the case - we are on an edge on which we have to change
        //  the lane no matter which lanes we are using so far.
        // we have to tell the vehicle the best lane so far...
        // the assumption is that we only have to find the first one
        //  - because the vehicle has to change lanes, it will do this into
        //  the proper direction as the lanes moving the the proper edge are
        //  lying side by side
        const std::vector<MSLane*> &lanes = (*ce)->getLanes();
        bool oneFound = false;
        int bestPos = 0;
        MSLane *next = 0;
        // we go over the next edge's lanes and determine the first that may be used
        for (std::vector<MSLane*>::const_iterator i=lanes.begin(); !oneFound&&i!=lanes.end();) {
            if (allowed!=0 && find(allowed->begin(), allowed->end(), *i)!=allowed->end()) {
                oneFound = true;
                next = *i;
            } else {
                ++i;
                ++bestPos;
            }
        }
        // ... it is now stored in next and its position in bestPos
        if (oneFound) {
            // ok, we have found a best lane
            //  (in fact, this should be the case if the route is valid, nonetheless...)
            // now let's say that the best lane is the nearest one to the found
            int bestDistance = -100;
            MSLane *bestL = 0;
            // go over all lanes of current edge
            const std::vector<MSLane*> &clanes = l->getEdge().getLanes();
            for (std::vector<MSLane*>::const_iterator i=clanes.begin(); i!=clanes.end(); ++i) {
                // go over all connected lanes
                for (MSLinkCont::const_iterator k=lc.begin(); k!=lc.end(); ++k) {
                    if ((*k)->getLane()==0) {
                        continue;
                    }
                    // the best lane must be on the proper edge
                    if (&(*k)->getLane()->getEdge()==*ce) {
                        std::vector<MSLane*>::const_iterator l=find(lanes.begin(), lanes.end(), (*k)->getLane());
                        if (l!=lanes.end()) {
                            int pos = (int)distance(lanes.begin(), l);
                            int cdist = abs(pos-bestPos);
                            if (bestDistance==-100||bestDistance>cdist) {
                                bestDistance = cdist;
                                bestL = *i;
                            }
                        }
                    }
                }
            }
            if (bestL==l) {
                best.occupation = next->getVehLenSum();
                best.length = next->getLength();
            } else {
                best.occupation = 0;
                best.length = 0;
                best.bestContinuations.clear();
            }
        }
    }
    oq.length += best.length;
    oq.occupation += best.occupation;
    copy(best.bestContinuations.begin(), best.bestContinuations.end(), back_inserter(oq.bestContinuations));
}



const std::vector<MSVehicle::LaneQ> &
MSVehicle::getBestLanes(bool forceRebuild, MSLane *startLane) const throw() {
    if (startLane==0) {
        startLane = myLane;
    }
    if (myLastBestLanesEdge==&startLane->getEdge()&&!forceRebuild) {
        std::vector<LaneQ> &lanes = *myBestLanes.begin();
        std::vector<LaneQ>::iterator i;
        for (i=lanes.begin(); i!=lanes.end(); ++i) {
            SUMOReal nextOccupation = 0;
            for (std::vector<MSLane*>::const_iterator j=(*i).bestContinuations.begin(); j!=(*i).bestContinuations.end(); ++j) {
                nextOccupation += (*j)->getVehLenSum();
            }
            (*i).nextOccupation = nextOccupation;
            if ((*i).lane==startLane) {
                myCurrentLaneInBestLanes = i;
            }
        }
        return *myBestLanes.begin();
    }
    myLastBestLanesEdge = &startLane->getEdge();
    myBestLanes.clear();
    myBestLanes.push_back(std::vector<LaneQ>());
    const std::vector<MSLane*> &lanes = (*myCurrEdge)->getLanes();
    MSRouteIterator ce = myCurrEdge;
    int seen = 0;
    const std::vector<MSLane*> *allowed = 0;
    if (ce!=myRoute->end()&&ce+1!=myRoute->end()) {
        allowed = (*ce)->allowedLanes(**(ce+1), myType->getVehicleClass());
    }
    for (std::vector<MSLane*>::const_iterator i=lanes.begin(); i!=lanes.end(); ++i) {
        LaneQ q;
        q.lane = *i;
        q.length = 0;//q.lane->getLength();
        q.occupation = 0;//q.lane->getVehLenSum();
        if (!myStops.empty()&& &myStops.front().lane->getEdge()==&q.lane->getEdge()) {
            if (myStops.front().lane==q.lane) {
                q.allowsContinuation = allowed==0||find(allowed->begin(), allowed->end(), q.lane)!=allowed->end();
                q.length += q.lane->getLength();
                q.occupation += q.lane->getVehLenSum();
            } else {
                q.allowsContinuation = false;
                q.occupation = q.lane->getVehLenSum();
                const Stop &s = myStops.front();
                SUMOReal endPos = s.endPos;
                if (s.busstop!=0) {
                    // on bus stops, we have to wait for free place if they are in use...
                    endPos = s.busstop->getLastFreePos();
                }
                q.length = endPos;
            }
        } else {
            q.allowsContinuation = allowed==0||find(allowed->begin(), allowed->end(), q.lane)!=allowed->end();
        }
        myBestLanes[0].push_back(q);
    }
    if (ce!=myRoute->end()) {
        for (std::vector<MSVehicle::LaneQ>::iterator i=myBestLanes.begin()->begin(); i!=myBestLanes.begin()->end(); ++i) {
            if ((*i).allowsContinuation) {
                rebuildContinuationsFor((*i), (*i).lane, ce, seen);
                (*i).length += (*i).lane->getLength();
                (*i).occupation += (*i).lane->getVehLenSum();
            }
        }
    }
    SUMOReal best = 0;
    int index = 0;
    int run = 0;
    for (std::vector<MSVehicle::LaneQ>::iterator i=myBestLanes.begin()->begin(); i!=myBestLanes.begin()->end(); ++i, ++run) {
        if (best<(*i).length) {
            best = (*i).length;
            index = run;
        }
        if ((*i).lane==startLane) {
            myCurrentLaneInBestLanes = i;
        }
    }
    run = 0;
    for (std::vector<MSVehicle::LaneQ>::iterator i=myBestLanes.begin()->begin(); i!=myBestLanes.begin()->end(); ++i, ++run) {
        (*i).bestLaneOffset =  index - run;
    }

        std::vector<LaneQ> &currLanes = *myBestLanes.begin();
        std::vector<LaneQ>::iterator i;
        for (i=currLanes.begin(); i!=currLanes.end(); ++i) {
            SUMOReal nextOccupation = 0;
            for (std::vector<MSLane*>::const_iterator j=(*i).bestContinuations.begin(); j!=(*i).bestContinuations.end(); ++j) {
                nextOccupation += (*j)->getVehLenSum();
            }
            (*i).nextOccupation = nextOccupation;
            if ((*i).lane==startLane) {
                myCurrentLaneInBestLanes = i;
            }
        }

    return *myBestLanes.begin();
}


void
MSVehicle::writeXMLRoute(OutputDevice &os, int index) const {
    // check if a previous route shall be written
    os.openTag("route");
    if (index>=0) {
        const ReplacedRoutesVector *v = (const ReplacedRoutesVector *)getCORNPointerValue(MSCORN::CORN_P_VEH_OLDROUTE);
        assert((int) v->size()>index);
        // write edge on which the vehicle was when the route was valid
        os << " replacedOnEdge=\"" << (*v)[index].edge->getID();
        // write the time at which the route was replaced
        os << "\" replacedAtTime=\"" << time2string((*v)[index].time) << "\" probability=\"0\" edges=\"";
        // get the route
        for (int i=0; i<index; ++i) {
            (*v)[i].route->writeEdgeIDs(os, (*v)[i].edge);
        }
        (*v)[index].route->writeEdgeIDs(os);
    } else {
        os << " edges=\"";
        if (hasCORNPointerValue(MSCORN::CORN_P_VEH_OLDROUTE)) {
            int noReroutes = getCORNIntValue(MSCORN::CORN_VEH_NUMBERROUTE);
            const ReplacedRoutesVector *v = (const ReplacedRoutesVector *)getCORNPointerValue(MSCORN::CORN_P_VEH_OLDROUTE);
            assert((int) v->size()==noReroutes);
            for (int i=0; i<noReroutes; ++i) {
                (*v)[i].route->writeEdgeIDs(os, (*v)[i].edge);
            }
        }
        myRoute->writeEdgeIDs(os);
        if (hasCORNPointerValue(MSCORN::CORN_P_VEH_EXIT_TIMES)) {
            os << "\" exitTimes=\"";
            const std::vector<SUMOTime> *exits = (const std::vector<SUMOTime> *)getCORNPointerValue(MSCORN::CORN_P_VEH_EXIT_TIMES);
            for (std::vector<SUMOTime>::const_iterator it = exits->begin(); it != exits->end(); ++it) {
                if (it != exits->begin()) {
                    os << " ";
                }
                os << time2string(*it);
            }
        }
    }
    (os << "\"").closeTag(true);
}


void
MSVehicle::saveState(std::ostream &os) {
    FileHelpers::writeString(os, myParameter->id);
    FileHelpers::writeFloat(os, myLastLaneChangeOffset);
    FileHelpers::writeFloat(os, myWaitingTime);
    FileHelpers::writeInt(os, myParameter->repetitionNumber);
#ifdef HAVE_SUBSECOND_TIMESTEPS
    FileHelpers::writeTime(os, myParameter->repetitionOffset);
#else
    FileHelpers::writeFloat(os, myParameter->repetitionOffset);
#endif
    FileHelpers::writeString(os, myRoute->getID());
    FileHelpers::writeTime(os, myParameter->depart);
    FileHelpers::writeString(os, myType->getID());
    FileHelpers::writeUInt(os, myRoute->posInRoute(myCurrEdge));
    if (hasCORNIntValue(MSCORN::CORN_VEH_DEPART_TIME)) {
        FileHelpers::writeInt(os, getCORNIntValue(MSCORN::CORN_VEH_DEPART_TIME));
    } else {
        FileHelpers::writeInt(os, -1);
    }
#ifdef HAVE_MESOSIM
    // !!! several things may be missing
    if (mySegment==0) {
        FileHelpers::writeUInt(os, 0);
    } else {
        FileHelpers::writeUInt(os, mySegment->getIndex());
    }
    FileHelpers::writeUInt(os, getQueIndex());
    FileHelpers::writeTime(os, myEventTime);
    FileHelpers::writeTime(os, myLastEntryTime);
#endif
}




const std::vector<MSLane*> &
MSVehicle::getBestLanesContinuation() const throw() {
    if (myBestLanes.empty()||myBestLanes[0].empty()||myLane->getEdge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
        return myEmptyLaneVector;
    }
    return (*myCurrentLaneInBestLanes).bestContinuations;
}


const std::vector<MSLane*> &
MSVehicle::getBestLanesContinuation(const MSLane * const l) const throw() {
    for (std::vector<std::vector<LaneQ> >::const_iterator i=myBestLanes.begin(); i!=myBestLanes.end(); ++i) {
        if ((*i).size()!=0&&(*i)[0].lane==l) {
            return (*i)[0].bestContinuations;
        }
    }
    return myEmptyLaneVector;
}



SUMOReal
MSVehicle::getDistanceToPosition(SUMOReal destPos, const MSEdge* destEdge) {
#ifdef DEBUG_VEHICLE_GUI_SELECTION
    SUMOReal distance = 1000000.;
#else
    SUMOReal distance = std::numeric_limits<SUMOReal>::max();
#endif
    if (isOnRoad() && destEdge != NULL) {
        if (&myLane->getEdge() == *myCurrEdge) {
            // vehicle is on a normal edge
            distance = myRoute->getDistanceBetween(getPositionOnLane(), destPos, *myCurrEdge, destEdge);
        } else {
            // vehicle is on inner junction edge
            distance = myLane->getLength() - getPositionOnLane();
            distance += myRoute->getDistanceBetween(0, destPos, *(myCurrEdge+1), destEdge);
        }
    }
    return distance;
}

void
MSVehicle::setWasVaporized(bool onDepart) {
    if (MSCORN::wished(MSCORN::CORN_VEH_VAPORIZED)) {
        myIntCORNMap[MSCORN::CORN_VEH_VAPORIZED] = onDepart ? 1 : 0;
    }
}


SUMOReal
MSVehicle::getHBEFA_CO2Emissions() const throw() {
    return HelpersHBEFA::computeCO2(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHBEFA_COEmissions() const throw() {
    return HelpersHBEFA::computeCO(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHBEFA_HCEmissions() const throw() {
    return HelpersHBEFA::computeHC(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHBEFA_NOxEmissions() const throw() {
    return HelpersHBEFA::computeNOx(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHBEFA_PMxEmissions() const throw() {
    return HelpersHBEFA::computePMx(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHBEFA_FuelConsumption() const throw() {
    return HelpersHBEFA::computeFuel(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


SUMOReal
MSVehicle::getHarmonoise_NoiseEmissions() const throw() {
    return HelpersHarmonoise::computeNoise(myType->getEmissionClass(), myState.speed(), myPreDawdleAcceleration);
}


void
MSVehicle::addPerson(MSPerson* person) throw() {
    if (!hasCORNPointerValue(MSCORN::CORN_P_VEH_PASSENGER)) {
        myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER] = new std::vector<MSPerson*>();
    }
    ((std::vector<MSPerson*>*) myPointerCORNMap[MSCORN::CORN_P_VEH_PASSENGER])->push_back(person);
    if (myStops.size() > 0 && myStops.front().reached && myStops.front().triggered) {
        myStops.front().duration = 0;
    }
}


void
MSVehicle::setBlinkerInformation() throw() {
		switchOffSignal(VEH_SIGNAL_BLINKER_RIGHT|VEH_SIGNAL_BLINKER_LEFT);
        int state = getLaneChangeModel().getState();
        if ((state&LCA_LEFT)!=0) {
            switchOnSignal(VEH_SIGNAL_BLINKER_LEFT);
        } else if ((state&LCA_RIGHT)!=0) {
			switchOnSignal(VEH_SIGNAL_BLINKER_RIGHT);
        } else {
            const MSLane &lane = getLane();
            MSLinkCont::const_iterator link = lane.succLinkSec(*this, 1, lane, getBestLanesContinuation());
            if (link!=lane.getLinkCont().end()&&lane.getLength()-getPositionOnLane()<lane.getMaxSpeed()*(SUMOReal) 7.) {
                switch ((*link)->getDirection()) {
                case MSLink::LINKDIR_TURN:
                case MSLink::LINKDIR_LEFT:
                case MSLink::LINKDIR_PARTLEFT:
		            switchOnSignal(VEH_SIGNAL_BLINKER_LEFT);
                    break;
                case MSLink::LINKDIR_RIGHT:
                case MSLink::LINKDIR_PARTRIGHT:
					switchOnSignal(VEH_SIGNAL_BLINKER_RIGHT);
                    break;
                default:
                    break;
                }
            }
        }

}


#ifndef NO_TRACI

bool
MSVehicle::startSpeedAdaption(float newSpeed, SUMOTime duration, SUMOTime currentTime) {
    if (newSpeed < 0 || duration <= 0/* || newSpeed >= getSpeed()*/) {
        return false;
    }
    speedBeforeAdaption = getSpeed();
    timeBeforeAdaption = currentTime;
    adaptDuration = duration;
    speedReduction = MAX2((SUMOReal) 0.0f, (SUMOReal)(speedBeforeAdaption - newSpeed));
    adaptingSpeed = true;
    return true;
}


void
MSVehicle::adaptSpeed() {
    SUMOReal maxSpeed = 0;
    SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
    if (!adaptingSpeed) {
        return;
    }
    if (isLastAdaption) {
        unsetIndividualMaxSpeed();
        adaptingSpeed = false;
        isLastAdaption = false;
        return;
    }
    if (currentTime <= timeBeforeAdaption + adaptDuration) {
        maxSpeed = speedBeforeAdaption - (speedReduction / adaptDuration)
                   * (currentTime - timeBeforeAdaption);
    } else {
        maxSpeed = speedBeforeAdaption - speedReduction;
        isLastAdaption = true;
    }
    setIndividualMaxSpeed(maxSpeed);
}


void
MSVehicle::checkLaneChangeConstraint(SUMOTime time) {
    if (!laneChangeConstraintActive) {
        return;
    }
    if ((time - timeBeforeLaneChange) >= laneChangeStickyTime) {
        laneChangeConstraintActive = false;
    }
}


void
MSVehicle::startLaneChange(unsigned lane, SUMOTime stickyTime) {
    if (lane < 0) {
        return;
    }
    timeBeforeLaneChange = MSNet::getInstance()->getCurrentTimeStep();
    laneChangeStickyTime = stickyTime;
    myDestinationLane = lane;
    laneChangeConstraintActive = true;
    checkForLaneChanges();
}


void
MSVehicle::checkForLaneChanges() {
    MSLane* tmpLane;
    unsigned currentLaneIndex = 0;
    if (!laneChangeConstraintActive) {
        myLaneChangeModel->requestLaneChange(REQUEST_NONE);
        return;
    }
    if ((unsigned int)(*myCurrEdge)->getLanes().size() <= myDestinationLane) {
        laneChangeConstraintActive = false;
        return;
    }
    tmpLane = myLane;
    while ((tmpLane =tmpLane->getRightLane()) != NULL) {
        currentLaneIndex++;
    }
    if (currentLaneIndex > myDestinationLane) {
        myLaneChangeModel->requestLaneChange(REQUEST_RIGHT);
    } else if (currentLaneIndex < myDestinationLane) {
        myLaneChangeModel->requestLaneChange(REQUEST_LEFT);
    } else {
        myLaneChangeModel->requestLaneChange(REQUEST_HOLD);
    }
}


void
MSVehicle::processTraCICommands(SUMOTime time) {
    // check for applied lane changing constraints
    checkLaneChangeConstraint(time);
    // change speed in case of previous "slowDown" command
    adaptSpeed();
}


bool
MSVehicle::addTraciStop(MSLane* lane, SUMOReal pos, SUMOReal radius, SUMOTime duration) {
    //if the stop exists update the duration
    for (std::list<Stop>::iterator iter = myStops.begin(); iter != myStops.end(); iter++) {
        if (iter->lane == lane && fabs(iter->endPos - pos) < POSITION_EPS) {
            if (duration == 0 && !iter->reached) {
                myStops.erase(iter);
            } else {
                iter->duration = duration;
            }
            return true;
        }
    }

    SUMOVehicleParameter::Stop newStop;
    newStop.lane = lane->getID();
    newStop.busstop = MSNet::getInstance()->getBusStopID(lane, pos);
    newStop.startPos = pos - POSITION_EPS;
    newStop.endPos = pos;
    newStop.duration = duration;
    newStop.until = -1;
    newStop.triggered = false;
    newStop.parking = false;
    newStop.index = STOP_INDEX_END;
    return addStop(newStop);
}


void 
MSVehicle::setTraCISpeed(SUMOReal speed) throw()
{
	myTraCISpeed = speed;
}

#endif


/****************************************************************************/
