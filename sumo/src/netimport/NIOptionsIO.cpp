/****************************************************************************/
/// @file    NIOptionsIO.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// A class for the initialisation, input and veryfying of the
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

#include <string>
#include <iostream>
#include <fstream>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "NIOptionsIO.h"
#include <utils/common/FileHelpers.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/RandHelper.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/SystemFrame.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
void
NIOptionsIO::fillOptions(OptionsCont &oc)
{
    // give some application descriptions
    oc.setApplicationDescription("Road network importer / builder for the road traffic simulation SUMO.");
#ifdef WIN32
    oc.setApplicationName("netconvert.exe", "SUMO netconvert Version " + (string)VERSION_STRING);
#else
    oc.setApplicationName("sumo-netconvert", "SUMO netconvert Version " + (string)VERSION_STRING);
#endif
    oc.addCallExample("-c <CONFIGURATION>");
    oc.addCallExample("-n ./nodes.xml -e ./edges.xml -v -t ./owntypes.xml");


    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Projection");
    oc.addOptionSubTopic("TLS Building");
    oc.addOptionSubTopic("Ramp Guessing");
    oc.addOptionSubTopic("Edge Removal");
    oc.addOptionSubTopic("Unregulated Nodes");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Building Defaults");
    oc.addOptionSubTopic("Report");

    // register options
    // register I/O options
    oc.doRegister("sumo-net", 's', new Option_FileName());
    oc.addDescription("sumo-net", "Input", "Read SUMO-net from FILE");

    oc.doRegister("xml-node-files", 'n', new Option_FileName());
    oc.addSynonyme("xml-node-files", "xml-nodes");
    oc.addDescription("xml-node-files", "Input", "Read XML-node defs from FILE");

    oc.doRegister("xml-edge-files", 'e', new Option_FileName());
    oc.addSynonyme("xml-edge-files", "xml-edges");
    oc.addDescription("xml-edge-files", "Input", "Read XML-edge defs from FILE");

    oc.doRegister("xml-connection-files", 'x', new Option_FileName());
    oc.addSynonyme("xml-connection-files", "xml-connections");
    oc.addDescription("xml-connection-files", "Input", "Read XML-connection defs from FILE");

    oc.doRegister("xml-type-files", 't', new Option_FileName());
    oc.addSynonyme("xml-type-files", "xml-types");
    oc.addDescription("xml-type-files", "Input", "Read XML-type defs from FILE");

    oc.doRegister("arcview", new Option_FileName());
    oc.addDescription("arcview", "Input", "Read ARCVIEW-net from files starting with 'FILE'");

    oc.doRegister("elmar", new Option_FileName());
    oc.addDescription("elmar", "Input", "Read splitted Elmar-network from path 'FILE'");

    oc.doRegister("elmar2", new Option_FileName());
    oc.addDescription("elmar2", "Input", "Read unsplitted Elmar-network from path 'FILE'");

    oc.doRegister("tiger", new Option_FileName());
    oc.addDescription("tiger", "Input", "Read Tiger-network from path 'FILE'");

    oc.doRegister("cell-node-file", new Option_FileName());
    oc.addSynonyme("cell-node-file", "cell-nodes");
    oc.addDescription("cell-node-file", "Input", "Read Cell-nodes from FILE");

    oc.doRegister("cell-edge-file", new Option_FileName());
    oc.addSynonyme("cell-edge-file", "cell-edges");
    oc.addDescription("cell-edge-file", "Input", "Read Cell-edges from FILE");

    oc.doRegister("visum-file", new Option_FileName());
    oc.addSynonyme("visum-file", "visum");
    oc.addDescription("visum-file", "Input", "Read VISUM-net from FILE");

    oc.doRegister("vissim-file", new Option_FileName());
    oc.addSynonyme("vissim-file", "vissim");
    oc.addDescription("vissim-file", "Input", "Read VISSIM-net from FILE");

    oc.doRegister("artemis-path", new Option_FileName());
    oc.addSynonyme("artemis-path", "artemis");
    oc.addDescription("artemis-path", "Input", "Read ARTEMIS-net from path 'FILE'");


    // register processing options
    oc.doRegister("dismiss-loading-errors", new Option_Bool(false)); // !!! describe, document
    oc.addDescription("dismiss-loading-errors", "Processing", "Continue on broken input");

    oc.doRegister("capacity-norm", 'N', new Option_Float((SUMOReal) 20000));
    oc.addDescription("capacity-norm", "Processing", "The factor for flow to no. lanes conv. (Cell)");

    oc.doRegister("speed-in-kmh", new Option_Bool(false));
    oc.addDescription("speed-in-kmh", "Processing", "vmax is parsed as given in km/h (some)");


    oc.doRegister("arcview.street-id", new Option_String());
    oc.addDescription("arcview.street-id", "Processing", "Read edge ids from column STR (ArcView)");

    oc.doRegister("arcview.from-id", new Option_String());
    oc.addDescription("arcview.from-id", "Processing", "Read from-node ids from column STR (ArcView)");

    oc.doRegister("arcview.to-id", new Option_String());
    oc.addDescription("arcview.to-id", "Processing", "Read to-node ids from column STR (ArcView)");

    oc.doRegister("arcview.type-id", new Option_String());
    oc.addDescription("arcview.type-id", "Processing", "Read type ids from column STR (ArcView)");

    oc.doRegister("arcview.use-defaults-on-failure", new Option_Bool(false));
    oc.addDescription("arcview.use-defaults-on-failure", "Processing", "Uses edge type defaults on problems (ArcView)");

    oc.doRegister("arcview.all-bidi", new Option_Bool(false));
    oc.addDescription("arcview.all-bidi", "Processing", "Insert edges in both directions (ArcView)");

    oc.doRegister("arcview.utm", new Option_Integer(32));
    oc.addDescription("arcview.utm", "Processing", "Use INT as UTM zone (ArcView)");

    oc.doRegister("arcview.guess-projection", new Option_Bool(false));
    oc.addDescription("arcview.guess-projection", "Processing", "Guess the proper projection (ArcView)");

    // register further vissim-options
    oc.doRegister("vissim.offset", new Option_Float(5.0f));
    oc.addDescription("vissim.offset", "Processing", "Structure join offset (VISSIM)");

    oc.doRegister("vissim.default-speed", new Option_Float(50.0f/3.6f));
    oc.addDescription("vissim.default-speed", "Processing", "Use FLOAT as default speed (VISSIM)");

    oc.doRegister("vissim.speed-norm", new Option_Float(1.0f));
    oc.addDescription("vissim.speed-norm", "Processing", "Factor for edge velocity (VISSIM)");

    // register further vissim-options
    oc.doRegister("visum.use-net-prio", new Option_Bool(false));
    oc.addDescription("visum.use-net-prio", "Processing", "Uses priorities from types");


    // register further navteq-options
    oc.doRegister("navtech.rechecklanes", new Option_Bool(false));
    oc.addDescription("navtech.rechecklanes", "Processing", "");

    // add netbuilding options
    NBNetBuilder::insertNetBuildOptions(oc);

    // add rand options
    RandHelper::insertRandOptions(oc);
}


