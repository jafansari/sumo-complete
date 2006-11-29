//---------------------------------------------------------------------------//
//                        ROEdge.cpp -
//  An edge the router may route through
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.33  2006/11/29 07:51:22  dkrajzew
// added the possibility to use the loaded weights outside their boundaries
//
// Revision 1.32  2006/11/17 09:09:58  dkrajzew
// warnings removed
//
// Revision 1.31  2006/11/16 13:56:45  dkrajzew
// warnings removed
//
// Revision 1.30  2006/11/14 06:48:58  dkrajzew
// readapting changes in the router-API
//
// Revision 1.29  2006/09/18 10:15:17  dkrajzew
// changed vehicle class naming
//
// Revision 1.28  2006/08/01 07:12:11  dkrajzew
// faster access to weight time lines added
//
// Revision 1.27  2006/04/18 08:15:49  dkrajzew
// removal of loops added
//
// Revision 1.26  2006/04/11 11:03:18  dkrajzew
// all structures now return their id via getID()
//
// Revision 1.25  2006/02/13 07:24:11  dkrajzew
// usage of vehicle classes revalidated
//
// Revision 1.24  2006/01/26 08:42:50  dkrajzew
// made lanes and edges being aware to vehicle classes
//
// Revision 1.23  2006/01/16 13:38:33  dkrajzew
// debugging
//
// Revision 1.22  2006/01/16 10:46:24  dkrajzew
// some initial work on  the dfrouter
//
// Revision 1.21  2005/10/17 09:21:36  dkrajzew
// memory leaks removed
//
// Revision 1.20  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.19  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.18  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.17  2005/05/04 08:46:09  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.16  2004/11/23 10:25:51  dkrajzew
// debugging
//
// Revision 1.15  2004/07/02 09:39:41  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived
//  for an online-routing
//
// Revision 1.14  2004/04/23 12:43:00  dkrajzew
// warnings and errors are now reported to MsgHandler, not cerr
//
// Revision 1.13  2004/04/14 13:53:50  roessel
// Changes and additions in order to implement supplementary-weights.
//
// Revision 1.12  2004/03/19 13:03:01  dkrajzew
// some style adaptions
//
// Revision 1.11  2004/03/03 15:33:53  roessel
// Tried to make postloadInit more readable.
// Added an assert to avoid division by zero.
//
// Revision 1.10 2004/01/26 08:01:10 dkrajzew
// loaders and route-def types are now renamed in an senseful way;
// further changes in order to make both new routers work;
// documentation added
//
// Revision 1.9  2003/11/11 08:04:45  dkrajzew
// avoiding emissions of vehicles on too short edges
//
// Revision 1.8  2003/09/17 10:14:27  dkrajzew
// handling of unset values patched
//
// Revision 1.7  2003/06/19 15:22:38  dkrajzew
// inifinite loop on lane searching patched
//
// Revision 1.6 2003/06/18 11:20:54 dkrajzew new message and error
// processing: output to user may be a message, warning or an error
// now; it is reported to a Singleton (MsgHandler); this handler puts
// it further to output instances. changes: no verbose-parameter
// needed; messages are exported to singleton
//
// Revision 1.5  2003/04/14 13:54:20  roessel
// Removed "EdgeType::" in method ROEdge::getNoFollowing().
//
// Revision 1.4  2003/04/09 15:39:10  dkrajzew
// router debugging & extension: no routing over sources, random routes added
//
// Revision 1.3  2003/02/07 10:45:04  dkrajzew
// updated
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include "ROLane.h"
#include "ROEdge.h"
#include "ROVehicle.h"
#include "ROVehicleType.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * static member definitions
 * ======================================================================= */
ROEdge::DictType ROEdge::myDict;
bool myHaveWarned = false; // !!! make this static or so


/* =========================================================================
 * method definitions
 * ======================================================================= */
ROEdge::ROEdge(const std::string &id, int index, bool useBoundariesOnOverride)
    : _id(id), _dist(0), _speed(-1),
      _supplementaryWeightAbsolut(0),
      _supplementaryWeightAdd(0),
      _supplementaryWeightMult(0),
      _usingTimeLine(false),
      myIndex(index), myLength(-1), _hasSupplementaryWeights(false),
      myUseBoundariesOnOverride(useBoundariesOnOverride),
      myHaveBuildShortCut(false), myPackedValueLine(0)
{
}


ROEdge::~ROEdge()
{
    for(std::vector<ROLane*>::iterator i=myLanes.begin(); i!=myLanes.end(); ++i) {
        delete (*i);
    }
    delete _supplementaryWeightAbsolut;
    delete _supplementaryWeightAdd;
    delete _supplementaryWeightMult;
    delete[] myPackedValueLine;
}


size_t
ROEdge::getNumericalID() const
{
    return myIndex;
}


void
ROEdge::addLane(ROLane *lane)
{
    SUMOReal length = lane->getLength();
    assert(myLength==-1||length==myLength);
    myLength = length;
    _dist = length > _dist ? length : _dist;
    SUMOReal speed = lane->getSpeed();
    _speed = speed > _speed ? speed : _speed;
	myDictLane[lane->getID()] = lane;
	myLanes.push_back(lane);

	std::vector<SUMOVehicleClass>::const_iterator i;
	const std::vector<SUMOVehicleClass> &allowed = lane->getAllowedClasses();
	// for allowed classes
	for(i=allowed.begin(); i!=allowed.end(); ++i) {
		SUMOVehicleClass allowedC = *i;
		std::vector<SUMOVehicleClass>::iterator t;
		// add to allowed if not already in there
		t = find(myAllowedClasses.begin(), myAllowedClasses.end(), allowedC);
		if(t==myAllowedClasses.end()) {
			myAllowedClasses.push_back(allowedC);
		}
		// remove from disallowed if allowed on the lane
		t = find(myNotAllowedClasses.begin(), myNotAllowedClasses.end(), allowedC);
		if(t!=myNotAllowedClasses.end()) {
			myNotAllowedClasses.erase(t);
		}
	}
	// for disallowed classes
	const std::vector<SUMOVehicleClass> &disallowed = lane->getNotAllowedClasses();
	for(i=disallowed.begin(); i!=disallowed.end(); ++i) {
		SUMOVehicleClass disallowedC = *i;
		std::vector<SUMOVehicleClass>::iterator t;
		// add to disallowed if not already in there
		//  and not within allowed
		t = find(myAllowedClasses.begin(), myAllowedClasses.end(), disallowedC);
		if(t==myAllowedClasses.end()) {
			t = find(myNotAllowedClasses.begin(), myNotAllowedClasses.end(), disallowedC);
			if(t==myNotAllowedClasses.end()) {
				myNotAllowedClasses.push_back(disallowedC);
			}
		}
	}
}


void
ROEdge::addWeight(SUMOReal value, SUMOTime timeBegin, SUMOTime timeEnd)
{
    _ownValueLine.add(timeBegin, timeEnd, value);
    _usingTimeLine = true;
}


void
ROEdge::addFollower(ROEdge *s)
{
    myFollowingEdges.push_back(s);
}


SUMOReal
ROEdge::getEffort(const ROVehicle *const, SUMOTime time) const
{
    FloatValueTimeLine::SearchResult searchResult;
    FloatValueTimeLine::SearchResult supplementarySearchResult;
    // check whether an absolute value shalle be used
    if (_hasSupplementaryWeights) {
        searchResult = _supplementaryWeightAbsolut->getSearchStateAndValue( time );
        if (searchResult.first) {
            // ok, we have an absolute value for this time step, return it
            return searchResult.second;
        }
    }

    // ok, no absolute value was found, use the normal value (without)
    //  weight as default
    SUMOReal value = (SUMOReal) (_dist / _speed);
    if(_usingTimeLine) {
        if(!myHaveBuildShortCut) {
            myPackedValueLine = _ownValueLine.buildShortCut(myShortCutBegin, myShortCutEnd, myLastPackedIndex, myShortCutInterval);
            myHaveBuildShortCut = true;
        }
        if(myShortCutBegin>time||myShortCutEnd<time) {
            if(myUseBoundariesOnOverride) {
                if(!myHaveWarned) {
                    WRITE_WARNING("No interval matches passed time "+ toString<SUMOTime>(time)  + " in edge '" + _id + "'.\n Using first/last entry.");
                    myHaveWarned = true;
                }
                if(myShortCutBegin>time) {
                    value = myPackedValueLine[0];
                } else {
                    value = myPackedValueLine[myLastPackedIndex];
                }
            } else {
                // value is already set
                //  warn if wished
                if(!myHaveWarned) {
                    WRITE_WARNING("No interval matches passed time "+ toString<SUMOTime>(time)  + " in edge '" + _id + "'.\n Using edge's length / edge's speed.");
                    myHaveWarned = true;
                }
            }
        } else {
            value = myPackedValueLine[(time-myShortCutBegin)/myShortCutInterval];
        }
    }

    // check for additional values
    if ( _hasSupplementaryWeights == true ) {
        // for factors
        supplementarySearchResult = _supplementaryWeightMult->getSearchStateAndValue(time);
        if (supplementarySearchResult.first) {
            value *= supplementarySearchResult.second;
        }
        // for a value to add
        supplementarySearchResult = _supplementaryWeightAdd->getSearchStateAndValue(time);
        if (supplementarySearchResult.first) {
            value += supplementarySearchResult.second;
        }
    }
    // return final value
    return value;
}


size_t
ROEdge::getNoFollowing() const
{
    if(getType()==ET_SINK) {
        return 0;
    }
    return myFollowingEdges.size();
}


ROEdge *
ROEdge::getFollower(size_t pos) const
{
    return myFollowingEdges[pos];
}


bool
ROEdge::isConnectedTo(ROEdge *e)
{
    return find(myFollowingEdges.begin(), myFollowingEdges.end(), e)!=myFollowingEdges.end();
}


SUMOReal
ROEdge::getCost(const ROVehicle *const v, SUMOTime time) const
{
    return getEffort(v, time);
}


SUMOReal
ROEdge::getDuration(const ROVehicle *const v, SUMOTime time) const
{
    return getEffort(v, time);
}


const std::string &
ROEdge::getID() const
{
    return _id;
}


void
ROEdge::setType(ROEdge::EdgeType type)
{
    myType = type;
}


ROEdge::EdgeType
ROEdge::getType() const
{
    return myType;
}


SUMOReal
ROEdge::getLength() const
{
    return myLength;
}

void
ROEdge::setSupplementaryWeights( FloatValueTimeLine* absolut,
                                 FloatValueTimeLine* add,
                                 FloatValueTimeLine* mult )
{
    _supplementaryWeightAbsolut = absolut;
    _supplementaryWeightAdd     = add;
    _supplementaryWeightMult    = mult;
    assert( _supplementaryWeightAbsolut != 0 &&
            _supplementaryWeightAdd     != 0 &&
            _supplementaryWeightMult    != 0 );
    _hasSupplementaryWeights = true;
}


void
ROEdge::clear()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}

bool
ROEdge::dictionary(string id, ROEdge* ptr)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        myDict.insert(DictType::value_type(id, ptr));
        return true;
    }
    return false;
}

ROEdge*
ROEdge::dictionary2(string id)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        return 0;
    }
    return it->second;
}




ROLane *
ROEdge::getLane(std::string name)
{
    //(assert(laneNo<myLanes->size()); ??? was ist assert
    //return *((*myLanes)[laneNo]);
    std::map<std::string, ROLane*>::const_iterator i=
		ROEdge::myDictLane.find(name);
    //assert(i!=myEmitterDict.end());
    return (*i).second;
}


ROLane*
ROEdge::getLane(size_t index)
{
	return myLanes[index];
}


SUMOReal
ROEdge::getSpeed() const
{
    return _speed;
}


int
ROEdge::getLaneNo() const
{
    return myLanes.size();
}


bool
ROEdge::prohibits(const ROVehicle * const vehicle) const
{
	if(myAllowedClasses.size()==0&&myNotAllowedClasses.size()==0) {
		return false;
	}
    // ok, vehicles with an unknown class may be only prohibited
    //  if the edge is limited to a set of classes
	SUMOVehicleClass vclass = vehicle->getType()->getClass();
    if(vclass==SVC_UNKNOWN&&myAllowedClasses.size()==0) {
        return false;
    }
	// check whether it is explicitely disallowed
	if(find(myNotAllowedClasses.begin(), myNotAllowedClasses.end(), vclass)!=myNotAllowedClasses.end()) {
		return true;
	}
	// check whether it is within the allowed classes
	if(myAllowedClasses.size()==0||find(myAllowedClasses.begin(), myAllowedClasses.end(), vclass)!=myAllowedClasses.end()) {
		return false;
	}
	// ok, we have a set of allowed vehicle classes, but this vehicle's class
	//  is not among them
	return true;
}


void
ROEdge::setNodes(RONode *from, RONode *to)
{
    myFromNode = from;
    myToNode = to;
}


RONode *
ROEdge::getFromNode() const
{
    return myFromNode;
}


RONode *
ROEdge::getToNode() const
{
    return myToNode;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


