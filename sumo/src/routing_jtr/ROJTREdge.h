#ifndef ROJTREdge_h
#define ROJTREdge_h
//---------------------------------------------------------------------------//
//                        ROJTREdge.h -
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
// $Log$
// Revision 1.2  2006/01/31 11:01:40  dkrajzew
// patching incoherences; added possibility to end on non-sink-edges
//
// Revision 1.1  2005/10/10 12:09:36  dkrajzew
// renamed ROJP*-classes to ROJTR*
//
// Revision 1.7  2005/10/07 11:42:39  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/23 06:04:58  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 12:05:34  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/05/04 08:57:12  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.3  2004/12/16 12:26:52  dkrajzew
// debugging
//
// Revision 1.2  2004/07/02 09:40:36  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived for an online-routing (lane index added)
//
// Revision 1.1  2004/02/06 08:43:46  dkrajzew
// new naming applied to the folders (jp-router is now called jtr-router)
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

#include <string>
#include <map>
#include <vector>
#include <utils/router/FloatValueTimeLine.h>
#include <router/ROEdge.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class ROLane;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class ROJTREdge
 * A router's edge extended by the definition about the probability a
 *  vehicle's probabilities to choose a certain following edge over time.
 */
class ROJTREdge : public ROEdge {
public:
    /// Constructor
    ROJTREdge(const std::string &id, int index);

    /// Desturctor
    ~ROJTREdge();

    /// Adds information about a connected edge
    void addFollower(ROEdge *s);

    /// adds the information about the percentage of using a certain follower
    void addFollowerProbability(ROJTREdge *follower,
        SUMOTime begTime, SUMOTime endTime, SUMOReal probability);

    /// Returns the next edge to use
    ROJTREdge *chooseNext(SUMOTime time) const;

    /// Sets the turning definition defaults
    void setTurnDefaults(const std::vector<SUMOReal> &defs);

private:
    /// Definition of a map that stores the probabilities of using a certain follower over time
    typedef std::map<ROJTREdge*, FloatValueTimeLine*> FollowerUsageCont;

    /// Storage for the probabilities of using a certain follower over time
    FollowerUsageCont myFollowingDefs;

    /// The defaults for turnings
    std::vector<SUMOReal> myParsedTurnings;

private:
    /// we made the copy constructor invalid
    ROJTREdge(const ROJTREdge &src);

    /// we made the assignment operator invalid
    ROJTREdge &operator=(const ROJTREdge &src);

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

