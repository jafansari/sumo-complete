/****************************************************************************/
/// @file    ROVehicle.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A vehicle as used by router
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

#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <string>
#include <iostream>
#include "ROVehicleType.h"
#include "RORouteDef.h"
#include "ROVehicle.h"
#include "RORouteDef_Alternatives.h"
#include "RORoute.h"
#include "ROHelper.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
ROVehicle::ROVehicle(const SUMOVehicleParameter &pars,
                     RORouteDef *route, ROVehicleType *type) throw()
        : myParameter(pars), myType(type), myRoute(route) {}


ROVehicle::~ROVehicle() throw() {}


void
ROVehicle::saveAllAsXML(SUMOAbstractRouter<ROEdge,ROVehicle> &router, OutputDevice &os,
                        OutputDevice * const altos, bool withExitTimes) const throw(IOError) {
    // check whether the vehicle's type was saved before
    if (myType!=0&&!myType->isSaved()) {
        // ... save if not
        myType->writeXMLDefinition(os);
        if (altos!=0) {
            myType->writeXMLDefinition(*altos);
        }
        myType->markSaved();
    }

    // write the vehicle (new style, with included routes)
    os << "   ";
    myParameter.writeAs("vehicle", os, OptionsCont::getOptions());
    if (altos!=0) {
        (*altos) << "   ";
        myParameter.writeAs("vehicle", *altos, OptionsCont::getOptions());
    }

    // check whether the route shall be saved
    if (!myRoute->isSaved()) {
        os << "      ";
        myRoute->writeXMLDefinition(router, os, this, false, withExitTimes);
        if (altos!=0) {
            (*altos) << "      ";
            myRoute->writeXMLDefinition(router, *altos, this, true, withExitTimes);
        }
    }
    os << "   </vehicle>\n";
    if (altos!=0) {
        (*altos) << "   </vehicle>\n";
    }
}


ROVehicle *
ROVehicle::copy(const std::string &id, unsigned int depTime,
                RORouteDef *newRoute) throw() {
    SUMOVehicleParameter pars(myParameter);
    pars.id = id;
    pars.depart = depTime;
    return new ROVehicle(pars, newRoute, myType);
}


/****************************************************************************/

