/****************************************************************************/
/// @file    NLJunctionControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Builder of microsim-junctions and tls
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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

#include <map>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <microsim/MSJunctionLogic.h>
#include <microsim/MSNoLogicJunction.h>
#include <microsim/MSRightOfWayJunction.h>
#include <microsim/MSInternalJunction.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/traffic_lights/MSAgentbasedTrafficLightLogic.h>
#include <microsim/traffic_lights/MSOffTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <netbuild/NBNode.h>
#include "NLBuilder.h"
#include "NLJunctionControlBuilder.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// static members
// ===========================================================================
const int NLJunctionControlBuilder::NO_REQUEST_SIZE = -1;

// ===========================================================================
// method definitions
// ===========================================================================
NLJunctionControlBuilder::NLJunctionControlBuilder(MSNet &net, NLDetectorBuilder &db) throw()
        : myNet(net), myDetectorBuilder(db), myOffset(0), myJunctions(0) {
    myLogicControl = new MSTLLogicControl();
    myJunctions = new MSJunctionControl();
}


NLJunctionControlBuilder::~NLJunctionControlBuilder() throw() {
    delete myLogicControl;
    delete myJunctions;
}


void
NLJunctionControlBuilder::openJunction(const std::string &id,
                                       const std::string &key,
                                       const std::string &type,
                                       SUMOReal x, SUMOReal y,
                                       const PositionVector &shape,
                                       const std::vector<MSLane*> &incomingLanes,
                                       const std::vector<MSLane*> &internalLanes) throw(InvalidArgument) {
#ifdef HAVE_INTERNAL_LANES
    myActiveInternalLanes = internalLanes;
#endif
    myActiveIncomingLanes = incomingLanes;
    myActiveID = id;
    myActiveKey = key;
    if (!SUMOXMLDefinitions::NodeTypes.hasString(type)) {
        throw InvalidArgument("An unknown or invalid junction type '" + type + "' occured in junction '" + id + "'.");
    }

    myType = SUMOXMLDefinitions::NodeTypes.get(type);
    myPosition.set(x, y);
    myShape = shape;
}


void
NLJunctionControlBuilder::closeJunction() throw(InvalidArgument, ProcessError) {
    if (myJunctions==0) {
        throw ProcessError("Information about the number of nodes was missing.");
    }
    MSJunction *junction = 0;
    switch (myType) {
    case NODETYPE_NOJUNCTION:
    case NODETYPE_DEAD_END:
    case NODETYPE_DEAD_END_DEPRECATED:
    case NODETYPE_DISTRICT:
        junction = buildNoLogicJunction();
        break;
    case NODETYPE_TRAFFIC_LIGHT:
    case NODETYPE_RIGHT_BEFORE_LEFT:
    case NODETYPE_PRIORITY_JUNCTION:
        junction = buildLogicJunction();
        break;
    case NODETYPE_INTERNAL:
#ifdef HAVE_INTERNAL_LANES
        if (MSGlobals::gUsingInternalLanes) {
            junction = buildInternalJunction();
        }
#endif
        break;
    default:
        throw InvalidArgument("False junction logic type.");
    }
    if (junction!=0) {
        if (!myJunctions->add(myActiveID, junction)) {
            throw InvalidArgument("Another junction with the id '" + myActiveID + "' exists.");
        }
    }
}


MSJunctionControl *
NLJunctionControlBuilder::build() const throw() {
    MSJunctionControl *js = myJunctions;
    myJunctions = 0;
    return js;
}


MSJunction *
NLJunctionControlBuilder::buildNoLogicJunction() throw() {
    return new MSNoLogicJunction(myActiveID, myPosition, myShape, myActiveIncomingLanes
#ifdef HAVE_INTERNAL_LANES
                                 , myActiveInternalLanes
#endif
                                );
}


MSJunction *
NLJunctionControlBuilder::buildLogicJunction() throw(InvalidArgument) {
    MSJunctionLogic *jtype = getJunctionLogicSecure();
    // build the junction
    return new MSRightOfWayJunction(myActiveID, myPosition, myShape, myActiveIncomingLanes,
#ifdef HAVE_INTERNAL_LANES
                                    myActiveInternalLanes,
#endif
                                    jtype);
}


