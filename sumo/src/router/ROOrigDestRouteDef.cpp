//---------------------------------------------------------------------------//
//                        ROOrigDestRouteDef.cpp -
//  A route where only the origin and the destination edges are known
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
// Revision 1.7  2003/06/18 11:36:50  dkrajzew
// a new interface which allows to choose whether to stop after a route could not be computed or not; not very sphisticated, in fact
//
// Revision 1.6  2003/04/10 15:47:01  dkrajzew
// random routes are now being prunned to avoid some stress with turning vehicles
//
// Revision 1.5  2003/04/09 15:39:11  dkrajzew
// router debugging & extension: no routing over sources, random routes added
//
// Revision 1.4  2003/03/20 16:39:16  dkrajzew
// periodical car emission implemented; windows eol removed
//
// Revision 1.3  2003/02/07 10:45:04  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <string>
#include <iostream>
#include <cassert>
#include "ROEdge.h"
#include "RORouteDef.h"
#include "RORoute.h"
#include "ROOrigDestRouteDef.h"
#include "RORouter.h"

using namespace std;

ROOrigDestRouteDef::ROOrigDestRouteDef(const std::string &id,
                                       ROEdge *from, ROEdge *to,
                                       bool removeFirst)
    : RORouteDef(id), _from(from), _to(to), _current(0),
    myRemoveFirst(removeFirst)
{
    assert(_from!=0);
    assert(_to!=0);
}

ROOrigDestRouteDef::~ROOrigDestRouteDef()
{
    delete _current;
}

ROEdge *
ROOrigDestRouteDef::getFrom() const
{
    return _from;
}


ROEdge *
ROOrigDestRouteDef::getTo() const
{
    return _to;
}


RORoute *
ROOrigDestRouteDef::buildCurrentRoute(RORouter &router, long begin,
                                      bool continueOnUnbuild)
{
    ROEdgeVector rv = router.compute(_from, _to, begin, continueOnUnbuild);
    if(myRemoveFirst&&rv.size()>1) {
        rv.removeEnds();
    }
    return new RORoute(_id, 0, 1, rv);
}


void
ROOrigDestRouteDef::addAlternative(RORoute *current, long begin)
{
    _current = current;
    _startTime = begin;
}


void
ROOrigDestRouteDef::xmlOutCurrent(std::ostream &res, bool isPeriodical) const
{
    _current->xmlOut(res, isPeriodical);
}


void
ROOrigDestRouteDef::xmlOutAlternatives(std::ostream &altres) const
{
    altres << "   <routealt id=\"" << _current->getID()
        << "\" last=\"0\">" << endl;
    altres << "      <route cost=\"" << _current->recomputeCosts(_startTime)
        << "\" propability=\"1\">";
    _current->xmlOutEdges(altres);
    altres << "</route>" << endl;
    altres << "   </routealt>" << endl;
}



/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "ROOrigDestRouteDef.icc"
//#endif

// Local Variables:
// mode:C++
// End:


