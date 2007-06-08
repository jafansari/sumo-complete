/****************************************************************************/
/// @file    MSEmitter.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 21.07.2005
/// @version $Id$
///
// A vehicle emitting device
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/helpers/Command.h>
#include <utils/helpers/WrappingCommand.h>
#include <microsim/MSLane.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <microsim/MSEventControl.h>
#include "MSEmitter.h"
#include <microsim/MSGlobals.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;



/* -------------------------------------------------------------------------
 * MSTriggeredReader::UserCommand-methods
 * ----------------------------------------------------------------------- */
MSEmitter::MSEmitter_FileTriggeredChild::MSEmitter_FileTriggeredChild(
    MSNet &net, const std::string &aXMLFilename,
    MSEmitter &parent, MSVehicleControl &vc)
        : MSTriggeredXMLReader(net, aXMLFilename), MSEmitterChild(parent, vc),
        myHaveNext(false), myFlow(-1), myHaveInitialisedFlow(false), myRunningID(0)
{
    myBeginTime = net.getCurrentTimeStep();
}


MSEmitter::MSEmitter_FileTriggeredChild::~MSEmitter_FileTriggeredChild()
{}


SUMOTime
MSEmitter::MSEmitter_FileTriggeredChild::execute(SUMOTime)
{
    if (myParent.childCheckEmit(this)) {
        buildAndScheduleFlowVehicle();
        return (SUMOTime) computeOffset(myFlow);
    }
    return 1;
}


bool
MSEmitter::MSEmitter_FileTriggeredChild::processNextEntryReaderTriggered()
{
    if (myFlow>=0) {
        return true;
    }
    if (!myHaveNext) {
        return true;
    }
    if (myParent.childCheckEmit(this)) {
        myHaveNext = false;
        return true;
    }
    return false;
}


void
MSEmitter::MSEmitter_FileTriggeredChild::buildAndScheduleFlowVehicle()
{
    string aVehicleId = myParent.getID() + "_" + toString(myRunningID++);
    MSVehicleType* aVehType = myVTypeDist.getOverallProb()>0
                              ? myVTypeDist.get()
                              : MSNet::getInstance()->getVehicleControl().getRandomVType();
    if (aVehType==0) {
        WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": no valid vehicle type exists.");
        WRITE_WARNING("Continuing with next element.");
        return;// false;
    }
    // check and assign vehicle type
    MSRoute *aEmitRoute = myRouteDist.get();
    if (aEmitRoute==0) {
        WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": no valid route exsists.");
        WRITE_WARNING("Continuing with next element.");
        return;// false;
    }

    MSVehicle *veh =
        MSNet::getInstance()->getVehicleControl().buildVehicle(
            aVehicleId, aEmitRoute, _offset+1, aVehType, 0, 0);
    myParent.schedule(this, veh, -1);
    myHaveNext = true;
}