#ifdef HAVE_INTERNAL_LANES
MSJunction *
NLJunctionControlBuilder::buildInternalJunction() throw() {
    // build the junction
    return new MSInternalJunction(myActiveID, myPosition, myShape, myActiveIncomingLanes,
                                  myActiveInternalLanes);
}
#endif


MSJunctionLogic *
NLJunctionControlBuilder::getJunctionLogicSecure() throw(InvalidArgument) {
    // get and check the junction logic
    if (myLogics.find(myActiveID)==myLogics.end()) {
        throw InvalidArgument("Missing junction logic '" + myActiveID + "'.");
    }
    return myLogics[myActiveID];
}


MSTLLogicControl::TLSLogicVariants &
NLJunctionControlBuilder::getTLLogic(const std::string &id) const throw(InvalidArgument) {
    return getTLLogicControlToUse().get(id);
}


void
NLJunctionControlBuilder::closeTrafficLightLogic() throw(InvalidArgument, ProcessError) {
    if (myActiveProgram=="off") {
        if (myAbsDuration>0) {
            throw InvalidArgument("The off program for TLS '" + myActiveKey + "' has phases.");
        }
        if (!getTLLogicControlToUse().add(myActiveKey, myActiveProgram,
                                          new MSOffTrafficLightLogic(getTLLogicControlToUse(), myActiveKey))) {
            throw InvalidArgument("Another logic with id '" + myActiveKey + "' and subid '" + myActiveProgram + "' exists.");
        }
        return;
    }
    if (myAbsDuration==0) {
        throw InvalidArgument("TLS program '" + myActiveProgram + "' for TLS '" + myActiveKey + "' has a duration of 0.");
    }
    // compute the initial step and first switch time of the tls-logic
    unsigned int step = 0;
    SUMOTime firstEventOffset = 0;
    SUMOTime offset = (myNet.getCurrentTimeStep() + myOffset % myAbsDuration) % myAbsDuration;
    MSSimpleTrafficLightLogic::Phases::const_iterator i = myActivePhases.begin();
    while (offset>=(*i)->duration) {
        step++;
        offset -= (*i)->duration;
        ++i;
    }
    firstEventOffset = (*i)->duration - offset + myNet.getCurrentTimeStep();

    //
    if (myActiveProgram=="") {
        myActiveProgram = "default";
    }
    MSTrafficLightLogic *tlLogic = 0;
    // build the tls-logic in dependance to its type
    switch(myLogicType) {
        case TLTYPE_ACTUATED:
            tlLogic = new MSActuatedTrafficLightLogic(getTLLogicControlToUse(),
                    myActiveKey, myActiveProgram,
                    myActivePhases, step, firstEventOffset, myAdditionalParameter);
            break;
        case TLTYPE_AGENT:
            tlLogic = new MSAgentbasedTrafficLightLogic(getTLLogicControlToUse(),
                    myActiveKey, myActiveProgram,
                    myActivePhases, step, firstEventOffset, myAdditionalParameter);
            break;
        case TLTYPE_STATIC:
            tlLogic =
                new MSSimpleTrafficLightLogic(getTLLogicControlToUse(),
                        myActiveKey, myActiveProgram,
                        myActivePhases, step, firstEventOffset);
            tlLogic->setParameter(myAdditionalParameter);
    }
    TLInitInfo ii;
    ii.logic = tlLogic;
    ii.params = myAdditionalParameter;
    myJunctions2PostLoadInit.push_back(ii);
    myActivePhases.clear();
    if (tlLogic!=0) {
        try {
            if (!getTLLogicControlToUse().add(myActiveKey, myActiveProgram, tlLogic)) {
                throw InvalidArgument("Another logic with id '" + myActiveKey + "' and subid '" + myActiveProgram + "' exists.");
            }
            tlLogic->setParameter(myAdditionalParameter);
            tlLogic->init(myDetectorBuilder);
        } catch (InvalidArgument &) {
            delete tlLogic;
            throw;
        }
    }
}


void
NLJunctionControlBuilder::initJunctionLogic(const std::string &id) throw() {
    myActiveKey = id;
    myActiveProgram = "";
    myActiveLogic.clear();
    myActiveFoes.clear();
    myActiveConts.reset();
    myRequestSize = NO_REQUEST_SIZE; // seems not to be used
    myRequestItemNumber = 0;
    myCurrentHasError = false;
}


