//---------------------------------------------------------------------------//
//                        SystemFrame.cpp -
//  A set of actions common to all applications
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 23.06.2003
//  copyright            : (C) 2003 by Daniel Krajzewicz
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
// $Log$
// Revision 1.15  2006/08/01 07:22:23  dkrajzew
// removed build number information
//
// Revision 1.14  2006/04/07 10:41:48  dkrajzew
// code beautifying: embedding string in strings removed
//
// Revision 1.13  2006/01/16 13:38:23  dkrajzew
// help and error handling patched
//
// Revision 1.12  2006/01/09 13:31:17  dkrajzew
// debugging error handling
//
// Revision 1.11  2005/10/17 09:22:36  dkrajzew
// memory leaks removed
//
// Revision 1.10  2005/10/07 11:43:30  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.9  2005/09/15 12:13:08  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.8  2005/07/12 12:43:49  dkrajzew
// code style adapted
//
// Revision 1.7  2005/04/28 09:02:47  dkrajzew
// level3 warnings removed
//
// Revision 1.6  2004/11/23 10:27:45  dkrajzew
// debugging
//
// Revision 1.5  2003/10/27 10:54:31  dkrajzew
// problems on setting gui options patched -
//  - the configuration is not loaded directly any more
//
// Revision 1.4  2003/06/24 08:10:23  dkrajzew
// extended by the options sub system; dcumentation added
//
// Revision 1.3  2003/06/24 08:09:29  dkrajzew
// implemented SystemFrame and applied the changes to all applications
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

#include "SystemFrame.h"
#include <string>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/LogFile.h>
#include <utils/common/HelpPrinter.h>
#include "RandHelper.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * static member definitions
 * ======================================================================= */
LogFile *SystemFrame::myLogFile = 0;


/* =========================================================================
 * method definitions
 * ======================================================================= */
int
SystemFrame::init(bool gui, int argc, char **argv,
                    fill_options *fill_f,
                    check_options *check_f)
{
    // initialise the output for option processing
    MsgHandler::getErrorInstance()->report2cerr(true);
    MsgHandler::getErrorInstance()->report2cout(false);
    MsgHandler::getWarningInstance()->report2cerr(false);
    MsgHandler::getWarningInstance()->report2cout(true);
    MsgHandler::getMessageInstance()->report2cerr(false);
    MsgHandler::getMessageInstance()->report2cout(true);
    // initialise the xml-subsystem
    if(!XMLSubSys::init()) {
        return 2;
    }
    // initialise the options-subsystem
    if(argc<2&&!gui) {
        // no options are given
        return -3;
    }
    bool iret = OptionsSubSys::init(!gui, argc, argv, fill_f, check_f);
    // check whether the help shall be printed
    if(OptionsSubSys::getOptions().getBool("help")) {
        //HelpPrinter::print(help);
        return -2;
    }
    // check whether the settings shall be printed
    if(OptionsSubSys::getOptions().getBool("print-options")) {
        cout << OptionsSubSys::getOptions();
    }

    // were the options ok?
    if(!iret) {
        return -3;
    }

    // initialise the output
        // check whether it is a gui-version or not, first
    if(gui) {
        // within gui-based applications, nothing is reported to the console
        MsgHandler::getErrorInstance()->report2cout(false);
        MsgHandler::getErrorInstance()->report2cerr(false);
        MsgHandler::getWarningInstance()->report2cout(false);
        MsgHandler::getWarningInstance()->report2cerr(false);
        MsgHandler::getMessageInstance()->report2cout(false);
        MsgHandler::getMessageInstance()->report2cerr(false);
    } else {
        // within console-based applications, report everything to the console
        MsgHandler::getErrorInstance()->report2cout(false);
        MsgHandler::getErrorInstance()->report2cerr(true);
        MsgHandler::getWarningInstance()->report2cout(false);
        MsgHandler::getWarningInstance()->report2cerr(true);
        MsgHandler::getMessageInstance()->report2cout(true);
        MsgHandler::getMessageInstance()->report2cerr(false);
    }
        // then, check whether be verbose
    if(!gui&&!OptionsSubSys::getOptions().getBool("v")) {
        MsgHandler::getMessageInstance()->report2cout(false);
    }
        // check whether to suppress warnings
    if(!gui&&OptionsSubSys::getOptions().getBool("suppress-warnings")) {
        MsgHandler::getWarningInstance()->report2cout(false);
    }
    // build the logger if possible
    if(!gui&&OptionsSubSys::getOptions().isSet("log-file")) {
        myLogFile =
            new LogFile(OptionsSubSys::getOptions().getString("log-file"));
        if(!myLogFile->good()) {
            delete myLogFile;
            myLogFile = 0;
            MsgHandler::getErrorInstance()->inform("Could not build logging file '" + OptionsSubSys::getOptions().getString("log-file") + "'");
            return 1;
        } else {
            MsgHandler::getErrorInstance()->addRetriever(myLogFile);
            MsgHandler::getWarningInstance()->addRetriever(myLogFile);
            MsgHandler::getMessageInstance()->addRetriever(myLogFile);
        }
    }
    // return the state
    if(!gui) {
        RandHelper::initRandGlobal(OptionsSubSys::getOptions());
    }
    return 0;
}


void
SystemFrame::close()
{
    delete myLogFile;
    // close the xml-subsystem
    XMLSubSys::close();
    // delete build program options
    OptionsSubSys::close();
    // delete messages
    MsgHandler::cleanupOnEnd();
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
