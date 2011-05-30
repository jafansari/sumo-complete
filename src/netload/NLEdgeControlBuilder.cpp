/****************************************************************************/
/// @file    NLEdgeControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Interface for building edges
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

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <microsim/MSLane.h>
#include <microsim/MSInternalLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include "NLBuilder.h"
#include "NLEdgeControlBuilder.h"
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef HAVE_MESOSIM
#include <mesosim/MELoop.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
NLEdgeControlBuilder::NLEdgeControlBuilder()
        : myCurrentNumericalLaneID(0), myCurrentNumericalEdgeID(0), myEdges(0) {
    myActiveEdge = (MSEdge*) 0;
    m_pLaneStorage = new std::vector<MSLane*>();
    m_pDepartLane = (MSLane*) 0;
    m_iNoSingle = m_iNoMulti = 0;
}


NLEdgeControlBuilder::~NLEdgeControlBuilder() {
    delete m_pLaneStorage;
}


void
NLEdgeControlBuilder::beginEdgeParsing(
        const std::string &id, MSEdge::EdgeBasicFunction function, 
        const std::string& streetName) 
{
    myActiveEdge = buildEdge(id, streetName);
    if (!MSEdge::dictionary(id, myActiveEdge)) {
        throw InvalidArgument("Another edge with the id '" + id + "' exists.");
    }
    myEdges.push_back(myActiveEdge);
    m_pDepartLane = (MSLane*) 0;
    m_Function = function;
}


MSLane *
NLEdgeControlBuilder::addLane(const std::string &id,
                              SUMOReal maxSpeed, SUMOReal length, bool isDepart,
                              const PositionVector &shape, SUMOReal width, 
                              const SUMOVehicleClasses &allowed,
                              const SUMOVehicleClasses &disallowed) {
    // checks if the depart lane was set before
    if (isDepart&&m_pDepartLane!=0) {
        throw InvalidArgument("Lane's '" + id + "' edge already has a depart lane.");
    }
    MSLane *lane = 0;
    switch (m_Function) {
    case MSEdge::EDGEFUNCTION_INTERNAL:
        lane = new MSInternalLane(id, maxSpeed, length, myActiveEdge,
                                  myCurrentNumericalLaneID++, shape, width, allowed, disallowed);
        break;
    case MSEdge::EDGEFUNCTION_NORMAL:
    case MSEdge::EDGEFUNCTION_CONNECTOR:
        lane = new MSLane(id, maxSpeed, length, myActiveEdge,
                          myCurrentNumericalLaneID++, shape, width, allowed, disallowed);
        break;
    default:
        throw InvalidArgument("Unrecognised edge type.");
    }
    m_pLaneStorage->push_back(lane);
    if (isDepart) {
        m_pDepartLane = lane;
    }
    return lane;
}


MSEdge *
NLEdgeControlBuilder::closeEdge() {
    std::vector<MSLane*> *lanes = new std::vector<MSLane*>();
    lanes->reserve(m_pLaneStorage->size());
    copy(m_pLaneStorage->begin(), m_pLaneStorage->end(), back_inserter(*lanes));
    if (m_pLaneStorage->size()==1) {
        m_iNoSingle++;
    } else {
        m_iNoMulti++;
    }
    m_pLaneStorage->clear();
    myActiveEdge->initialize(m_pDepartLane, lanes, m_Function);
    return myActiveEdge;
}


MSEdgeControl *
NLEdgeControlBuilder::build() {
    for (EdgeCont::iterator i1=myEdges.begin(); i1!=myEdges.end(); i1++) {
        (*i1)->closeBuilding();
#ifdef HAVE_MESOSIM
        if (MSGlobals::gUseMesoSim) {
            MSGlobals::gMesoNet->buildSegmentsFor(**i1, OptionsCont::getOptions());
        }
#endif
    }
    return new MSEdgeControl(myEdges);
}


MSEdge *
NLEdgeControlBuilder::buildEdge(const std::string &id, const std::string &streetName) throw() {
    return new MSEdge(id, myCurrentNumericalEdgeID++, streetName);
}



/****************************************************************************/

