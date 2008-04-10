/****************************************************************************/
/// @file    GUIFrame.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 29.05.2005
/// @version $Id$
///
// Sets and checks options for gui applications
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

#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include "GUIFrame.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// methods
// ===========================================================================
void
GUIFrame::fillInitOptions()
{
    OptionsCont &oc = OptionsCont::getOptions();
    oc.doRegister("max-gl-width", 'w', new Option_Integer(1280));
    oc.addDescription("max-gl-width", "Open GL", ""); // !!!

    oc.doRegister("max-gl-height", 'h', new Option_Integer(1024));
    oc.addDescription("max-gl-height", "Open GL", ""); // !!!


    oc.doRegister("quit-on-end", 'Q', new Option_Bool(false));
    oc.addDescription("quit-on-end", "Process", "Quits the gui when the simulation stops");

    oc.doRegister("suppress-end-info", 'S', new Option_Bool(false));
    oc.addDescription("suppress-end-info", "Process", "Suppresses the information about the simulation's end");

    oc.doRegister("no-start", 'N', new Option_Bool(false));
    oc.addDescription("no-start", "Process", "Does not start the simulation after loading");


    oc.doRegister("disable-textures", 'T', new Option_Bool(false)); // !!!
    oc.addDescription("disable-textures", "Visualisation", "");


    // register report options
    oc.doRegister("verbose", 'v', new Option_Bool(false)); // !!!
    oc.addDescription("verbose", "Report", "Switches to verbose output");

    oc.doRegister("print-options", 'p', new Option_Bool(false));
    oc.addDescription("print-options", "Report", "Prints option values before processing");

    oc.doRegister("help", '?', new Option_Bool(false));
    oc.addDescription("help", "Report", "Prints this screen");

}


bool
GUIFrame::checkInitOptions()
{
    OptionsCont &oc = OptionsCont::getOptions();
    // check whether the parameter are ok
    if (oc.getInt("w")<0||oc.getInt("h")<0) {
        MsgHandler::getErrorInstance()->inform(
            "Both the screen's width and the screen's height must be larger than zero.");
        return false;
    }
    return true;
}



/****************************************************************************/