void
MSEmitter::MSEmitter_FileTriggeredChild::myStartElement(SumoXMLTag /*element*/,
        const std::string &name,
        const Attributes &attrs) throw()
{
    if (name=="routedistelem") {
        // parse route distribution
        // check if route exists
        string routeStr = getStringSecure(attrs, SUMO_ATTR_ID, "");
        MSRoute* route = MSRoute::dictionary(routeStr);
        if (route == 0) {
            MsgHandler::getErrorInstance()->inform(
                "MSTriggeredSource " + myParent.getID() + ": Route '" + routeStr + "' does not exist.");
            throw ProcessError();
        }
        // check frequency
        SUMOReal freq = getFloatSecure(attrs, SUMO_ATTR_PROB, -1);
        if (freq<0) {
            MsgHandler::getErrorInstance()->inform(
                "MSTriggeredSource " + myParent.getID() + ": Attribute \"probability\" has value < 0.");
            throw ProcessError();
        }
        // Attributes ok, add to routeDist
        myRouteDist.add(freq, route);
        return;
    }
    // vehicle-type distributions
    if (name=="vtypedistelem") {
        SUMOReal prob = -1;
        try {
            prob = getFloatSecure(attrs, SUMO_ATTR_PROB, -1);
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("False probability while parsing calibrator '" + myParent.getID() + "' (" + getStringSecure(attrs, SUMO_ATTR_PROB, "") + ").");
            return;
        }
        if (prob<=0) {
            MsgHandler::getErrorInstance()->inform("False probability while parsing calibrator '" + myParent.getID() + "' (" + toString(prob) + ").");
            return;
        }
        // get the id
        string id;
        try {
            id = getString(attrs, SUMO_ATTR_ID);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("Error in description: missing id of a vtype-object.");
            return;
        }
        MSVehicleType *vtype = MSNet::getInstance()->getVehicleControl().getVType(id);
        if (vtype==0) {
            MsgHandler::getErrorInstance()->inform("Error in description: unknown vtype-object '" + id + "'.");
            return;
        }
        myVTypeDist.add(prob, vtype);
    }

    if (name=="flow") {
        // get the flow information
        SUMOReal no = -1;
        try {
            no = getFloatSecure(attrs, SUMO_ATTR_NO, -1);
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("Non-numeric flow in emitter '" + myParent.getID() + "' (" + getStringSecure(attrs, SUMO_ATTR_NO, "") + ").");
            return;
        }
        if (no<0) {
            MsgHandler::getErrorInstance()->inform("Negative flow in emitter '" + myParent.getID() + "' (" + getStringSecure(attrs, SUMO_ATTR_NO, "") + ").");
            return;
        }
        // get the end of this def
        SUMOTime end = -1;
        try {
            end = (SUMOTime) getFloatSecure(attrs, SUMO_ATTR_END, -1);
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("Non-numeric flow end in emitter '" + myParent.getID() + "' (" + getStringSecure(attrs, SUMO_ATTR_NO, "") + ").");
            return;
        }

        myFlow = (SUMOReal) no;

        if (end==-1||end>=MSNet::getInstance()->getCurrentTimeStep()) {
            if (myFlow>0) {
                buildAndScheduleFlowVehicle();
                MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(
                    new WrappingCommand<MSEmitter::MSEmitter_FileTriggeredChild>(this, &MSEmitter::MSEmitter_FileTriggeredChild::execute),
                    (SUMOTime)(1. / (myFlow / 3600.))+MSNet::getInstance()->getCurrentTimeStep(),
                    MSEventControl::ADAPT_AFTER_EXECUTION);
                myHaveInitialisedFlow = true;
            }
        }
    }

    // check whethe the correct tag is read
    if (name=="emit") {
        // check and assign emission time
        int aEmitTime = getIntSecure(attrs, SUMO_ATTR_TIME, -1);
        if (aEmitTime<myBeginTime) {
            // do not process the vehicle if the emission time is before the simulation begin
            return;
        }
        // check and assign id
        string aVehicleId = getStringSecure(attrs, SUMO_ATTR_ID, "");
        if (myVehicleControl.getVehicle(aVehicleId)!=0) {
            WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": Vehicle " + aVehicleId + " already exists.\n Generating a default id.");
            aVehicleId = "";
        }
        if (aVehicleId=="") {
            aVehicleId = myParent.getID() +  "_" + toString(aEmitTime) +  "_" + toString(myRunningID++);
            if (myVehicleControl.getVehicle(aVehicleId)!=0) {
                WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": Vehicle " + aVehicleId + " already exists.\n Continuing with next element.");
                return;// false;
            }
        }
        // check and assign vehicle type
        string emitType = getStringSecure(attrs, SUMO_ATTR_VEHTYPE, "");
        MSVehicleType* aVehType = MSNet::getInstance()->getVehicleControl().getVType(emitType);
        if (aVehType == 0) {
            if (myVTypeDist.getOverallProb()!=0) {
                aVehType = myVTypeDist.get();
            }
            if (aVehType==0) {
                aVehType = MSNet::getInstance()->getVehicleControl().getRandomVType();
                if (aVehType==0) {
                    WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": no valid vehicle type exists.");
                    WRITE_WARNING("Continuing with next element.");
                    return;// false;
                }
            }
        }
        // check and assign vehicle type
        string emitRoute = getStringSecure(attrs, SUMO_ATTR_ROUTE, "");
        MSRoute *aEmitRoute = MSRoute::dictionary(emitRoute);
        if (aEmitRoute==0) {
            if (myRouteDist.getOverallProb()!=0) {
                aEmitRoute = myRouteDist.get();
            }
            if (aEmitRoute==0) {
                WRITE_WARNING("MSTriggeredSource " + myParent.getID()+ ": no valid route exsists.");
                WRITE_WARNING("Continuing with next element.");
                return;// false;
            }
        }
        // build vehicle
        SUMOReal aEmitSpeed = getFloatSecure(attrs, SUMO_ATTR_SPEED, -1);
        MSVehicle *veh =
            MSNet::getInstance()->getVehicleControl().buildVehicle(
                aVehicleId, aEmitRoute, aEmitTime, aVehType, 0, 0);
        myParent.schedule(this, veh, aEmitSpeed);
        myHaveNext = true;
        _offset = SUMOTime(aEmitTime);
    }
    // check whethe the correct tag is read
    if (name=="reset") {
        myVTypeDist.clear();
        myRouteDist.clear();
    }
}


