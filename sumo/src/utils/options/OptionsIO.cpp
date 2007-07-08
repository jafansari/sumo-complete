/****************************************************************************/
/// @file    OptionsIO.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 17 Dec 2001
/// @version $Id$
///
// Loads the configuration file using "OptionsLoader"
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

#include <string>
#include <iostream>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <cstdlib>
#include "OptionsIO.h"
#include "OptionsCont.h"
#include "OptionsLoader.h"
#include "OptionsParser.h"
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/TplConvert.h>

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
OptionsIO::getOptions(bool loadConfig, int argc, char **argv) throw(ProcessError)
{
    // preparse the options
    //  (maybe another configuration file was chosen)
    if (!OptionsParser::parse(argc, argv)) {
        throw ProcessError("Could not parse commandline options.");
    }
    // check whether to use the command line parameters only
    if (!loadConfig) {
        return;
    }
    // read the configuration when everything's ok
    OptionsCont::getOptions().resetWritable();
    loadConfiguration();
    // reparse the options
    //  (overwrite the settings from the configuration file)
    OptionsCont::getOptions().resetWritable();
    OptionsParser::parse(argc, argv);
}


void
OptionsIO::loadConfiguration() throw(ProcessError)
{
    OptionsCont &oc = OptionsCont::getOptions();
    if (!oc.exists("configuration-file") || !oc.isSet("configuration-file")) {
        return;
    }
    string path = oc.getString("configuration-file");
    if (!FileHelpers::exists(path)) {
        throw ProcessError("Could not find configuration '" + oc.getString("configuration-file") + "'.");
    }
    MsgHandler::getMessageInstance()->beginProcessMsg("Loading configuration...");
    // build parser
    SAXParser parser;
    parser.setValidationScheme(SAXParser::Val_Auto);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    // start the parsing
    OptionsLoader handler(path.c_str(), oc.getBool("verbose"));
    try {
        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);
        parser.parse(path.c_str());
        if (handler.errorOccured()) {
            throw ProcessError("Could not load configuration '" + path + "'.");
        }
    } catch (const XMLException &e) {
        throw ProcessError("Could not load configuration '" + path + "':\n " + TplConvert<XMLCh>::_2str(e.getMessage()));
    }
    MsgHandler::getMessageInstance()->endProcessMsg("done.");
}



/****************************************************************************/

