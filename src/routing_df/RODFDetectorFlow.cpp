/****************************************************************************/
/// @file    RODFDetectorFlow.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// Storage for flows within the DFROUTER
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

#include <iostream>
#include <cassert>
#include "RODFDetectorFlow.h"

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
RODFDetectorFlows::RODFDetectorFlows(SUMOTime startTime, SUMOTime endTime,
                                     SUMOTime stepOffset)
        : myBeginTime(startTime), myEndTime(endTime), myStepOffset(stepOffset),
        myMaxDetectorFlow(-1)
{}


RODFDetectorFlows::~RODFDetectorFlows()
{}


void
RODFDetectorFlows::addFlow(const std::string &id, int t, const FlowDef &fd)
{
    if (myFastAccessFlows.find(id)==myFastAccessFlows.end()) {
        size_t noItems = (size_t)((myEndTime-myBeginTime)/myStepOffset);
        myFastAccessFlows[id] = std::vector<FlowDef>(noItems);
        std::vector<FlowDef> &cflows = myFastAccessFlows.find(id)->second;
        // initialise
        for (std::vector<FlowDef>::iterator i=cflows.begin(); i<cflows.end(); ++i) {
            (*i).qPKW = 0;
            (*i).qLKW = 0;
            (*i).vPKW = 0;
            (*i).vLKW = 0;
            (*i).fLKW = 0;
            (*i).isLKW = 0;
            (*i).firstSet = true;
        }
    }
    assert(t<(int) myFastAccessFlows[id].size());
    FlowDef &ofd = myFastAccessFlows[id][t];
    if (ofd.firstSet) {
        ofd = fd;
        ofd.firstSet = false;
    } else {
        ofd.qLKW = ofd.qLKW + fd.qLKW;
        ofd.qPKW = ofd.qPKW + fd.qPKW;
        ofd.vLKW = ofd.vLKW + fd.vLKW;
        ofd.vPKW = ofd.vPKW + fd.vPKW;
    }
    if (ofd.qLKW!=0 && ofd.qPKW!=0) {
        ofd.fLKW = ofd.qLKW / ofd.qPKW ;
    } else {
        ofd.fLKW = 0;
    }
}




void
RODFDetectorFlows::setFlows(const std::string &detector_id,
                            std::vector<FlowDef> &flows)
{
    for (std::vector<FlowDef>::iterator i=flows.begin(); i<flows.end(); ++i) {
        FlowDef &ofd = *i;
        if (ofd.qLKW!=0 && ofd.qPKW!=0) {
            ofd.fLKW = ofd.qLKW / ofd.qPKW ;
        } else {
            ofd.fLKW = 0;
        }
    }
    myFastAccessFlows[detector_id] = flows;
}


void
RODFDetectorFlows::removeFlow(const std::string &detector_id)
{
    if (myFastAccessFlows.find(detector_id)!=myFastAccessFlows.end()) {
        myFastAccessFlows.erase(myFastAccessFlows.find(detector_id));
    }
}

/*
const FlowDef &
RODFDetectorFlows::getFlowDef( const std::string &id, int t ) const
{
	return myCurrFlows.find(id)->second.find(t)->second;
}


const std::map< int, FlowDef > &
RODFDetectorFlows::getFlowDefs( const std::string &id ) const
{
	return myCurrFlows.find(id)->second;
}
*/

bool
RODFDetectorFlows::knows(const std::string &det_id) const
{
    return myFastAccessFlows.find(det_id)!=myFastAccessFlows.end();
}


bool
RODFDetectorFlows::knows(const std::string &det_id, SUMOTime /*time*/) const
{
    if (myFastAccessFlows.find(det_id)==myFastAccessFlows.end()) {
        return false;
    }
    return true;//!!!
}

/*
void
RODFDetectorFlows::buildFastAccess(SUMOTime startTime, SUMOTime endTime,
                                 SUMOTime stepOffset)
{
    myBeginTime = startTime;
    myEndTime = endTime;
    myStepOffset = stepOffset;
    size_t noItems = (size_t) ((endTime-startTime)/stepOffset);
    std::map<std::string, std::map<SUMOTime, FlowDef> >::iterator j;
    // go through detectors
    for(j=myCurrentFlows.begin(); j!=myCurrentFlows.end(); ++j) {
        myFastAccessFlows[(*j).first] = std::vector<FlowDef>(noItems);
        std::vector<FlowDef> cflows = myFastAccessFlows.find((*j).first)->second;
        // initialise
        for(size_t k=0; k<noItems; k++) {
            cflows[k].qPKW = 0;
            cflows[k].qLKW = 0;
            cflows[k].vPKW = 0;
            cflows[k].vLKW = 0;
            cflows[k].fLKW = 0;
            cflows[k].isLKW = 0;
        }
        // build
        std::map<SUMOTime, FlowDef> &prev = myCurrentFlows.find((*j).first)->second;
        for(std::map<SUMOTime, FlowDef>::iterator i=prev.begin(); i!=prev.end(); i++) {
            cflows[(*i).first - myBeginTime] = (*i).second;
        }
    }
}
*/

const std::vector<FlowDef> &
RODFDetectorFlows::getFlowDefs(const std::string &id) const
{
    assert(myFastAccessFlows.find(id)!=myFastAccessFlows.end());
    assert(myFastAccessFlows.find(id)->second.size()!=0);
    return myFastAccessFlows.find(id)->second;
}


SUMOReal
RODFDetectorFlows::getFlowSumSecure(const std::string &id) const
{
    SUMOReal ret = 0;
    if (knows(id)) {
        const std::vector<FlowDef> &flows = getFlowDefs(id);
        for (std::vector<FlowDef>::const_iterator i=flows.begin(); i!=flows.end(); ++i) {
            ret += (*i).qPKW;
            ret += (*i).qLKW;
        }
    }
    return ret;
}


SUMOReal
RODFDetectorFlows::getMaxDetectorFlow() const
{
    if (myMaxDetectorFlow<0) {
        SUMOReal max = 0;
        std::map<std::string, std::vector<FlowDef> >::const_iterator j;
        for (j=myFastAccessFlows.begin(); j!=myFastAccessFlows.end(); ++j) {
            SUMOReal curr = 0;
            const std::vector<FlowDef> &flows = (*j).second;
            for (std::vector<FlowDef>::const_iterator i=flows.begin(); i!=flows.end(); ++i) {
                curr += (*i).qPKW;
                curr += (*i).qLKW;
            }
            if (max<curr) {
                max = curr;
            }
        }
        myMaxDetectorFlow = max;
    }
    return myMaxDetectorFlow;
}


void
RODFDetectorFlows::mesoJoin(const std::string &nid,
                            const std::vector<std::string> &oldids)
{
    for (std::vector<std::string>::const_iterator i=oldids.begin(); i!=oldids.end(); ++i) {
        if (!knows(*i)) {
            continue;
        }
        std::map<std::string, std::vector<FlowDef> >::iterator j = myFastAccessFlows.find(*i);
        std::vector<FlowDef> &flows = (*j).second;
        size_t index = 0;
        for (SUMOTime t=myBeginTime; t!=myEndTime; t+=myStepOffset) {
            addFlow(nid, t/myStepOffset, flows[index++]); // !!!
        }
        myFastAccessFlows.erase(j);
    }
}


void
RODFDetectorFlows::printAbsolute() const
{
    for (std::map<std::string, std::vector<FlowDef> >::const_iterator i=myFastAccessFlows.begin(); i!=myFastAccessFlows.end(); ++i) {
        cout << (*i).first << ":";
        const std::vector<FlowDef> &flows = (*i).second;
        SUMOReal qPKW = 0;
        SUMOReal qLKW = 0;
        for (std::vector<FlowDef>::const_iterator j=flows.begin(); j!=flows.end(); ++j) {
            qPKW += (*j).qPKW;
            qLKW += (*j).qLKW;
        }
        cout << qPKW << "/" << qLKW << endl;
    }
}

/****************************************************************************/