void
MSEmitter::MSEmitter_FileTriggeredChild::myCharacters(
    SumoXMLTag , const std::string &, const std::string &) throw()
{}


void
MSEmitter::MSEmitter_FileTriggeredChild::myEndElement(
    SumoXMLTag , const std::string &) throw()
{}


bool
MSEmitter::MSEmitter_FileTriggeredChild::nextRead()
{
    return myHaveNext;
}


SUMOReal
MSEmitter::MSEmitter_FileTriggeredChild::getLoadedFlow() const
{
    return myFlow;
}


void
MSEmitter::MSEmitter_FileTriggeredChild::inputEndReached()
{
    if (myFlow>0&&!myHaveInitialisedFlow) {
        buildAndScheduleFlowVehicle();
        MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(
            new WrappingCommand<MSEmitter::MSEmitter_FileTriggeredChild>(this, &MSEmitter::MSEmitter_FileTriggeredChild::execute),
            (SUMOTime)(1. / (myFlow / 3600.))+MSNet::getInstance()->getCurrentTimeStep(),
            MSEventControl::ADAPT_AFTER_EXECUTION);
        myHaveInitialisedFlow = true;
    }
}


// ===========================================================================
// method definitions
// ===========================================================================
MSEmitter::MSEmitter(const std::string &id,
                     MSNet &net,
                     MSLane* destLane, SUMOReal pos,
                     const std::string &aXMLFilename)
        : MSTrigger(id), myNet(net),
        myDestLane(destLane), myPos((SUMOReal) pos)
{
    assert(myPos>=0);
    myActiveChild =
        new MSEmitter_FileTriggeredChild(net, aXMLFilename, *this, net.getVehicleControl());
    myFileBasedEmitter = myActiveChild;
}


MSEmitter::~MSEmitter()
{
    {
        delete myFileBasedEmitter;
    }
    {
        std::map<MSEmitterChild*, std::pair<MSVehicle*, SUMOReal> >::iterator i;
        for (i=myToEmit.begin(); i!=myToEmit.end(); ++i) {
            delete(*i).second.first;
        }
    }
}


bool
MSEmitter::childCheckEmit(MSEmitterChild *child)
{
    if (myToEmit.find(child)==myToEmit.end()) {
        // should not happen - a child is calling and should have a vehicle added
        throw 1;
    }
    if (child!=myActiveChild) {
        // remove the vehicle previously inserted by the child
        delete myToEmit[child].first;
        // erase the child information
        myToEmit.erase(myToEmit.find(child));
        // inform child to process the next one (the current was not used)
        return true;
    }
    // get the vehicle and the speed the child has read/generated
    MSVehicle *veh = myToEmit[child].first;
    SUMOReal speed = myToEmit[child].second;
    // check whether the speed shall be patched
    MSVehicle::State state(myPos, MIN2(myDestLane->maxSpeed(), veh->getMaxSpeed()));
    if (speed>=0) {
        state = MSVehicle::State(myPos, speed);
    }
    // try to emit
#ifdef HAVE_MESOSIM
    if (MSGlobals::gUseMesoSim) {
        if (myDestLane->getEdge()->emit(*veh,  myNet.getCurrentTimeStep())) {
            myNet.getVehicleControl().vehiclesEmitted(1);
            veh->onDepart();
            // insert vehicle into the dictionary
            if (!myNet.getVehicleControl().addVehicle(veh->getID(), veh)) {
                // !!!
                throw 1;
            }
            // erase the child information
            myToEmit.erase(myToEmit.find(child));
            return true;
        }
    } else {
#endif
        if (myDestLane->isEmissionSuccess(veh, state)) {
            veh->enterLaneAtEmit(myDestLane, state);
            veh->onDepart();
            myNet.getVehicleControl().vehiclesEmitted(1);
            // insert vehicle into the dictionary
            if (!myNet.getVehicleControl().addVehicle(veh->getID(), veh)) {
                // !!!
                throw 1;
            }
            // erase the child information
            myToEmit.erase(myToEmit.find(child));
            return true;
        }
#ifdef HAVE_MESOSIM
    }
#endif
    return false;
}


void
MSEmitter::schedule(MSEmitterChild *child,
                    MSVehicle *v, SUMOReal speed)
{
    myToEmit[child] = make_pair(v, speed);
}


size_t
MSEmitter::getActiveChildIndex() const
{
    return
        myFileBasedEmitter==myActiveChild ? 0 : 1;
}


void
MSEmitter::setActiveChild(MSEmitterChild *c)
{
    myActiveChild = c;
}


/****************************************************************************/

