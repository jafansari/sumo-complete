/****************************************************************************/
/// @file    GUILoadThread.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Class describing the thread that performs the loading of a simulation
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
#include <guisim/GUINet.h>
#include <guisim/GUIEventControl.h>
#include <netload/NLBuilder.h>
#include <guinetload/GUIEdgeControlBuilder.h>
#include <guinetload/GUIJunctionControlBuilder.h>
#include <guinetload/GUIDetectorBuilder.h>
#include <guinetload/GUITriggerBuilder.h>
#include <guinetload/GUIGeomShapeBuilder.h>
#include <guinetload/GUIHandler.h>
#include <guisim/GUIVehicleControl.h>
#include <microsim/output/MSDetectorControl.h>
#include <utils/common/UtilExceptions.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/foxtools/MFXEventQue.h>
#include <microsim/MSFrame.h>
#include <utils/common/MsgRetrievingFunction.h>
#include "GUIApplicationWindow.h"
#include "GUILoadThread.h"
#include "GUIGlobals.h"
#include "GUIEvent_SimulationLoaded.h"
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/events/GUIEvent_SimulationEnded.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include <utils/gui/windows/GUIAppGlobals.h>
#include <utils/common/RandHelper.h>
#include <ctime>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;
using namespace FXEX;


// ===========================================================================
// member method definitions
// ===========================================================================
GUILoadThread::GUILoadThread(MFXInterThreadEventClient *mw,
                             MFXEventQue &eq, FXEX::FXThreadEvent &ev)
        : GUIAbstractLoadThread(mw, eq, ev)
{}


GUILoadThread::~GUILoadThread()
{}


FXint
GUILoadThread::run()
{
    GUINet *net = 0;
    int simStartTime = 0;
    int simEndTime = 0;

    // remove old options
    OptionsCont::getOptions().clear();
    // within gui-based applications, nothing is reported to the console
    MsgHandler::getErrorInstance()->report2cout(false);
    MsgHandler::getErrorInstance()->report2cerr(false);
    MsgHandler::getWarningInstance()->report2cout(false);
    MsgHandler::getWarningInstance()->report2cerr(false);
    MsgHandler::getMessageInstance()->report2cout(false);
    MsgHandler::getMessageInstance()->report2cerr(false);
    // register message callbacks
    MsgHandler::getMessageInstance()->addRetriever(myMessageRetriever);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->addRetriever(myWarningRetreiver);

    // try to load the given configuration
    if (!initOptions()) {
        // the options are not valid
        submitEndAndCleanup(net, simStartTime, simEndTime);
        return 0;
    }
    MsgHandler::initOutputOptions(true);
    if (!MSFrame::checkOptions()) {
        // the options are not valid
        submitEndAndCleanup(net, simStartTime, simEndTime);
        return 0;
    }



    RandHelper::initRandGlobal();
    // try to load
    OptionsCont &oc = OptionsCont::getOptions();
    MSFrame::setMSGlobals(oc);
    net = new GUINet(buildVehicleControl(), new GUIEventControl(),
                     new GUIEventControl(), new GUIEventControl());
    GUIEdgeControlBuilder *eb = buildEdgeBuilder();
    GUIJunctionControlBuilder jb(*net, oc);
    GUIDetectorBuilder db(*net);
    GUIGeomShapeBuilder sb(*net, gIDStorage);
    GUITriggerBuilder tb;
    GUIHandler handler("", *net, db, tb, *eb, jb, sb);
    NLBuilder builder(oc, *net, *eb, jb, db, tb, sb, handler);
    try {
        MsgHandler::getErrorInstance()->clear();
        MsgHandler::getWarningInstance()->clear();
        MsgHandler::getMessageInstance()->clear();
        if (!builder.build()) {
            delete net;
            net = 0;
        } else {
            net->initGUIStructures();
            simStartTime = oc.getInt("begin");
            simEndTime = oc.getInt("end");
        }
    } catch (ProcessError &e) {
        if (string(e.what())!=string("Process Error") && string(e.what())!=string("")) {
            MsgHandler::getErrorInstance()->inform(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        delete net;
        MSNet::clearAll();
        net = 0;
        MsgHandler::cleanupOnEnd();
#ifndef _DEBUG
    } catch (exception &e) {
        MsgHandler::getErrorInstance()->inform(e.what());
        delete net;
        MSNet::clearAll();
        net = 0;
        MsgHandler::cleanupOnEnd();
#endif
    }
    if (net==0) {
        MSNet::clearAll();
    }
    delete eb;
    submitEndAndCleanup(net, simStartTime, simEndTime);
    return 0;
}



GUIEdgeControlBuilder *
GUILoadThread::buildEdgeBuilder()
{
    return new GUIEdgeControlBuilder(gIDStorage);
}




GUIVehicleControl*
GUILoadThread::buildVehicleControl()
{
    return new GUIVehicleControl();
}


void
GUILoadThread::submitEndAndCleanup(GUINet *net,
                                   SUMOTime simStartTime,
                                   SUMOTime simEndTime)
{
    // remove message callbacks
    MsgHandler::getErrorInstance()->removeRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->removeRetriever(myWarningRetreiver);
    MsgHandler::getMessageInstance()->removeRetriever(myMessageRetriever);
    // inform parent about the process
    GUIEvent *e = new GUIEvent_SimulationLoaded(net, simStartTime, simEndTime, myFile);
    myEventQue.add(e);
    myEventThrow.signal();
}


bool
GUILoadThread::initOptions()
{
    try {
        OptionsCont &oc = OptionsCont::getOptions();
        oc.clear();
        MSFrame::fillOptions();
        if (myLoadNet) {
            oc.set("net-file", myFile);
        } else {
            oc.set("configuration-file", myFile);
        }
        OptionsIO::getOptions(true, 1, 0);
        return true;
    } catch (ProcessError &e) {
        if (string(e.what())!=string("Process Error") && string(e.what())!=string("")) {
            MsgHandler::getErrorInstance()->inform(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
    }
    return false;
}



/****************************************************************************/