void
NLJunctionControlBuilder::addLogicItem(int request,
                                       const std::string &response,
                                       const std::string &foes,
                                       bool cont) throw(InvalidArgument) {
    if (myCurrentHasError) {
        // had an error
        return;
    }
    if (request>63) {
        // bad request
        myCurrentHasError = true;
        throw InvalidArgument("Junction logic '" + myActiveKey + "' is larger than allowed; recheck the network.");
    }
    if (myRequestSize == NO_REQUEST_SIZE) {
        // initialize
        myRequestSize = (int)response.size();
    }
    if (response.size() != myRequestSize) {
        myCurrentHasError = true;
        throw InvalidArgument("Invalid response size " + toString(response.size()) +
                              " in Junction logic '" + myActiveKey + "' (expected  " + toString(myRequestSize) + ")");
    }
    if (foes.size() != myRequestSize) {
        myCurrentHasError = true;
        throw InvalidArgument("Invalid foes size " + toString(foes.size()) +
                              " in Junction logic '" + myActiveKey + "' (expected  " + toString(myRequestSize) + ")");
    }
    // assert that the logicitems come ordered by their request index
    assert(myActiveLogic.size() == (size_t) request);
    assert(myActiveFoes.size() == (size_t) request);
    // add the read response for the given request index
    myActiveLogic.push_back(std::bitset<64>(response));
    // add the read junction-internal foes for the given request index
    myActiveFoes.push_back(std::bitset<64>(foes));
    // add whether the vehicle may drive a little bit further
    myActiveConts.set(request, cont);
    // increse number of set information
    myRequestItemNumber++;
}


void
NLJunctionControlBuilder::initTrafficLightLogic(const std::string &id, const std::string &programID,
        TrafficLightType type, SUMOTime offset) throw() {
    myActiveKey = id;
    myActiveProgram = programID;
    myActivePhases.clear();
    myAbsDuration = 0;
    myRequestSize = NO_REQUEST_SIZE;
    myLogicType = type;
    myOffset = offset;
    myAdditionalParameter.clear();
}


void
NLJunctionControlBuilder::addPhase(SUMOTime duration, const std::string &state,
                                   int minDuration, int maxDuration) throw() {
    // build and add the phase definition to the list
    myActivePhases.push_back(new MSPhaseDefinition(duration, minDuration, maxDuration, state));
    // add phase duration to the absolute duration
    myAbsDuration += duration;
}


void
NLJunctionControlBuilder::closeJunctionLogic() throw(InvalidArgument) {
    if (myRequestSize == NO_REQUEST_SIZE) {
        // We have a legacy network. junction element did not contain logicitems; read the logic later
        return;
    }
    if (myCurrentHasError) {
        // had an error before...
        return;
    }
    if (myRequestItemNumber!=myRequestSize) {
        throw InvalidArgument("The description for the junction logic '" + myActiveKey + "' is malicious.");
    }
    if (myLogics.count(myActiveKey) > 0) {
        throw InvalidArgument("Junction logic '" + myActiveKey + "' was defined twice.");
    }
    MSJunctionLogic *logic = new MSBitsetLogic(myRequestSize,
            new MSBitsetLogic::Logic(myActiveLogic),
            new MSBitsetLogic::Foes(myActiveFoes),
            myActiveConts);
    myLogics[myActiveKey] = logic;
}


MSTLLogicControl *
NLJunctionControlBuilder::buildTLLogics() const throw(ProcessError) {
    if (!myLogicControl->closeNetworkReading()) {
        throw ProcessError("Traffic lights could not be built.");
    }
    MSTLLogicControl *ret = myLogicControl;
    myLogicControl = 0;
    return ret;
}


void
NLJunctionControlBuilder::addParam(const std::string &key,
                                   const std::string &value) throw() {
    myAdditionalParameter[key] = value;
}


MSTLLogicControl &
NLJunctionControlBuilder::getTLLogicControlToUse() const throw() {
    if (myLogicControl!=0) {
        return *myLogicControl;
    }
    return myNet.getTLSControl();
}


const std::string &
NLJunctionControlBuilder::getActiveKey() const throw() {
    return myActiveKey;
}


const std::string &
NLJunctionControlBuilder::getActiveSubKey() const throw() {
    return myActiveProgram;
}


/****************************************************************************/
