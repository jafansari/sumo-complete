/***************************************************************************
                          NamedColumnsParser.cpp
             A parser to retrieve information from a table with known column
             names where the comlumn order may change
                             -------------------
    project              : SUMO
    begin                : Fri, 19 Jul 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.1  2002/10/16 14:59:13  dkrajzew
// initial commit for classes that handle import functions
//
// Revision 1.1  2002/07/25 08:55:42  dkrajzew
// support for Visum7.5 & Cell import added
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#include <map>
#include <string>
#include <utils/common/UtilExceptions.h>
#include "NamedColumnsParser.h"

/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;

/* =========================================================================
 * method definitions
 * ======================================================================= */
NamedColumnsParser::NamedColumnsParser()
{
}

NamedColumnsParser::NamedColumnsParser(const std::string &def,
                                       const std::string &defDelim,
                                       const std::string &lineDelim)
{
    reinitMap(def, defDelim);
    _lineDelim = lineDelim;
}

NamedColumnsParser::~NamedColumnsParser()
{
}

void
NamedColumnsParser::reinit(const std::string &def,
                                const std::string &defDelim,
                                const std::string &lineDelim)
{
    reinitMap(def, defDelim);
    _lineDelim = lineDelim;
}

void
NamedColumnsParser::parseLine(const std::string &line)
{
    _line = StringTokenizer(line, _lineDelim);
}

std::string
NamedColumnsParser::get(const std::string &name)
{
    PosMap::iterator i=_defMap.find(name);
    if(i==_defMap.end()) {
        throw UnknownElement();
    }
    int pos = (*i).second;
    if(_line.size()<=pos) {
        throw OutOfBoundsException();
    }
    return _line.get(pos);
}


void
NamedColumnsParser::reinitMap(const std::string &s, const std::string &delim)
{
    _defMap.clear();
    int pos = 0;
    StringTokenizer st(s, delim);
    while(st.hasNext()) {
        _defMap.insert(map<string, int>::value_type(st.next(), pos++));
    }
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "NamedColumnsParser.icc"
//#endif

// Local Variables:
// mode:C++
// End:

