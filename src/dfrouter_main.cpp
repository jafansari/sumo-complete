/***************************************************************************
                          main.cpp
              The main procedure for the build of person/vehicle routes
                             -------------------
    project              : SUMO
    subproject           : router
    begin                : Thu, 06 Jun 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.13  2006/03/27 07:34:54  dkrajzew
// some further work...
//
// Revision 1.12  2006/03/08 12:51:28  dkrajzew
// further work on the dfrouter
//
// Revision 1.11  2006/02/13 07:35:04  dkrajzew
// current work on the DFROUTER added (unfinished)
//
// Revision 1.9  2006/01/31 11:04:10  dkrajzew
// debugging
//
// Revision 1.8  2006/01/26 08:54:44  dkrajzew
// adapted the new router API
//
// Revision 1.7  2006/01/19 17:42:58  ericnicolay
// base classes for the reading of the detectorflows
//
// Revision 1.6  2006/01/17 14:12:30  dkrajzew
// routes output added; debugging
//
// Revision 1.5  2006/01/16 13:21:28  dkrajzew
// computation of detector types validated for the 'messstrecke'-scenario
//
// Revision 1.4  2006/01/16 10:46:24  dkrajzew
// some initial work on  the dfrouter
//
// Revision 1.3  2006/01/09 13:33:30  dkrajzew
// debugging error handling
//
// Revision 1.2  2005/12/21 12:48:38  ericnicolay
// *** empty log message ***
//
// Revision 1.1  2005/12/08 12:51:15  ericnicolay
// add new workingmap for the dfrouter
//
// Revision 0.1  2005/12/06 14:51:08  dkrajzew
// first start
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

#include <sax/SAXException.hpp>
#include <sax/SAXParseException.hpp>
#include <utils/common/TplConvert.h>
#include <iostream>
#include <string>
#include <fstream>
#include <limits.h>
#include <ctime>
#include <router/ROLoader.h>
#include <router/RONet.h>
#include <router/ROVehicleType_Krauss.h>
#include <routing_df/RODFEdgeBuilder.h>
#include <router/ROFrame.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/HelpPrinter.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <routing_df/RODFFrame.h>
#include <routing_df/DFRONet.h>
#include <routing_df/DFDetector.h>
#include <routing_df/DFDetectorHandler.h>
#include <routing_df/DFRORouteCont.h>
#include <routing_df/DFDetectorFlow.h>
#include <routing_df/DFDetFlowLoader.h>
#include "dfrouter_help.h"
#include "dfrouter_build.h"
#include "sumo_version.h"
#include <utils/common/XMLHelpers.h>
#include <utils/common/FileHelpers.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * functions
 * ======================================================================= */
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
/**
 * loads the net
 * The net is in this meaning made up by the net itself and the dynamic
 * weights which may be supplied in a separate file
 */
DFRONet *
loadNet(OptionsCont &oc)
{
    // load the network if wished
    if(!oc.isSet("net-file")) {
        return 0;
    }
    ROVehicleBuilder vb;
    ROLoader *loader = new ROLoader(oc, vb, false);
    // load the net
    RODFEdgeBuilder builder;
	RONet *ronet =  loader->loadNet( builder );
    delete loader;
	if(ronet==0) {
        throw ProcessError();
    }

	DFRONet *net = new DFRONet( ronet, oc.getBool("highway-mode") );
    net->buildApproachList();

    return net;
}


DFDetectorCon *
readDetectors(OptionsCont &oc)
{
    if(!oc.isSet("detectors-file")||!FileHelpers::exists(oc.getString("detectors-file"))) {
        MsgHandler::getErrorInstance()->inform("The detector file is not given or can not be opened.");
        throw ProcessError();
    }
    MsgHandler::getMessageInstance()->inform("Loading detector definitions... ");
    DFDetectorCon *cont = new DFDetectorCon();
    DFDetectorHandler handler(oc, *cont);
    SAX2XMLReader* parser = XMLHelpers::getSAXReader(handler);
    try {
        parser->parse(oc.getString("detectors-file").c_str());
        MsgHandler::getMessageInstance()->inform("done.");
    } catch (SAXException &e) {
        delete cont;
        cont = 0;
        MsgHandler::getErrorInstance()->inform(TplConvert<XMLCh>::_2str(e.getMessage()));
    } catch (XMLException &e) {
        delete cont;
        cont = 0;
        MsgHandler::getErrorInstance()->inform(TplConvert<XMLCh>::_2str(e.getMessage()));
    }
    return cont;
}


DFDetectorFlows *
readDetectorFlows( OptionsCont &oc, DFDetectorCon * dc)
{
	if(!oc.isSet("detector-flows-file")) {
		// ok, not given, return an empty container
		return new DFDetectorFlows(oc.getInt("begin"), oc.getInt("end"), 60); // !!!
	}
	// check whether the file exists
	if(!FileHelpers::exists(oc.getString("detector-flows-file"))) {
        MsgHandler::getErrorInstance()->inform("The detector-flows-file '" + oc.getString("detector-flows-file") + "' can not be opened.");
		throw ProcessError();
	}
	// parse
    MsgHandler::getMessageInstance()->inform("Loading flows... ");
	DFDetFlowLoader dfl(dc, oc.getInt("begin"), oc.getInt("end"), 60); // !!!
	DFDetectorFlows *df = dfl.read(
        oc.getString("detector-flows-file"), oc.getBool("fast-flows"));
//    df->buildFastAccess(oc.getInt("begin"), oc.getInt("end"), 60); // !!!
    MsgHandler::getMessageInstance()->inform("done.");
	return df;
}




/**
 * Computes the routes saving them
 */
void
startComputation(DFRONet *optNet, OptionsCont &oc)
{
    // read the detector definitions (mandatory)
    DFDetectorCon *detectors = readDetectors(oc);
    if(detectors==0) {
        throw 1;
    }
    // read routes (optionally)
	/*!!!
    DFRORouteCont *routes = new DFRORouteCont();
    if(oc.isSet("routes-input")) {
		throw 1;//!!!
        routes->readFrom(oc.getString("routes-input"));
    }
	*/
	DFDetectorFlows *flows = readDetectorFlows(oc, detectors);

    // if a network was loaded... (mode1)
    if(optNet!=0) {
        // compute the detector types (optionally)
        if(!detectors->detectorsHaveCompleteTypes()||oc.isSet("revalidate-detectors")) {
            MsgHandler::getMessageInstance()->inform("Computing detector types...");
            optNet->computeTypes(*detectors);
            MsgHandler::getMessageInstance()->inform("done.");
        }
        // compute routes between the detectors (optionally)
        if(!detectors->detectorsHaveRoutes()||oc.isSet("revalidate-routes")) {
            MsgHandler::getMessageInstance()->inform("Computing routes...");
            optNet->buildRoutes(*detectors,
                oc.getBool("all-end-follower"), oc.getBool("keep-unfound-ends"));
            MsgHandler::getMessageInstance()->inform("done.");
        }
    }

    // check
		// whether the detectors are valid
    if(!detectors->detectorsHaveCompleteTypes()) {
        MsgHandler::getErrorInstance()->inform("The detector types are not defined; use in combination with a network");
		throw ProcessError();
    }
		// whether the detectors have routes
    if(!detectors->detectorsHaveRoutes()) {
        MsgHandler::getErrorInstance()->inform("The emitters have no routes; use in combination with a network");
		throw ProcessError();
    }

    // save the detectors if wished
    if(oc.isSet("detectors-output")) {
        detectors->save(oc.getString("detectors-output"));
    }
	// save their positions as POIs if wished
    if(oc.isSet("detectors-poi-output")) {
        detectors->saveAsPOIs(oc.getString("detectors-poi-output"));
    }

    // save the routes file if it was changed or it's wished
    if(detectors->detectorsHaveRoutes()&&oc.isSet("routes-output")) {
        detectors->saveRoutes(oc.getString("routes-output"));
    }

	// !!!
	// save emitters if wished
	if(oc.isSet("emitters-output")) {
        MsgHandler::getMessageInstance()->inform("Writing emitters...");
        optNet->buildEdgeFlowMap(*flows, *detectors, 0, 86400, 60); // !!!
		detectors->writeEmitters(oc.getString("emitters-output"), *flows,
			0, 86400, 60,
			oc.getBool("write-calibrators"));
        MsgHandler::getMessageInstance()->inform("done.");
	}
    // save end speed trigger if wished
	if(oc.isSet("speed-trigger-output")) {
        MsgHandler::getMessageInstance()->inform("Writing speed triggers...");
		detectors->writeSpeedTrigger(oc.getString("speed-trigger-output"), *flows,
			0, 86400, 60);
        MsgHandler::getMessageInstance()->inform("done.");
	}
    // save checking detectors if wished
	if(oc.isSet("validation-output")) {
        MsgHandler::getMessageInstance()->inform("Writing validation detectors...");
		detectors->writeValidationDetectors(oc.getString("validation-output"),
            oc.getBool("validation-output.add-sources"), true, true); // !!!
        MsgHandler::getMessageInstance()->inform("done.");
	}
    // build global rerouter on end if wished
	if(oc.isSet("end-reroute-output")) {
        MsgHandler::getMessageInstance()->inform("Writing highway end rerouter...");
		detectors->writeEndRerouterDetectors(oc.getString("end-reroute-output")); // !!!
        MsgHandler::getMessageInstance()->inform("done.");
	}
	/*
    // save the emission definitions
    if(oc.isSet("flow-definitions")) {
        buildVehicleEmissions(oc.getString("flow-definitions"));
    }
	*/
    //
	delete flows;
    delete detectors;
//!!!    delete routes;
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    int ret = 0;
    DFRONet *net = 0;
#ifndef _DEBUG
    try {
#endif
        // initialise the application system (messaging, xml, options)
        int init_ret = SystemFrame::init(false, argc, argv, RODFFrame::fillOptions);
        if(init_ret<0) {
            cout << "SUMO dfrouter" << endl;
            cout << " (c) DLR/ZAIK 2000-2006; http://sumo.sourceforge.net" << endl;
            switch(init_ret) {
            case -1:
                cout << " Version " << version << endl;
                cout << " Build #" << NEXT_BUILD_NUMBER << endl;
                break;
            case -2:
                HelpPrinter::print(help);
                break;
            default:
                cout << " Use --help to get the list of options." << endl;
            }
            SystemFrame::close();
            return 0;
        } else if(init_ret!=0||!RODFFrame::checkOptions(OptionsSubSys::getOptions())) {
            throw ProcessError();
        }
        // retrieve the options
        OptionsCont &oc = OptionsSubSys::getOptions();
		ROFrame::setDefaults(oc);
        net = loadNet(oc);
        // build routes
        startComputation(net, oc);
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    }
#endif
    delete net;
    SystemFrame::close();
    if(ret==0) {
        cout << "Success." << endl;
    }
    return ret;
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

