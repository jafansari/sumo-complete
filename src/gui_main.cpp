/***************************************************************************
                          main.cpp
			  The main procedure for the conversion /
			  building of networks
                             -------------------
    project              : SUMO
    subproject           : simulation
    begin                : Tue, 20 Nov 2001
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// $Log$
// Revision 1.5  2003/06/19 07:07:52  dkrajzew
// false order of calling XML- and Options-subsystems patched
//
// Revision 1.4  2003/06/18 11:26:15  dkrajzew
// new message and error processing: output to user may be a message, warning or an error now; it is reported to a Singleton (MsgHandler); this handler puts it further to output instances. changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.3  2003/04/16 09:57:05  dkrajzew
// additional parameter of maximum display size added
//
// Revision 1.2  2003/02/07 10:37:30  dkrajzew
// files updated
//
// Revision 1.1  2002/10/16 14:51:08  dkrajzew
// Moved from ROOT/sumo to ROOT/src; added further help and main files for netconvert, router, od2trips and gui version
//
// Revision 1.9  2002/07/31 17:42:10  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.12  2002/07/22 12:52:23  dkrajzew
// Source handling added
//
// Revision 1.11  2002/07/11 07:30:43  dkrajzew
// Option_FileName invented to allow relative path names within the configuration files; two not yet implemented parameter introduced
//
// Revision 1.10  2002/07/02 12:48:10  dkrajzew
// --help now does not require -c
//
// Revision 1.9  2002/07/02 08:16:19  dkrajzew
// Program flow changed to allow better options removal; return values corrected
//
// Revision 1.8  2002/06/17 15:57:43  dkrajzew
// unreferenced variable declarations removed
//
// Revision 1.7  2002/05/14 07:43:51  dkrajzew
// _SPEEDCHECK-methods moved completely to MSNet
//
// Revision 1.6  2002/04/29 13:52:07  dkrajzew
// the program flow, especially the handling of errors improved
//
// Revision 1.5  2002/04/18 06:04:53  dkrajzew
// Forgotten test switch removed
//
// Revision 1.4  2002/04/17 11:20:40  dkrajzew
// Windows-carriage returns removed
//
// Revision 1.3  2002/04/16 12:21:13  dkrajzew
// Usage of SUMO_DATA removed
//
// Revision 1.2  2002/04/15 06:55:47  dkrajzew
// new loading paradigm implemented
//
// Revision 2.6  2002/03/20 08:13:54  dkrajzew
// help-output added
//
// Revision 2.5  2002/03/15 12:45:49  dkrajzew
// Warning is set to true forever due to bugs in value testing when no warnings are used (will be fixed later)
//
// Revision 2.4  2002/03/14 08:09:13  traffic
// Option for no raw output added
//
// Revision 2.3  2002/03/07 07:54:43  traffic
// implemented the usage of stdout as the default raw output
//
// Revision 2.2  2002/03/06 10:12:17  traffic
// Enviroment variable changef from SUMO to SUMO_DATA
//
// Revision 2.1  2002/03/05 14:51:25  traffic
// SegViolation on unset path debugged
//
// Revision 2.0  2002/02/14 14:43:11  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.4  2002/02/13 15:35:33  croessel
// Merging sourceForge with tesseraCVS.
//
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#include <ctime>

#include <iostream>
#include <fstream>
#include <microsim/MSNet.h>
#include <microsim/MSEmitControl.h>
#include <netload/NLNetBuilder.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsParser.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/HelpPrinter.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/xml/XMLSubSys.h>
#include <qstring.h>
#include <qapplication.h>
#include <qgl.h>
#include <gui/GUIApplicationWindow.h>
#include "sumo_help.h"

/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* -------------------------------------------------------------------------
 * build options
 * ----------------------------------------------------------------------- */
OptionsCont *
getOptions(int argc, char **argv)
{
    OptionsCont *oc = new OptionsCont();
    // register screen size options
    oc->doRegister("max-gl-width", 'w', new Option_Integer(1152));
    oc->doRegister("max-gl-height", 'h', new Option_Integer(864));
    if(!OptionsParser::parse(oc, argc, argv)) {
        delete oc;
        return 0;
    }
    // check whether the parameter are ok
    if(oc->getInt("w")<0||oc->getInt("h")<0) {
        cout
            << "Both the screen's width and the screen's height must be larger than zero."
            << endl;
        delete oc;
        return 0;
    }
    return oc;
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    OptionsCont *oc = getOptions(argc, argv);
    if(oc==0) {
        return 1;
    }
//    srand(time(0));

    srand(1040208551);
    int ret = 0;
    try {
        if(!XMLSubSys::init()) {
            throw ProcessError();
        }
        // initialise the xml-subsystem
        if(!SystemFrame::init(true, 0)) {
            throw ProcessError();
        }
        // initialise the q-application
        QApplication a( argc, argv );
        if ( !QGLFormat::hasOpenGL() ) {
            MsgHandler::getErrorInstance()->inform(
                "This system has no OpenGL support. Exiting." );
	        throw ProcessError();
        }
        // build the main window
        GUIApplicationWindow * mw =
            new GUIApplicationWindow(
                oc->getInt("w"),
                oc->getInt("h"));
        a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
        mw->show();
        ret = a.exec();
    } catch(...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on error).");
        ret = 1;
    }
    SystemFrame::close(oc);
    return ret;
}


