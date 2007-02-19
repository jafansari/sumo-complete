/****************************************************************************/
/// @file    NLSucceedingLaneBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 22 Oct 2001
/// @version $Id$
///
// }
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
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
#include <microsim/MSLane.h>
#include <microsim/MSInternalLane.h>
#include <microsim/MSLink.h>
#include <microsim/MSLinkCont.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include "NLBuilder.h"
#include "NLSucceedingLaneBuilder.h"
#include "NLJunctionControlBuilder.h"
#include <utils/xml/XMLBuildingExceptions.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/options/OptionsCont.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
NLSucceedingLaneBuilder::NLSucceedingLaneBuilder(NLJunctionControlBuilder &jb)
        : myJunctionControlBuilder(jb)
{
    m_SuccLanes = new MSLinkCont();
    m_SuccLanes->reserve(10);
}


NLSucceedingLaneBuilder::~NLSucceedingLaneBuilder()
{
    delete m_SuccLanes;
}


void
NLSucceedingLaneBuilder::openSuccLane(const string &laneId)
{
    m_CurrentLane = laneId;
}


void
NLSucceedingLaneBuilder::addSuccLane(bool yield, const string &laneId,
#ifdef HAVE_INTERNAL_LANES
                                     const std::string &viaID,
                                     SUMOReal pass,
#endif
                                     MSLink::LinkDirection dir,
                                     MSLink::LinkState state,
                                     bool internalEnd,
                                     const std::string &tlid, size_t linkNo)
{
    // check whether the link is a dead link
    if (laneId=="SUMO_NO_DESTINATION") {
        // build the dead link and add it to the container
#ifdef HAVE_INTERNAL_LANES
        m_SuccLanes->push_back(new MSLink(0, 0, yield, MSLink::LINKDIR_NODIR, MSLink::LINKSTATE_DEADEND, false));
#else
        m_SuccLanes->push_back(new MSLink(0, yield, MSLink::LINKDIR_NODIR, MSLink::LINKSTATE_DEADEND));
#endif
        return;
    }
    // get the lane the link belongs to
    MSLane *lane = MSLane::dictionary(laneId);
    if (lane==0) {
        throw XMLIdNotKnownException("lane", laneId);
    }
#ifdef HAVE_INTERNAL_LANES
    MSLane *via = 0;
    if (viaID!="" && OptionsSubSys::getOptions().getBool("use-internal-links")) {
        via = MSLane::dictionary(viaID);
        if (via==0) {
            throw XMLIdNotKnownException("lane", viaID);
        }
    }
    if (pass>=0) {
        static_cast<MSInternalLane*>(lane)->setPassPosition(pass);
    }
#endif
    // check whether this link is controlled by a traffic light
    MSTLLogicControl::TLSLogicVariants logics;
    if (tlid!="") {
        logics = myJunctionControlBuilder.getTLLogic(tlid);
        if (logics.ltVariants.size()==0) {
            throw XMLIdNotKnownException("tl-logic", tlid);
        }
    }
    // build the link
    // if internal lanes are used, the next lane of a normal edge
    // will be an internal lane
    /*    if(via!=0&&OptionsSubSys::getOptions().getBool("use-internal-links")) {
            lane = via;
        } else {
            via = 0;
        }*/
#ifdef HAVE_INTERNAL_LANES
    MSLink *link = new MSLink(lane, via, yield, dir, state, internalEnd);
#else
    MSLink *link = new MSLink(lane, yield, dir, state);
#endif
    // if a traffic light is responsible for it, inform the traffic light
    if (logics.ltVariants.size()!=0) {
        MSLane *current = MSLane::dictionary(m_CurrentLane);
        if (current==0) {
            throw XMLIdNotKnownException("lane", m_CurrentLane);
        }
        std::map<std::string, MSTrafficLightLogic *>::iterator i;
        for (i=logics.ltVariants.begin(); i!=logics.ltVariants.end(); ++i) {
            (*i).second->addLink(link, current, linkNo);
        }
    }
    // add the link to the container
    m_SuccLanes->push_back(link);
}


void
NLSucceedingLaneBuilder::closeSuccLane()
{
    MSLane *current = MSLane::dictionary(m_CurrentLane);
    if (current==0) {
        throw XMLIdNotKnownException("lane", m_CurrentLane);
    }
    MSLinkCont *cont = new MSLinkCont();
    cont->reserve(m_SuccLanes->size());
    copy(m_SuccLanes->begin(), m_SuccLanes->end(), back_inserter(*cont));
    current->initialize(/*m_Junction, */cont);
    m_SuccLanes->clear();
}


std::string
NLSucceedingLaneBuilder::getSuccingLaneName() const
{
    return m_CurrentLane;
}



/****************************************************************************/

