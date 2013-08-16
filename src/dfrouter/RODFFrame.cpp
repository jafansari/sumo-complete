/****************************************************************************/
/// @file    RODFFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// Sets and checks options for df-routing
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo-sim.org/
// Copyright (C) 2001-2013 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
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
#include <fstream>
#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/common/SystemFrame.h>
#include "RODFFrame.h"
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>


#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
void
RODFFrame::fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "run routing with options from file");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Defaults");
    oc.addOptionSubTopic("Time");
    SystemFrame::addReportOptions(oc); // fill this subtopic, too


    // register the options
    // register input-options
    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");
    oc.addDescription("net-file", "Input", "Loads the SUMO-network FILE");

    /* never used
    oc.doRegister("route-files", 'r', new Option_FileName());
    oc.addDescription("route-files", "Input", "Loads SUMO-routes from FILE");
    */

    oc.doRegister("detector-files", 'd', new Option_FileName());
    oc.addSynonyme("detector-files", "detectors");
    oc.addDescription("detector-files", "Input", "Loads detector descriptions from FILE");

    oc.doRegister("measure-files", 'f', new Option_FileName());
    oc.addSynonyme("measure-files", "detflows");
    oc.addSynonyme("measure-files", "detector-flow-files", true);
    oc.addDescription("measure-files", "Input", "Loads detector flows from FILE(s)");


    // register output options
    oc.doRegister("routes-output", 'o', new Option_FileName());
    oc.addDescription("routes-output", "Output", "Saves computed routes to FILE");

    oc.doRegister("routes-for-all", new Option_Bool(false));
    oc.addDescription("routes-for-all", "Output", "Forces DFROUTER to compute routes for in-between detectors");

    oc.doRegister("detector-output", new Option_FileName());
    oc.addSynonyme("detector-output", "detectors-output", true);
    oc.addDescription("detector-output", "Output", "Saves typed detectors to FILE");

    oc.doRegister("detectors-poi-output", new Option_FileName());
    oc.addDescription("detectors-poi-output", "Output", "Saves detector positions as pois to FILE");

    oc.doRegister("emitters-output", new Option_FileName());
    oc.addDescription("emitters-output", "Output", "Saves emitter definitions for source detectors to FILE");

    oc.doRegister("emitters-poi-output", new Option_FileName()); // !!! describe
    oc.addDescription("emitters-poi-output", "Output", "Saves emitter positions as pois to FILE");

    oc.doRegister("variable-speed-sign-output", new Option_FileName());
    oc.addSynonyme("variable-speed-sign-output", "speed-trigger-output", true);
    oc.addDescription("variable-speed-sign-output", "Output", "Saves variable seed sign definitions for sink detectors to FILE");

    oc.doRegister("end-reroute-output", new Option_FileName());
    oc.addDescription("end-reroute-output", "Output", "Saves rerouter definitions for sink detectors to FILE");

    oc.doRegister("validation-output", new Option_FileName());
    oc.addDescription("validation-output", "Output", "");

    oc.doRegister("validation-output.add-sources", new Option_Bool(false));
    oc.addDescription("validation-output.add-sources", "Output", "");


    // register processing options
    // to guess empty flows
    oc.doRegister("guess-empty-flows", new Option_Bool(false)); // !!! describe
    oc.addDescription("guess-empty-flows", "Processing", "");

    // for guessing source/sink detectors
    oc.doRegister("highway-mode", 'h', new Option_Bool(false)); // !!! describe
    oc.addDescription("highway-mode", "Processing", "Switches to highway-mode");

    // for detector reading
    oc.doRegister("ignore-invalid-detectors", new Option_Bool(false));
    oc.addDescription("ignore-invalid-detectors", "Processing", "Only warn about unparseable detectors");

    // for detector type computation
    oc.doRegister("revalidate-detectors", new Option_Bool(false));
    oc.addDescription("revalidate-detectors", "Processing", "Recomputes detector types even if given");

    // for route computation
    oc.doRegister("revalidate-routes", new Option_Bool(false));
    oc.addDescription("revalidate-routes", "Processing", "Recomputes routes even if given");

    oc.doRegister("all-end-follower", new Option_Bool(false));
    oc.addDescription("all-end-follower", "Processing", "Continues routes till the first street after a sink");

    oc.doRegister("keep-unfinished-routes", new Option_Bool(false));
    oc.addSynonyme("keep-unfinished-routes", "keep-unfound-ends", true);
    oc.addDescription("keep-unfinished-routes", "Processing", "Keeps routes even if they have exhausted max-search-depth");

    oc.doRegister("keep-longer-routes", new Option_Bool(false));
    oc.addDescription("keep-longer-routes", "Processing", "Keeps routes even if a shorter one exists");

    oc.doRegister("max-search-depth", new Option_Integer(30));
    oc.addSynonyme("max-search-depth", "max-nodet-follower", true);
    oc.addDescription("max-search-depth", "Processing", "Number of edges to follow a route without passing a detector");

    oc.doRegister("emissions-only", new Option_Bool(false));
    oc.addDescription("emissions-only", "Processing", "Writes only emission times");

    oc.doRegister("disallowed-edges", new Option_String(""));
    oc.addDescription("disallowed-edges", "Processing", "Do not route on these edges");

    oc.doRegister("keep-turnarounds", new Option_Bool(false));
    oc.addDescription("keep-turnarounds", "Processing", "Allow turnarounds as route continuations");

    oc.doRegister("min-route-length", new Option_Float(-1));
    oc.addSynonyme("min-route-length", "min-dist", true);
    oc.addSynonyme("min-route-length", "min-distance", true);
    oc.addDescription("min-route-length", "Processing", "Minimum distance in meters between start and end node of every route.");

    // flow reading
    oc.doRegister("time-factor", new Option_String("60", "TIME"));
    oc.addDescription("time-factor", "Processing", "Multiply flow times with TIME to get seconds");

    oc.doRegister("time-offset", new Option_String("0", "TIME"));
    oc.addDescription("time-offset", "Processing", "Subtracts TIME seconds from (scaled) flow times");

    oc.doRegister("time-step", new Option_String("60", "TIME"));
    oc.addDescription("time-step", "Processing", "Expected distance between two successive data sets");

    // saving further structures
    oc.doRegister("calibrator-output", new Option_Bool(false)); // !!!undescribed
    oc.addSynonyme("calibrator-output", "write-calibrators", true);
    oc.addDescription("calibrator-output", "Processing", "Write calibrators to FILE");

    oc.doRegister("include-unused-routes", new Option_Bool(false)); // !!!undescribed
    oc.addDescription("include-unused-routes", "Processing", "");

    //
    oc.doRegister("revalidate-flows", new Option_Bool(false));
    oc.addDescription("revalidate-flows", "Processing", "");

    oc.doRegister("remove-empty-detectors", new Option_Bool(false));
    oc.addDescription("remove-empty-detectors", "Processing", "Removes empty detectors from the list");

    oc.doRegister("strict-sources", new Option_Bool(false)); // !!!undescribed
    oc.addDescription("strict-sources", "Processing", "");

    /* disabled, see ticket #521
    oc.doRegister("join-lanes", new Option_Bool(false));
    oc.addSynonyme("join-lanes", "mesosim", true);
    oc.addDescription("join-lanes", "Processing", "Joins detectors lying on same height");
    */

    //
    oc.doRegister("scale", new Option_Float(1.));
    oc.addDescription("scale", "Processing", "Scale factor for flows");

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


    // register the simulation settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time; Previous defs will be discarded");

    oc.doRegister("end", 'e', new Option_String("86400", "TIME"));
    oc.addDescription("end", "Time", "Defines the end time; Later defs will be discarded; Defaults to one day");


    // register report options
    oc.doRegister("report-empty-detectors", new Option_Bool(false));
    oc.addDescription("report-empty-detectors", "Report", "Lists detectors with no flow (enable -v)");

    oc.doRegister("print-absolute-flows", new Option_Bool(false));
    oc.addDescription("print-absolute-flows", "Report", "Prints aggregated detector flows");

    // register report options
    oc.doRegister("no-step-log", new Option_Bool(false));
    oc.addDescription("no-step-log", "Report", "Disable console output of route parsing step");

    RandHelper::insertRandOptions();
}


bool
RODFFrame::checkOptions() {
    return true;
}



/****************************************************************************/

