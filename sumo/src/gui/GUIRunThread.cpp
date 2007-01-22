//---------------------------------------------------------------------------//
//                        GUIRunThread.cpp -
//  The thread that runs the simulation
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
        "$Id$";
}
// $Log$
// Revision 1.38  2006/12/04 08:00:22  dkrajzew
// removed some warnings
//
// Revision 1.37  2006/11/16 12:30:53  dkrajzew
// warnings removed
//
// Revision 1.36  2006/09/18 09:57:43  dkrajzew
// removed deprecated c2c functions, added new made by Danilot Boyom
//
// Revision 1.35  2006/01/09 11:50:41  dkrajzew
// warnings removed
//
// Revision 1.34  2005/12/01 07:32:03  dkrajzew
// final try/catch is now only used in the release version
//
// Revision 1.33  2005/11/29 13:22:20  dkrajzew
// dded a minimum simulation speed definition before the simulation ends (unfinished)
//
// Revision 1.32  2005/10/07 11:36:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.31  2005/09/22 13:30:40  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.30  2005/09/15 11:05:28  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.29  2005/05/04 07:48:05  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.28  2005/02/01 10:07:24  dkrajzew
// performance computation added
//
// Revision 1.27  2004/12/16 12:20:09  dkrajzew
// debugging
//
// Revision 1.26  2004/11/25 16:26:46  dkrajzew
// consolidated and debugged some detectors and their usage
//
// Revision 1.25  2004/11/23 10:11:33  dkrajzew
// adapted the new class hierarchy
//
// Revision 1.24  2004/07/02 08:28:50  dkrajzew
// some changes needed to derive the threading classes more easily added
//
// Revision 1.23  2004/04/02 11:10:20  dkrajzew
// simulation-wide output files are now handled by MSNet directly
//
// Revision 1.22  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.21  2004/02/05 16:29:30  dkrajzew
// memory leaks removed
//
// Revision 1.20  2003/12/11 06:21:15  dkrajzew
// implemented MSVehicleControl as the instance responsible for vehicles
//
// Revision 1.19  2003/12/09 11:22:13  dkrajzew
// errors during simulation are now caught properly
//
// Revision 1.18  2003/12/04 13:24:45  dkrajzew
// error handliung improved
//
// Revision 1.17  2003/11/26 10:57:14  dkrajzew
// messages from the simulation are now also passed to the message handler
//
// Revision 1.16  2003/11/20 13:17:33  dkrajzew
// further work on aggregated views
//
// Revision 1.15  2003/11/12 14:06:33  dkrajzew
// visualisation of tl-logics added
//
// Revision 1.14  2003/11/11 08:42:14  dkrajzew
// synchronisation problems of parameter tracker updates patched
//
// Revision 1.13  2003/10/30 08:57:53  dkrajzew
// first implementation of aggregated views using E2-detectors
//
// Revision 1.12  2003/08/20 11:58:04  dkrajzew
// cleaned up a bit
//
// Revision 1.11  2003/07/22 14:56:46  dkrajzew
// changes due to new detector handling
//
// Revision 1.10  2003/06/19 10:56:03  dkrajzew
// user information about simulation ending added; the gui may shutdown on
//  end and be started with a simulation now;
//
// Revision 1.9  2003/06/06 11:12:38  dkrajzew
// deletion of singletons changed/added
//
// Revision 1.8  2003/06/06 10:33:47  dkrajzew
// changes due to moving the popup-menus into a subfolder
//
// Revision 1.7  2003/06/05 06:26:16  dkrajzew
// first tries to build under linux: warnings removed; Makefiles added
//
// Revision 1.6  2003/05/20 09:23:54  dkrajzew
// some statistics added; some debugging done
//
// Revision 1.5  2003/04/04 08:37:50  dkrajzew
// view centering now applies net size; closing problems debugged;
//  comments added; tootip button added
//
// Revision 1.4  2003/02/07 10:34:14  dkrajzew
// files updated
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <cassert>
#include <string>
#include <iostream>
#include <algorithm>

#include <utils/common/MsgRetrievingFunction.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <guisim/GUINet.h>
#include <microsim/MSVehicleControl.h>
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/events/GUIEvent_SimulationStep.h>
#include <utils/gui/events/GUIEvent_SimulationEnded.h>
#include "GUIApplicationWindow.h"
#include "GUIRunThread.h"
#include "GUIGlobals.h"
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/iodevices/SharedOutputDevices.h>
#include <utils/gui/windows/GUIAppGlobals.h>
#include <utils/common/SysUtils.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace FXEX;
using namespace std;


/* =========================================================================
 * member method definitions
 * ======================================================================= */
GUIRunThread::GUIRunThread(MFXInterThreadEventClient *parent,
                           FXRealSpinDial &simDelay, MFXEventQue &eq,
                           FXEX::FXThreadEvent &ev)
    : FXSingleEventThread(gFXApp, parent), 
    _net(0), _quit(false), _simulationInProgress(false), _ok(true),
    mySimDelay(simDelay), myEventQue(eq), myEventThrow(ev)
{
    myErrorRetriever = new MsgRetrievingFunction<GUIRunThread>(this,
        &GUIRunThread::retrieveError);
    myMessageRetriever = new MsgRetrievingFunction<GUIRunThread>(this,
        &GUIRunThread::retrieveMessage);
    myWarningRetreiver = new MsgRetrievingFunction<GUIRunThread>(this,
        &GUIRunThread::retrieveWarning);
}


GUIRunThread::~GUIRunThread()
{
    // the thread shall stop
    _quit = true;
    deleteSim();
    delete myErrorRetriever;
    delete myMessageRetriever;
    delete myWarningRetreiver;
    // wait for the thread
    while(_simulationInProgress||_net!=0);
}


void
GUIRunThread::init(GUINet *net, SUMOTime start, SUMOTime end)
{
    // assign new values
    _net = net;
    _simStartTime = start;
    _simEndTime = end;
    _step = start;
    _net->initialiseSimulation();
    // register message callbacks
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
    MsgHandler::getMessageInstance()->addRetriever(myMessageRetriever);
    MsgHandler::getWarningInstance()->addRetriever(myWarningRetreiver);
}


FXint
GUIRunThread::run()
{
    SUMOTime beg = 0;
    SUMOReal end = 0;
    SUMOTime end2 = -1;
    // perform an endless loop
    while(!_quit) {
        // if the simulation shall be perfomed, do it
        if(!_halting&&_net!=0&&_ok) {
            if(getNet().logSimulationDuration()) {
                beg = SysUtils::getCurrentMillis();
                if(end2!=-1) {
                    getNet().setIdleDuration((int) (beg-end2));
                }
            }
            // check whether we shall stop at this step
            bool haltAfter =
                find(gBreakpoints.begin(), gBreakpoints.end(), _step)!=gBreakpoints.end();
            // do the step
            makeStep();
            // stop if wished
            if(haltAfter) {
                stop();
            }
            // wait if wanted
            SUMOReal val = (SUMOReal) mySimDelay.getValue();
            if(getNet().logSimulationDuration()) {
                end = SysUtils::getCurrentMillis();
                getNet().setSimDuration((int) (end-beg));
                end2 = SysUtils::getCurrentMillis();
            }
            if((int) val!=0) {
                sleep((int) val);
            }
        } else {
            // sleep if the siulation is not running
            sleep(500);
        }
    }
    // delete a maybe existing simulation at the end
    deleteSim();
    return 0;
}


void
GUIRunThread::makeStep()
{
    GUIEvent *e = 0;
    // simulation is being perfomed
    _simulationInProgress = true;
    // execute a single step
#ifndef _DEBUG
    try {
#endif
        mySimulationLock.lock();
        _net->simulationStep(_simStartTime, _step);
        _net->guiSimulationStep();
        mySimulationLock.unlock();

        // inform parent that a step has been performed
        e = new GUIEvent_SimulationStep();
        myEventQue.add(e);
        myEventThrow.signal();
        // increase step counter
        _step++;
        // stop the simulation when the last step has been reached
        if(_step==_simEndTime) {
            e = new GUIEvent_SimulationEnded(
                GUIEvent_SimulationEnded::ER_END_STEP_REACHED, _step);
            myEventQue.add(e);
            myEventThrow.signal();
            _halting = true;
        }
        // stop the execution when only a single step should have
        //  been performed
        if(_single) {
            _halting = true;
        }
        // simulation step is over
        _simulationInProgress = false;
        // check whether all vehicles loaded have left the simulation
        if(_net->getVehicleControl().haveAllVehiclesQuit()) {
            _halting = true;
            e = new GUIEvent_SimulationEnded(
                GUIEvent_SimulationEnded::ER_NO_VEHICLES, _step-1);
            myEventQue.add(e);
            myEventThrow.signal();
        }
#ifndef _DEBUG
    } catch (...) {
        mySimulationLock.unlock();
        _simulationInProgress = false;
        e = new GUIEvent_SimulationEnded(
            GUIEvent_SimulationEnded::ER_ERROR_IN_SIM, _step);
        myEventQue.add(e);
        myEventThrow.signal();
        _halting = true;
        _ok = false;
    }
#endif
    // check whether the simulation got too slow, halt then
    if(_net->logSimulationDuration() && _net->getTooSlowRTF()>0) {
        SUMOReal rtf =
            ((SUMOReal) _net->getVehicleControl().getRunningVehicleNo()/(SUMOReal) _net->getSimStepDurationInMillis()*(SUMOReal) 1000.);
        if(rtf<_net->getTooSlowRTF()) {
            _halting = true;
            e = new GUIEvent_SimulationEnded(
                GUIEvent_SimulationEnded::ER_NO_VEHICLES, _step-1);
            myEventQue.add(e);
            myEventThrow.signal();
        }
    }
}


void
GUIRunThread::resume()
{
    if(_step<_simEndTime) {
        _single = false;
        _halting = false;
    }
}


void
GUIRunThread::singleStep()
{
    _single = true;
    _halting = false;
}


void
GUIRunThread::begin()
{
#ifndef _DEBUG
    try {
#endif
        _step = _simStartTime;
        _single = false;
        _halting = false;
        _ok = true;
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("A serious error occured.");
        _ok = false;
        _simulationInProgress = false;
        GUIEvent_SimulationEnded *e = new GUIEvent_SimulationEnded(
            GUIEvent_SimulationEnded::ER_ERROR_IN_SIM, _step);
        myEventQue.add(e);
        myEventThrow.signal();
        _halting = true;
    }
#endif
}


void
GUIRunThread::stop()
{
    _single = false;
    _halting = true;
}


bool
GUIRunThread::simulationAvailable() const
{
    return _net!=0;
}


void
GUIRunThread::deleteSim()
{
    _halting = true;
    // remove message callbacks
    MsgHandler::getErrorInstance()->removeRetriever(myErrorRetriever);
    MsgHandler::getWarningInstance()->removeRetriever(myWarningRetreiver);
    MsgHandler::getMessageInstance()->removeRetriever(myMessageRetriever);
    //
    mySimulationLock.lock();
    if(_net!=0) {
        _net->closeSimulation(_simStartTime, _simEndTime);
    }
    while(_simulationInProgress);
    delete _net;
    _net = 0;
delete SharedOutputDevices::getInstance();
    mySimulationLock.unlock();
}


GUINet &
GUIRunThread::getNet() const
{
    return *_net;
}


SUMOTime
GUIRunThread::getCurrentTimeStep() const
{
    return _step;
}



void
GUIRunThread::prepareDestruction()
{
    _halting = true;
    _quit = true;
}


void
GUIRunThread::retrieveMessage(const std::string &msg)
{
    if(!_simulationInProgress) {
        return;
    }
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_MESSAGE, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


void
GUIRunThread::retrieveWarning(const std::string &msg)
{
    if(!_simulationInProgress) {
        return;
    }
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_WARNING, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


void
GUIRunThread::retrieveError(const std::string &msg)
{
    if(!_simulationInProgress) {
        return;
    }
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_ERROR, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


bool
GUIRunThread::simulationIsStartable() const
{
    return _net!=0&&(_halting||_single);
}


bool
GUIRunThread::simulationIsStopable() const
{
    return _net!=0&&(!_halting);
}


bool
GUIRunThread::simulationIsStepable() const
{
    return _net!=0&&(_halting||_single);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:




