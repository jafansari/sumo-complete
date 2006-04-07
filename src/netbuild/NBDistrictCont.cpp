/***************************************************************************
                          NBDistrictCont.cpp
              A container for all of the nets districts
                             -------------------
    project              : SUMO
    subproject           : netbuilder / netconverter
    begin                : Tue, 20 Nov 2001
    copyright            : (C) 2001 by DLR http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.12  2006/04/07 10:41:47  dkrajzew
// code beautifying: embedding string in strings removed
//
// Revision 1.11  2005/10/17 09:02:44  dkrajzew
// got rid of the old MSVC memory leak checker; memory leaks removed
//
// Revision 1.10  2005/10/07 11:38:18  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.9  2005/09/23 06:01:05  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.8  2005/09/15 12:02:45  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.7  2005/04/27 11:48:25  dkrajzew
// level3 warnings removed; made containers non-static
//
// Revision 1.6  2004/11/23 10:21:40  dkrajzew
// debugging
//
// Revision 1.5  2003/06/18 11:13:13  dkrajzew
// new message and error processing: output to user may be a message,
//  warning or an error now; it is reported to a Singleton (MsgHandler);
//  this handler puts it further to output instances.
//  changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.4  2003/03/17 14:22:33  dkrajzew
// further debug and windows eol removed
//
// Revision 1.3  2003/03/03 14:58:59  dkrajzew
// debugging; handling of imported traffic light definitions
//
// Revision 1.2  2003/02/07 10:43:43  dkrajzew
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

#include <string>
#include <iostream>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "NBDistrict.h"
#include "NBDistrictCont.h"

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
NBDistrictCont::NBDistrictCont()
{
}


NBDistrictCont::~NBDistrictCont()
{
    clear();
}


bool
NBDistrictCont::insert(NBDistrict *district)
{
    DistrictCont::iterator i = _districts.find(district->getID());
    if(i!=_districts.end()) return false;
    _districts.insert(DistrictCont::value_type(district->getID(), district));
    return true;
}


NBDistrict *
NBDistrictCont::retrieve(const string &id)
{
    DistrictCont::iterator i = _districts.find(id);
    if(i==_districts.end()) return 0;
    return (*i).second;
}


void
NBDistrictCont::writeXML(ostream &into)
{
    for(DistrictCont::iterator i=_districts.begin(); i!=_districts.end(); i++) {
        (*i).second->writeXML(into);
    }
    into << endl;
}


int NBDistrictCont::size()
{
    return _districts.size();
}


void
NBDistrictCont::clear()
{
    for(DistrictCont::iterator i=_districts.begin(); i!=_districts.end(); i++) {
        delete((*i).second);
    }
    _districts.clear();
}


void
NBDistrictCont::report()
{
    WRITE_MESSAGE("   " + toString<int>(size()) + " districts loaded.");
}


bool
NBDistrictCont::addSource(const std::string &dist, NBEdge *source,
                          SUMOReal weight)
{
    NBDistrict *o = retrieve(dist);
    if(o==0) {
        return false;
    }
    return o->addSource(source, weight);
}


bool
NBDistrictCont::addSink(const std::string &dist, NBEdge *destination,
                        SUMOReal weight)
{
    NBDistrict *o = retrieve(dist);
    if(o==0) {
        return false;
    }
    return o->addSink(destination, weight);
}


void
NBDistrictCont::removeFromSinksAndSources(NBEdge *e)
{
    for(DistrictCont::iterator i=_districts.begin(); i!=_districts.end(); i++) {
        (*i).second->removeFromSinksAndSources(e);
    }
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

