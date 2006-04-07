//---------------------------------------------------------------------------//
//                        ROJTREdge.cpp -
//  An edge the router may route through
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 20 Jan 2004
//  copyright            : (C) 2004 by Daniel Krajzewicz
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
// Revision 1.3  2006/04/07 10:41:47  dkrajzew
// code beautifying: embedding string in strings removed
//
// Revision 1.2  2006/01/31 11:01:40  dkrajzew
// patching incoherences; added possibility to end on non-sink-edges
//
// Revision 1.1  2005/10/10 12:09:36  dkrajzew
// renamed ROJP*-classes to ROJTR*
//
// Revision 1.9  2005/10/07 11:42:39  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/23 06:04:58  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 12:05:34  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/05/04 08:57:12  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.5  2004/12/16 12:26:52  dkrajzew
// debugging
//
// Revision 1.4  2004/11/23 10:26:59  dkrajzew
// debugging
//
// Revision 1.3  2004/07/02 09:40:36  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived for an online-routing (lane index added)
//
// Revision 1.2  2004/02/16 13:48:07  dkrajzew
// false usage of a random number patched
//
// Revision 1.1  2004/02/06 08:43:46  dkrajzew
// new naming applied to the folders (jp-router is now called jtr-router)
//
// Revision 1.3  2004/02/02 16:21:06  dkrajzew
// new default behaviour on incomplete edge propabilities
//
// Revision 1.2  2004/01/26 09:57:14  dkrajzew
// setting of the default direction percentages corrected
//
// Revision 1.1  2004/01/26 06:09:11  dkrajzew
// initial commit for jp-classes
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

#include <algorithm>
#include <cassert>
#include <utils/common/MsgHandler.h>
#include "ROJTREdge.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
ROJTREdge::ROJTREdge(const std::string &id, int index)
    : ROEdge(id, index)
{
}


ROJTREdge::~ROJTREdge()
{
    for(FollowerUsageCont::iterator i=myFollowingDefs.begin(); i!=myFollowingDefs.end(); i++) {
        delete (*i).second;
    }
}


void
ROJTREdge::addFollower(ROEdge *s)
{
    ROEdge::addFollower(s);
    myFollowingDefs[static_cast<ROJTREdge*>(s)] =
        new FloatValueTimeLine();
}


void
ROJTREdge::addFollowerProbability(ROJTREdge *follower, SUMOTime begTime,
                                 SUMOTime endTime, SUMOReal probability)
{
    FollowerUsageCont::iterator i = myFollowingDefs.find(follower);
    if(i==myFollowingDefs.end()) {
        MsgHandler::getErrorInstance()->inform("The edges '" + getID() + "' and '" + follower->getID() + "' are not connected.");
        return;
    }
	(*i).second->add(begTime, endTime, probability);
}


ROJTREdge *
ROJTREdge::chooseNext(SUMOTime time) const
{
    if(myFollowingEdges.size()==0) {
        return 0;
    }
    // check whether any definition exists
    FollowerUsageCont::const_iterator i;
    // check how many edges are defined for the given time step
    size_t noDescs = 0;
    bool hasDescription = true;
    for(i=myFollowingDefs.begin(); i!=myFollowingDefs.end()&&hasDescription; i++) {
        if((*i).second->describesTime(time)) {
            noDescs++;
        }
    }
    // get a random number between zero and one
    SUMOReal chosen = (SUMOReal)rand() /
        ( static_cast<SUMOReal>(RAND_MAX) + 1);
    // if no description is given for the current time
    if(noDescs==0) {
        //  use the defaults
        std::vector<SUMOReal>::const_iterator j;
        size_t pos = 0;
        for(j=myParsedTurnings.begin(); j!=myParsedTurnings.end(); j++) {
            chosen = chosen - (*j);
            if(chosen<0) {
                return static_cast<ROJTREdge*>(myFollowingEdges[pos]);
            }
            pos++;
        }
        return static_cast<ROJTREdge*>(myFollowingEdges[0]);
    }
    // if the probabilities are given for all following edges
    if(noDescs==myFollowingEdges.size()) {
        // use the loaded definition
            // choose the appropriate one from the list
        for(i=myFollowingDefs.begin(); i!=myFollowingDefs.end(); i++) {
            chosen = chosen - (*i).second->getValue(time);
            if(chosen<=0) {
                return (*i).first;
            }
        }
        return (*myFollowingDefs.begin()).first;
    } else {
        // ok, at least one description is missing
        //  the missing descriptions will be treated as zero
        bool allZero = true; // we won't be as stupid as our users may be...
        while(allZero) { // ... we may loop several times, but not forever
            allZero = false;
            for(i=myFollowingDefs.begin(); i!=myFollowingDefs.end(); i++) {
                if((*i).second->describesTime(time)) {
                    chosen = chosen - (*i).second->getValue(time);
                    if(chosen<=0) {
                        return (*i).first;
                    }
                    allZero = allZero |
                        ((*i).second->getValue(time)!=0);
                }
            }
        }
        return (*myFollowingDefs.begin()).first;
    }
}


void
ROJTREdge::setTurnDefaults(const std::vector<SUMOReal> &defs)
{
    // I hope, we'll find a less ridiculous solution for this
    std::vector<SUMOReal> tmp(defs.size()*myFollowingEdges.size(), 0);
        // store in less common multiple
    size_t i;
    for(i=0; i<defs.size(); i++) {
        for(size_t j=0; j<myFollowingEdges.size(); j++) {
            tmp[i*myFollowingEdges.size()+j] = (SUMOReal)
				(defs[i] / 100.0 / (myFollowingEdges.size()));
        }
    }
        // parse from less common multiple
    for(i=0; i<myFollowingEdges.size(); i++) {
        SUMOReal value = 0;
        for(size_t j=0; j<defs.size(); j++) {
            value += tmp[i*defs.size()+j];
        }
        myParsedTurnings.push_back((SUMOReal) value);
    }
}



/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


