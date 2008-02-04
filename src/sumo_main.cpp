/****************************************************************************/
/// @file    sumo_main.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Main for SUMO
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

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <ctime>
#include <string>
#include <iostream>
#include <microsim/MSNet.h>
#include <microsim/MSRoute.h>
#include <microsim/MSEmitControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/trigger/MSTriggerControl.h>
#include <netload/NLBuilder.h>
#include <netload/NLHandler.h>
#include <netload/NLTriggerBuilder.h>
#include <netload/NLEdgeControlBuilder.h>
#include <netload/NLJunctionControlBuilder.h>
#include <netload/NLDetectorBuilder.h>
#include <netload/NLGeomShapeBuilder.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <microsim/MSFrame.h>
#include <microsim/output/MSDetectorControl.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef TRACI
#include <traci-server/TraCIServer.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif

// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// functions
// ===========================================================================
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
/**
 * loads the net, additional routes and the detectors
 */
MSNet *
load(OptionsCont &oc)
{
    MSFrame::setMSGlobals(oc);
    MSNet *net = new MSNet(new MSVehicleControl(), new MSEventControl(),
        new MSEventControl(), new MSEventControl());
    NLEdgeControlBuilder eb;
    NLJunctionControlBuilder jb(*net, oc);
    NLDetectorBuilder db(*net);
    NLTriggerBuilder tb;
    NLGeomShapeBuilder sb(*net);
    NLHandler handler("", *net, db, tb, eb, jb, sb, oc.getInt("incremental-dua-base"), oc.getInt("incremental-dua-step"));
    NLBuilder builder(oc, *net, eb, jb, db, tb, sb, handler);
    if (!builder.build()) {
        delete net;
        throw ProcessError();
    }
    return net;
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    OptionsCont &oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("A microscopic road traffic simulation.");
#ifdef WIN32
    oc.setApplicationName("sumo.exe", "SUMO sumo Version " + (string)VERSION_STRING);
#else
    oc.setApplicationName("sumo", "SUMO sumo Version " + (string)VERSION_STRING);
#endif
    int ret = 0;
    MSNet *net = 0;
    try {
        // initialise subsystems
        XMLSubSys::init();
        MSFrame::fillOptions();
        OptionsIO::getOptions(true, argc, argv);
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        MsgHandler::initOutputOptions();
        if (!MSFrame::checkOptions()) throw ProcessError();
        RandHelper::initRandGlobal();
        // load the net
        net = load(oc);
        if (net!=0) {
#ifdef TRACI
            // If a remote-port is given, the simulation waits for being controlled from the outside ...
            if (oc.getInt("remote-port") != 0) {
                WRITE_MESSAGE("waiting for request on port " + toString<int>(oc.getInt("remote-port")));
                traci::TraCIServer rs;
                WRITE_MESSAGE("Simulation started with time offset: " + toString<int>(oc.getInt("begin")));
                rs.run();
                WRITE_MESSAGE("Simulation ended at time: " + toString<int>(net->getCurrentTimeStep()));
            } else
                // ... just run as always
#endif
            {
                // report the begin when wished
                WRITE_MESSAGE("Simulation started with time: " + toString<int>(oc.getInt("begin")));
                // simulate
                net->simulate(oc.getInt("begin"), oc.getInt("end"));
                // report the end when wished
                WRITE_MESSAGE("Simulation ended at time: " + toString<int>(net->getCurrentTimeStep()));
            }
        }
    } catch (ProcessError &e) {
        if (string(e.what())!=string("Process Error") && string(e.what())!=string("")) {
            MsgHandler::getErrorInstance()->inform(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on unknown error).", false);
        ret = 1;
#endif
    }
    delete net;
    OutputDevice::closeAll();
    SystemFrame::close();
    return ret;
}



/****************************************************************************/

