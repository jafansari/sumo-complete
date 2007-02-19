/****************************************************************************/
/// @file    NIVissimSingleTypeParser_Zuflussdefinition.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 18 Dec 2002
/// @version $Id$
///
//
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

#include <iostream>
#include <utils/common/TplConvert.h>
#include "../NIVissimLoader.h"
#include "../tempstructs/NIVissimSource.h"
#include "NIVissimSingleTypeParser_Zuflussdefinition.h"

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
NIVissimSingleTypeParser_Zuflussdefinition::NIVissimSingleTypeParser_Zuflussdefinition(NIVissimLoader &parent)
        : NIVissimLoader::VissimSingleTypeParser(parent)
{}


NIVissimSingleTypeParser_Zuflussdefinition::~NIVissimSingleTypeParser_Zuflussdefinition()
{}


bool
NIVissimSingleTypeParser_Zuflussdefinition::parse(std::istream &from)
{
    string id, edgeid;
    from >> id; // type-checking is missing!
    string tag, name;
    // override some optional values till q
    while (tag!="q") {
        tag = overrideOptionalLabel(from);
        if (tag=="name") {
            name = readName(from);
        } else if (tag=="strecke") {
            from >> edgeid; // type-checking is missing!
        }
    }
    // read q
    bool exact = false;
    SUMOReal q;
    tag = myRead(from);
    if (tag=="exakt") {
        exact = true;
        tag = myRead(from);
    }
    q = TplConvert<char>::_2SUMOReal(tag.c_str());
    // read the vehicle types
    from >> tag;
    int vehicle_combination;
    from >> vehicle_combination;
    // check whether optional time information is available
    tag = readEndSecure(from);
    SUMOReal beg, end;
    beg = -1;
    end = -1;
    if (tag=="zeit") {
        from >> tag;
        from >> beg;
        from >> tag;
        from >> end;
    }
    return NIVissimSource::dictionary(id, name, edgeid, q, exact, vehicle_combination,
                                      beg, end);
}



/****************************************************************************/

