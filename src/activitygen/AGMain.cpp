/****************************************************************************/
/// @file    AGMain.cpp
/// @author  Piotr Woznica & Walter Bamberger
/// @date    Tue, 20 Jul 2010
/// @version $Id$
///
// Main object of the ActivityGen application
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
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

#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
#include <exception>
#include <functional>
#include <typeinfo>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <router/RONet.h>
#include <router/ROLoader.h>
#include <duarouter/RODUAFrame.h>
#include <duarouter/RODUAEdgeBuilder.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/RandHelper.h>
#include <utils/common/DijkstraRouterTT.h>
#include <utils/common/DijkstraRouterEffort.h>
#include <router/RONetHandler.h>
#include <router/ROFrame.h>
//ActivityGen
#include "AGActivityGen.h"
#include "city/AGTime.h"

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
void initAndOptions(int argc, char *argv[]) {
	OptionsCont &oc = OptionsCont::getOptions();

	XMLSubSys::init(false);

	// Options handling
	oc.addCallExample("--net-file <INPUT>.net.xml --stat-file <INPUT>.stat.xml --output-file <OUTPUT>.rou.xml");
	oc.addCallExample("--net-file <INPUT>.net.xml --stat-file <INPUT>.stat.xml --output-file <OUTPUT>.rou.xml --duration-d <NBR_OF_DAYS>");

	// insert options sub-topics
	SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
	oc.addOptionSubTopic("Input");
	oc.addOptionSubTopic("Output");
	oc.addOptionSubTopic("Processing");
	oc.addOptionSubTopic("Defaults");
	oc.addOptionSubTopic("Time");
	SystemFrame::addReportOptions(oc); // fill this subtopic, too

	// insert options
	ROFrame::fillOptions(oc);
	oc.doRegister("expand-weights", new Option_Bool(false));
	oc.addDescription("expand-weights", "Processing",
			"Expand weights behind the simulation's end");
	//        addImportOptions();
	//        addDUAOptions();
	// add rand options

	// Options of ActivityGen
	oc.doRegister("debug", new Option_Bool(false));
	oc.addDescription("debug", "Report",
			"Detailled messages about every single step");

	oc.doRegister("stat-file", 's', new Option_FileName());
	oc.addDescription("stat-file", "Input", "Loads the SUMO-statistics FILE");

	oc.doRegister("duration-d", new Option_Integer());
	oc.addDescription("duration-d", "Time", "OPTIONAL sets the duration of the simulation in days");

	oc.doRegister("time-begin", new Option_Integer());
	oc.addDescription("time-begin", "Time", "OPTIONAL sets the time of beginning of the simulation during the first day (in seconds)");

	oc.doRegister("time-end", new Option_Integer());
	oc.addDescription("time-end", "Time", "OPTIONAL sets the time of ending of the simulation during the last day (in seconds)");

	RandHelper::insertRandOptions();
	OptionsIO::getOptions(true, argc, argv);
	MsgHandler::initOutputOptions();
	//if (!RODUAFrame::checkOptions()) throw ProcessError();
	RandHelper::initRandGlobal();
}

/// Loads the network
void loadNet(RONet &toFill, ROAbstractEdgeBuilder &eb) throw (ProcessError) {
	OptionsCont &oc = OptionsCont::getOptions();
	std::string file = oc.getString("net-file");
	if (file == "") {
		throw ProcessError("Missing definition of network to load!");
	}
	if (!FileHelpers::exists(file)) {
		throw ProcessError("The network file '" + file
				+ "' could not be found.");
	}
	MsgHandler::getMessageInstance()->beginProcessMsg("Loading net ...");
	RONetHandler handler(toFill, eb);
	handler.setFileName(file);
	if (!XMLSubSys::runParser(handler, file)) {
		MsgHandler::getMessageInstance()->endProcessMsg("failed.");
		throw ProcessError();
	} else {
		MsgHandler::getMessageInstance()->endProcessMsg("done.");
	}
}


int main(int argc, char *argv[]) {
	OptionsCont &oc = OptionsCont::getOptions();
	// give some application descriptions
	oc.setApplicationDescription(
			"Generates routes of persons throughout a day for the microscopic road traffic simulation SUMO.");
	oc.setApplicationName("activitygen", "SUMO activitygen Version " + (std::string)VERSION_STRING);
	RONet *net = 0;
	try {
		// Initialize subsystems and process options
		initAndOptions(argc, argv);
		if (oc.processMetaOptions(argc < 2)) {
			SystemFrame::close();
			return 0;
		}

		// Load network
		net = new RONet();
		RODUAEdgeBuilder builder(oc.getBool("expand-weights"), oc.getBool("interpolate"));
		loadNet(*net, builder);
		MsgHandler::getMessageInstance()->inform("Loaded " + toString(
				net->getEdgeNo()) + " edges.");

	} catch (ProcessError &pe) {
		cout << typeid(pe).name() << ": " << pe.what() << endl;
		return 1;

	} catch (exception &e) {
		cout << "Unknown Exception " << typeid(e).name() << ": " << e.what()
				<< endl;
		return 1;

	} catch (...) {
		cout << "Unknown Exception" << endl;
		return 1;
	}

	cout << setprecision(1);

    std::cout << "\n\t ---- begin AcitivtyGen ----\n" << std::endl;
    string statFile = oc.getString("stat-file");
    string routeFile = oc.getString("output-file");
    AGTime duration(1,0,0);
    AGTime begin(0);
    AGTime end(0);
    if(oc.isSet("duration-d"))
    {
    	duration.setDay(oc.getInt("duration-d"));
    }
    if(oc.isSet("time-begin"))
    {
    	begin.addSeconds(oc.getInt("time-begin") % 86400);
    }
    if(oc.isSet("time-end"))
    {
    	end.addSeconds(oc.getInt("time-end") % 86400);
    }
    AGActivityGen actiGen(statFile, routeFile, net);
    actiGen.importInfoCity();
    actiGen.makeActivityTrips(duration.getDay(), begin.getTime(), end.getTime());

    std::cout << "\n\t ---- end of ActivityGen ---- \n" << std::endl;
	return 0;
}

/****************************************************************************/

