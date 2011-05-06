/****************************************************************************/
/// @file    NBJunctionLogicCont.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Class for the io-ing between junctions (nodes) and the computers
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include "NBJunctionLogicCont.h"
#include <utils/common/FileHelpers.h>
#include <utils/common/UtilExceptions.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
NBJunctionLogicCont::NBJunctionLogicCont() {}


NBJunctionLogicCont::~NBJunctionLogicCont() {
    clear();
}


bool
NBJunctionLogicCont::exists(const std::string &key) {
    return myMap.find(key)!=myMap.end();
}


void
NBJunctionLogicCont::add(const std::string &key,
                         const std::string &xmlDescription) {
    myMap[key] = xmlDescription;
}


void
NBJunctionLogicCont::writeXML(OutputDevice &into) {
    for (LogicMap::iterator i=myMap.begin(); i!=myMap.end(); i++) {
        into << (*i).second;
    }
    if (myMap.size()!=0) {
        into << "\n";
    }
}


void
NBJunctionLogicCont::clear() {
    myMap.clear();
}



/****************************************************************************/

