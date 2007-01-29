//---------------------------------------------------------------------------//
//                        GUIAbstractLoadThread.cpp -
//  Class describing the thread that performs the loading of a simulation
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
// Revision 1.5  2006/11/21 08:19:08  dkrajzew
// debugging
//
// Revision 1.4  2006/04/18 07:54:32  dkrajzew
// unifying threads
//
// Revision 1.3  2005/10/10 12:11:33  dkrajzew
// debugging
//
// Revision 1.2  2005/09/15 12:20:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.1  2004/11/23 10:38:32  dkrajzew
// debugging
//
// Revision 1.1  2004/10/22 12:50:56  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.26  2004/07/02 08:28:50  dkrajzew
// some changes needed to derive the threading classes more easily added
//
// Revision 1.25  2004/04/02 11:10:20  dkrajzew
// simulation-wide output files are now handled by MSNet directly
//
// Revision 1.24  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.23  2004/02/16 13:44:26  dkrajzew
// dump output generating function renamed in order to add vehicle dump
//  ability in the future
//
// Revision 1.22  2004/02/10 07:05:58  dkrajzew
// debugging of network loading after a network failed to be loaded
//
// Revision 1.21  2003/12/11 06:18:03  dkrajzew
// network loading and initialisation improved
//
// Revision 1.20  2003/12/09 11:23:07  dkrajzew
// some memory leaks removed
//
// Revision 1.19  2003/12/04 13:22:48  dkrajzew
// better error handling applied
//
// Revision 1.18  2003/11/26 09:39:13  dkrajzew
// added a logging windows to the gui (the passing of more than a single
//  lane to come makes it necessary)
//
// Revision 1.17  2003/10/28 08:32:13  dkrajzew
// random number specification option added
//
// Revision 1.16  2003/09/22 14:54:22  dkrajzew
// some refactoring on GUIAbstractLoadThread-usage
//
// Revision 1.15  2003/09/22 12:27:02  dkrajzew
// qt-includion problems patched
//
// Revision 1.14  2003/09/05 14:45:44  dkrajzew
// first tries for an implementation of aggregated views
//
// Revision 1.13  2003/07/22 14:56:46  dkrajzew
// changes due to new detector handling
//
// Revision 1.12  2003/07/07 08:09:38  dkrajzew
// Some further error checking was added and the usage of the SystemFrame
//  was refined
//
// Revision 1.11  2003/06/24 08:09:28  dkrajzew
// implemented SystemFrame and applied the changes to all applications
//
// Revision 1.10  2003/06/19 10:56:03  dkrajzew
// user information about simulation ending added; the gui may shutdown on
//  end and be started with a simulation now;
//
// Revision 1.9  2003/06/18 11:04:53  dkrajzew
// new error processing adapted
//
// Revision 1.8  2003/06/06 11:12:37  dkrajzew
// deletion of singletons changed/added
//
// Revision 1.7  2003/06/06 10:33:47  dkrajzew
// changes due to moving the popup-menus into a subfolder
//
// Revision 1.6  2003/06/05 06:26:16  dkrajzew
// first tries to build under linux: warnings removed; Makefiles added
//
// Revision 1.5  2003/03/20 16:17:52  dkrajzew
// windows eol removed
//
// Revision 1.4  2003/03/12 16:55:18  dkrajzew
// centering of objects debugged
//
// Revision 1.3  2003/02/07 10:34:14  dkrajzew
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

#include <iostream>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/XMLBuildingExceptions.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/common/MsgHandler.h>
#include <utils/foxtools/MFXEventQue.h>
#include <sumo_only/SUMOFrame.h>
#include <utils/common/MsgRetrievingFunction.h>
#include "GUIAbstractLoadThread.h"
#include <utils/gui/events/GUIEvent_Message.h>
#include <utils/gui/events/GUIEvent_SimulationEnded.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include <utils/gui/windows/GUIAppGlobals.h>

#include <ctime>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;
using namespace FXEX;


/* =========================================================================
 * member method definitions
 * ======================================================================= */
GUIAbstractLoadThread::GUIAbstractLoadThread(MFXInterThreadEventClient *mw,
                             MFXEventQue &eq, FXEX::FXThreadEvent &ev)
    : FXSingleEventThread(gFXApp, mw), myParent(mw), myEventQue(eq),
    myEventThrow(ev)
{
    myErrorRetriever = new MsgRetrievingFunction<GUIAbstractLoadThread>(this,
        &GUIAbstractLoadThread::retrieveError);
    myMessageRetriever = new MsgRetrievingFunction<GUIAbstractLoadThread>(this,
        &GUIAbstractLoadThread::retrieveMessage);
    myWarningRetreiver = new MsgRetrievingFunction<GUIAbstractLoadThread>(this,
        &GUIAbstractLoadThread::retrieveWarning);
    MsgHandler::getErrorInstance()->addRetriever(myErrorRetriever);
}


GUIAbstractLoadThread::~GUIAbstractLoadThread()
{
    delete myErrorRetriever;
    delete myMessageRetriever;
    delete myWarningRetreiver;
}


void
GUIAbstractLoadThread::load(const std::string &file)
{
    _file = file;
    start();
}


void
GUIAbstractLoadThread::retrieveMessage(const std::string &msg)
{
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_MESSAGE, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


void
GUIAbstractLoadThread::retrieveWarning(const std::string &msg)
{
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_WARNING, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


void
GUIAbstractLoadThread::retrieveError(const std::string &msg)
{
    GUIEvent *e = new GUIEvent_Message(MsgHandler::MT_ERROR, msg);
    myEventQue.add(e);
    myEventThrow.signal();
}


const std::string &
GUIAbstractLoadThread::getFileName() const
{
    return _file;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