bool
NIOptionsIO::checkOptions(OptionsCont &oc)
{
    bool ok = true;
    try {
        if (!checkCompleteDescription(oc)) {
            if (!checkNodes(oc)) ok = false;
            if (!checkEdges(oc)) ok = false;
            if (!checkOutput(oc)) ok = false;
        }
    } catch (InvalidArgument &e) {
        MsgHandler::getErrorInstance()->inform(e.what());
        return false;
    }
    return ok;
}


bool
NIOptionsIO::checkCompleteDescription(OptionsCont &)
{
    return false;
}


bool
NIOptionsIO::checkNodes(OptionsCont &oc)
{
    // check the existance of a name for the nodes file
    if (oc.isSet("n") ||
            oc.isSet("e") ||
            oc.isSet("cell-nodes") ||
            oc.isSet("visum") ||
            oc.isSet("vissim") ||
            oc.isSet("artemis") ||
            oc.isSet("tiger") ||
            oc.isSet("elmar") ||
            oc.isSet("arcview") ||
            oc.isSet("sumo-net")) {
        return true;
    }
    MsgHandler::getErrorInstance()->inform("The nodes must be supplied.");
    return false;
}


bool
NIOptionsIO::checkEdges(OptionsCont &oc)
{
    // check whether at least a sections or a edges file is supplied
    if (oc.isSet("e") ||
            oc.isSet("cell-edges") ||
            oc.isSet("visum") ||
            oc.isSet("vissim") ||
            oc.isSet("artemis") ||
            oc.isSet("arcview") ||
            oc.isSet("sumo-net")) {
        return true;
    }
    MsgHandler::getErrorInstance()->inform("Either sections or edges must be supplied.");
    return false;
}


bool
NIOptionsIO::checkOutput(OptionsCont &oc)
{
    ofstream strm(oc.getString("output-file").c_str()); // !!! should be made when input are ok
    if (!strm.good()) {
        MsgHandler::getErrorInstance()->inform("The output file \"" + oc.getString("o") + "\" can not be build.");
        return false;
    }
    return true;
}



/****************************************************************************/

