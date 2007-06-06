/****************************************************************************/
/// @file    ROAbstractRouteDefLoader.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The basic class for loading routes
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

#include <string>
#include "ROAbstractRouteDefLoader.h"
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/options/OptionsCont.h>
#include "RONet.h"

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
ROAbstractRouteDefLoader::ROAbstractRouteDefLoader(ROVehicleBuilder &vb,
        RONet &net,
        SUMOTime begin,
        SUMOTime end,
        const std::string &/*file*/)
        : _net(net), myBegin(begin), myEnd(end), myVehicleBuilder(vb)
{}


ROAbstractRouteDefLoader::~ROAbstractRouteDefLoader()
{}


void
ROAbstractRouteDefLoader::skipUntilBegin()
{
    myReadRoutesAtLeastUntil(myBegin);
}


void
ROAbstractRouteDefLoader::readRoutesAtLeastUntil(SUMOTime time)
{
    if (!myReadRoutesAtLeastUntil(time)) {
        throw ProcessError("Problems on parsing " + getDataName() + " file.");
    }
}



/****************************************************************************/

