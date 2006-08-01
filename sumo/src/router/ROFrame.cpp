//---------------------------------------------------------------------------//
//                        ROFrame.cpp -
//  Some helping methods for usage within sumo and sumo-gui
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
// Revision 1.13  2006/08/01 07:19:57  dkrajzew
// removed build number information
//
// Revision 1.12  2006/04/18 08:15:49  dkrajzew
// removal of loops added
//
// Revision 1.11  2006/04/07 10:41:47  dkrajzew
// code beautifying: embedding string in strings removed
//
// Revision 1.10  2006/01/26 08:37:23  dkrajzew
// removed warnings 4786
//
// Revision 1.9  2006/01/16 13:38:22  dkrajzew
// help and error handling patched
//
// Revision 1.8  2006/01/09 12:00:58  dkrajzew
// debugging vehicle color usage
//
// Revision 1.7  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/07/12 12:39:01  dkrajzew
// edge-based mean data implemented; previous lane-based is now optional
//
// Revision 1.3  2005/05/04 08:47:14  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; usage of the -R option within the jtr-router debugged
//
// Revision 1.2  2004/12/20 10:48:36  dkrajzew
// net-files changed to net-file
//
// Revision 1.1  2004/11/23 10:25:52  dkrajzew
// debugging
//
// Revision 1.2  2004/10/29 06:18:15  dksumo
// max-alternatives options added;
// fastened up the output of warnings and messages
//
// Revision 1.1  2004/10/22 12:50:23  dksumo
// initial checkin into an internal, standalone SUMO CVS
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
#include <fstream>
#include <ctime>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <router/ROVehicleType_Krauss.h>
#include "ROFrame.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
void
ROFrame::fillOptions(OptionsCont &oc)
{
    // add rand and dev options
    RandHelper::insertRandOptions(oc);
    // register the file i/o options
    oc.doRegister("configuration-file", 'c', new Option_FileName());
    oc.addSynonyme("configuration-file", "configuration");

    oc.doRegister("output", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "output");

    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");

    oc.doRegister("alternatives", 'a', new Option_FileName());

    oc.doRegister("weights", 'w', new Option_FileName());
    oc.addSynonyme("weights", "weight-files");

    oc.doRegister("lane-weights", 'l', new Option_FileName());

    // register the simulation settings
    oc.doRegister("begin", 'b', new Option_Integer(0));
    oc.doRegister("end", 'e', new Option_Integer(864000));

    // register vehicle type defaults
    oc.doRegister("krauss-vmax", 'V', new Option_Float(SUMOReal(70)));
    oc.doRegister("krauss-a", 'A', new Option_Float(SUMOReal(2.6)));
    oc.doRegister("krauss-b", 'B', new Option_Float(SUMOReal(4.5)));
    oc.doRegister("krauss-length", 'L', new Option_Float(SUMOReal(5)));
    oc.doRegister("krauss-eps", 'E', new Option_Float(SUMOReal(0.5)));

    // register the report options
    oc.doRegister("verbose", 'v', new Option_Bool(false));
    oc.doRegister("suppress-warnings", 'W', new Option_Bool(false));
    oc.doRegister("print-options", 'p', new Option_Bool(false));
    oc.doRegister("help", new Option_Bool(false));
    oc.doRegister("log-file", 'l', new Option_FileName());
    oc.doRegister("stats-period", new Option_Integer(-1));

    // register the data processing options
    oc.doRegister("continue-on-unbuild", new Option_Bool(false));
    oc.doRegister("unsorted", new Option_Bool(false));
    oc.doRegister("move-on-short", new Option_Bool(false));
    oc.doRegister("max-alternatives", new Option_Integer(5));
    // add possibility to insert random vehicles
    oc.doRegister("random-per-second", 'R', new Option_Float());
    oc.doRegister("prune-random", new Option_Bool(false));
    oc.doRegister("remove-loops", new Option_Bool(false)); // !!! undescibed
}



bool
ROFrame::checkOptions(OptionsCont &oc)
{
    // check whether the output is valid and can be build
    if(!oc.isSet("o")) {
        MsgHandler::getErrorInstance()->inform("No output specified.");
        return false;
    }
    //
    if(oc.getInt("max-alternatives")<2) {
        MsgHandler::getErrorInstance()->inform("At least two alternatives should be enabled");
        return false;
    }
    //
    return true;
}


/**
 * Inserts the default from options into the vehicle
 *  type descriptions
 */
void
ROFrame::setDefaults(OptionsCont &oc)
{
    // insert the krauss-values
    ROVehicleType_Krauss::myDefault_A =
        oc.getFloat("krauss-a");
    ROVehicleType_Krauss::myDefault_B =
        oc.getFloat("krauss-b");
    ROVehicleType_Krauss::myDefault_EPS =
        oc.getFloat("krauss-eps");
    ROVehicleType_Krauss::myDefault_LENGTH =
        oc.getFloat("krauss-length");
    ROVehicleType_Krauss::myDefault_MAXSPEED =
        oc.getFloat("krauss-vmax");
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


