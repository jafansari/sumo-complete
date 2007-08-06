/****************************************************************************/
/// @file    RODFDetectorHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// A handler for loading detector descriptions
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
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "RODFDetectorHandler.h"

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
RODFDetectorHandler::RODFDetectorHandler(OptionsCont &oc, RODFDetectorCon &con,
                                     const std::string &file)
        : SUMOSAXHandler(file),
        myOptions(oc),  myContainer(con)
{}


RODFDetectorHandler::~RODFDetectorHandler() throw()
{}


void
RODFDetectorHandler::myStartElement(SumoXMLTag element,
                                  const Attributes &attrs) throw(ProcessError)
{
    if (element==SUMO_TAG_DETECTOR_DEFINITION) {
        string id;
        try {
            id = getString(attrs, SUMO_ATTR_ID);
        } catch (EmptyData&) {
            throw ProcessError("A detector without an id occured within '" + getFileName() + ".");
        }
        string lane;
        try {
            lane = getString(attrs, SUMO_ATTR_LANE);
        } catch (EmptyData&) {
            throw ProcessError("A detector without a lane information occured within '" + getFileName() + "' (detector id='" + id + ").");
        }
        SUMOReal pos;
        try {
            pos = getFloat(attrs, SUMO_ATTR_POS);
        } catch (EmptyData&) {
            throw ProcessError("A detector without a lane position occured within '" + getFileName() + "' (detector id='" + id + ").");
        } catch (NumberFormatException&) {
            throw ProcessError("Not numeric lane position within '" + getFileName() + "' (detector id='" + id + ").");
        }
        string mml_type = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
        RORODFDetectorType type = TYPE_NOT_DEFINED;
        if (mml_type=="between") {
            type = BETWEEN_DETECTOR;
        } else if (mml_type=="source"||mml_type=="highway_source") { // !!! highway-source is legacy (removed accoring output on 06.08.2007)
            type = SOURCE_DETECTOR;
        } else if (mml_type=="sink") {
            type = SINK_DETECTOR;
        }
        RODFDetector *detector = new RODFDetector(id, lane, pos, type);
        if (!myContainer.addDetector(detector)) {
            delete detector;
            throw ProcessError("Could not add detector '" + id + "' (probably the id is already used).");
        }
    }
}



/****************************************************************************/

