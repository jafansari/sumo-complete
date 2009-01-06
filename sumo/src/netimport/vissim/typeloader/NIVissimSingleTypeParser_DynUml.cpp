/****************************************************************************/
/// @file    NIVissimSingleTypeParser_DynUml.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 18 Dec 2002
/// @version $Id$
///
//
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
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
#include <vector>
#include <utils/common/TplConvert.h>
#include "../NIVissimLoader.h"
#include "NIVissimSingleTypeParser_DynUml.h"

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
NIVissimSingleTypeParser_DynUml::NIVissimSingleTypeParser_DynUml(NIVissimLoader &parent)
        : NIVissimLoader::VissimSingleTypeParser(parent)
{}


NIVissimSingleTypeParser_DynUml::~NIVissimSingleTypeParser_DynUml()
{}


bool
NIVissimSingleTypeParser_DynUml::parse(std::istream &from)
{
    std::vector<std::string> tmp;
    tmp.push_back("reisezeit");
    tmp.push_back("kante");
    readUntil(from, "kirchhoffexponent");
    string tag = readEndSecure(from, tmp);
    while (tag!="DATAEND") {
        tag = readEndSecure(from, tmp);
    }
    return true;
}



/****************************************************************************/

