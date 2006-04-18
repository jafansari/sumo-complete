/***************************************************************************
                          NLDiscreteEventBuilder .h
                          A building helper for simulation actions
                             -------------------
    begin                : Sep, 2003
    copyright            : (C) 2003 by DLR http://ivf.dlr.de/
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
namespace
{
     const char rcsid[] =
         "$Id$";
}
// $Log$
// Revision 1.12  2006/04/18 08:05:45  dkrajzew
// beautifying: output consolidation
//
// Revision 1.11  2006/02/27 12:09:49  dkrajzew
// variants container named properly
//
// Revision 1.10  2006/02/23 11:27:57  dkrajzew
// tls may have now several programs
//
// Revision 1.9  2005/10/10 12:10:59  dkrajzew
// reworking the tls-API: made tls-control non-static; made net an element of traffic lights
//
// Revision 1.8  2005/10/07 11:41:49  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/15 12:04:36  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/05/04 08:39:46  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.5  2004/11/23 10:12:46  dkrajzew
// new detectors usage applied
//
// Revision 1.4  2004/02/16 13:49:08  dkrajzew
// loading of e2-link-dependent detectors added
//
// Revision 1.3  2004/01/26 07:07:36  dkrajzew
// work on detectors: e3-detectors loading and visualisation;
//  variable offsets and lengths for lsa-detectors;
//  coupling of detectors to tl-logics;
//  different detector visualistaion in dependence to his controller
//
// Revision 1.2  2004/01/12 14:36:21  dkrajzew
// removed some dead code; documentation added
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

#include "NLDiscreteEventBuilder.h"
#include <utils/xml/AttributesHandler.h>
#include <utils/sumoxml/SUMOXMLDefinitions.h>
#include <utils/common/MsgHandler.h>
#include <microsim/MSNet.h>
#include <microsim/actions/Command_SaveTLSState.h>
#include <microsim/MSEventControl.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/UtilExceptions.h>

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
NLDiscreteEventBuilder::NLDiscreteEventBuilder(MSNet &net)
    : AttributesHandler(sumoattrs, noSumoAttrs),
    myNet(net)
{
    myActions["SaveTLSState"] = EV_SAVETLSTATE;
}


NLDiscreteEventBuilder::~NLDiscreteEventBuilder()
{
}


void
NLDiscreteEventBuilder::addAction(const Attributes &attrs,
                                  const std::string &basePath)
{
    string type = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
    // check whether the type was given
    if(type=="") {
        MsgHandler::getErrorInstance()->inform("An action's type is not given.");
        return;
    }
    // get the numerical representation
    KnownActions::iterator i = myActions.find(type);
    if(i==myActions.end()) {
        MsgHandler::getErrorInstance()->inform("The action type '" + type + "' is not known.");
        return;
    }
    ActionType at = (*i).second;
    // build the action
    Command *a;
    switch(at) {
    case EV_SAVETLSTATE:
        a = buildSaveTLStateCommand(attrs, basePath);
        break;
    default:
        throw 1;
    }
}


Command *
NLDiscreteEventBuilder::buildSaveTLStateCommand(const Attributes &attrs,
                                                const std::string &basePath)
{
    // get the parameter
    string dest = getStringSecure(attrs, SUMO_ATTR_DEST, "");
    string source = getStringSecure(attrs, SUMO_ATTR_SOURCE, "*");
    // check the parameter
    if(dest==""||source=="") {
        MsgHandler::getErrorInstance()->inform("Incomplete description of an 'SaveTLSState'-action occured.");
        return 0;
    }
    if(!FileHelpers::isAbsolute(dest)) {
        dest = FileHelpers::getConfigurationRelative(basePath, dest);
    }
    // get the logics
    if(!myNet.getTLSControl().knows(source)) {
        MsgHandler::getErrorInstance()->inform("The traffic light logic to save (" + source +  ") is not given.");
        throw ProcessError();
    }
    const MSTLLogicControl::TLSLogicVariants &logics = myNet.getTLSControl().get(source);
    // build the action
    return new Command_SaveTLSState(logics, dest);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
