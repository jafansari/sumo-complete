/****************************************************************************/
/// @file    ROFrame.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Sets and checks options for routing
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

#include <iostream>
#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <utils/common/SUMOTime.h>
#include "ROFrame.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
void
ROFrame::fillOptions(OptionsCont &oc) {
    // register options
    // register I/O options
    oc.doRegister("output", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "output");
    oc.addDescription("output-file", "Output", "Write generated routes to FILE");

    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");
    oc.addDescription("net-file", "Input", "Use FILE as SUMO-network to route on");

    oc.doRegister("districts", 'd', new Option_FileName());
    oc.addDescription("districts", "Input", "Read (additional) districts from FILE");

    oc.doRegister("alternatives", 'a', new Option_FileName());
    oc.addDescription("alternatives", "Input", "Read alternatives from FILE");

    oc.doRegister("weights", 'w', new Option_FileName());
    oc.addSynonyme("weights", "weight-files");
    oc.addDescription("weights", "Input", "Read network weights from FILE(s)");

    oc.doRegister("lane-weights", new Option_FileName());
    oc.addDescription("lane-weights", "Input", "Read lane-weights from FILE(s)");

    oc.doRegister("measure", 'x', new Option_String("traveltime"));
    oc.addDescription("measure", "Input", "Name of the xml attribute which gives the edge weight");

    // register the time settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time; Previous trips will be discarded");

    oc.doRegister("end", 'e', new Option_String(SUMOTIME_MAXSTRING, "TIME"));
    oc.addDescription("end", "Time", "Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent");


    // register the processing options
    oc.doRegister("continue-on-unbuild", new Option_Bool(false));
    oc.addDescription("continue-on-unbuild", "Processing", "Continue if a route could not be build");

    oc.doRegister("unsorted", new Option_Bool(false));
    oc.addDescription("unsorted", "Processing", "Assume input is unsorted");

    oc.doRegister("randomize-flows", new Option_Bool(false));
    oc.addDescription("randomize-flows", "Processing", "generate random departure times for flow input");

    oc.doRegister("max-alternatives", new Option_Integer(5));
    oc.addDescription("max-alternatives", "Processing", "Prune the number of alternatives to INT");

    oc.doRegister("remove-loops", new Option_Bool(false));
    oc.addDescription("remove-loops", "Processing", "Remove loops within the route; Remove turnarounds at start and end of the route");

    oc.doRegister("repair", new Option_Bool(false));
    oc.addDescription("repair", "Processing", "Tries to correct a false route");

    oc.doRegister("interpolate", new Option_Bool(false));
    oc.addDescription("interpolate", "Processing", "Interpolate edge weights at interval boundaries");

    oc.doRegister("with-taz", new Option_Bool(false));
    oc.addDescription("with-taz", "Processing", "Use origin and destination zones (districts) for in- and output");


    // register defaults options
    oc.doRegister("departlane", new Option_String());
    oc.addDescription("departlane", "Defaults", "Assigns a default depart lane");

    oc.doRegister("departpos", new Option_String());
    oc.addDescription("departpos", "Defaults", "Assigns a default depart position");

    oc.doRegister("departspeed", new Option_String());
    oc.addDescription("departspeed", "Defaults", "Assigns a default depart speed");

    oc.doRegister("arrivallane", new Option_String());
    oc.addDescription("arrivallane", "Defaults", "Assigns a default arrival lane");

    oc.doRegister("arrivalpos", new Option_String());
    oc.addDescription("arrivalpos", "Defaults", "Assigns a default arrival position");

    oc.doRegister("arrivalspeed", new Option_String());
    oc.addDescription("arrivalspeed", "Defaults", "Assigns a default arrival speed");

    oc.doRegister("defaults-override", new Option_Bool(false));
    oc.addDescription("defaults-override", "Defaults", "Defaults will override given values");


    // register report options
    oc.doRegister("stats-period", new Option_Integer(-1));
    oc.addDescription("stats-period", "Report", "Defines how often statistics shall be printed");
}


bool
ROFrame::checkOptions(OptionsCont &oc) {
    // check whether the output is valid and can be build
    if (!oc.isSet("output")) {
        MsgHandler::getErrorInstance()->inform("No output specified.");
        return false;
    }
    //
    if (oc.getInt("max-alternatives")<2) {
        MsgHandler::getErrorInstance()->inform("At least two alternatives should be enabled");
        return false;
    }
    // check departure/arrival options
    return true;
}



/****************************************************************************/

