/****************************************************************************/
/// @file    RORouteDef.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Basic class for route definitions (not the computed routes)
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
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/Named.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include "ROEdge.h"
#include "RORoute.h"
#include "ROAbstractRouter.h"
#include "ReferencedItem.h"
#include "RORouteDef.h"
#include "ROVehicle.h"
#include "ROVehicleType.h"

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
RORouteDef::RORouteDef(const std::string &id, const RGBColor &color)
        : ReferencedItem(), Named(StringUtils::convertUmlaute(id)),
        myColor(color)
{}


RORouteDef::~RORouteDef()
{}


void
RORouteDef::patchID()
{
    // patch the name
    size_t idx = _id.rfind('_');
    if (idx!=string::npos) {
        try {
            int no = TplConvert<char>::_2int(_id.substr(idx+1).c_str());
            _id = _id.substr(0, idx+1) + toString<int>(no+1);
        } catch (NumberFormatException &) {
            _id = _id + "_0";
        }
    } else {
        _id = _id + "_0";
    }
}


int
RORouteDef::getLastUsedIndex() const
{
    return 0;
}


size_t
RORouteDef::getAlternativesSize() const
{
    return 1;
}


const RGBColor &
RORouteDef::getColor() const
{
    return myColor;
}



/****************************************************************************/

