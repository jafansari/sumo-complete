/****************************************************************************/
/// @file    NIXMLTypesHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id:NIXMLTypesHandler.cpp 4701 2007-11-09 14:29:29Z dkrajzew $
///
// Importer for edge type information stored in XML
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
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include "NIXMLTypesHandler.h"
#include <netbuild/NBTypeCont.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/TplConvert.h>
#include <utils/common/MsgHandler.h>

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
NIXMLTypesHandler::NIXMLTypesHandler(NBTypeCont &tc)
        : SUMOSAXHandler("xml-types - file"),
        myTypeCont(tc)
{}


NIXMLTypesHandler::~NIXMLTypesHandler() throw()
{}


void
NIXMLTypesHandler::myStartElement(SumoXMLTag element,
                                  const Attributes &attrs) throw(ProcessError)
{
    if (element!=SUMO_TAG_TYPE) {
        return;
    }
    string id;
    try {
        // parse the id
        id = getString(attrs, SUMO_ATTR_ID);
        int priority = 0;
        int noLanes = 0;
        SUMOReal speed = 0;
        // get the priority
        try {
            priority = getIntSecure(attrs, SUMO_ATTR_PRIORITY, myTypeCont.getDefaultPriority());
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("Not numeric value for Priority (at tag ID='" + id + "').");
        }
        // get the number of lanes
        try {
            noLanes = getIntSecure(attrs, SUMO_ATTR_NOLANES, myTypeCont.getDefaultNoLanes());
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("Not numeric value for NoLanes (at tag ID='" + id + "').");
        }
        // get the speed
        try {
            speed = getFloatSecure(attrs, SUMO_ATTR_SPEED, (SUMOReal) myTypeCont.getDefaultSpeed());
        } catch (NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform("Not numeric value for Speed (at tag ID='" + id + "').");
        }
        // get the function
        NBEdge::EdgeBasicFunction function = NBEdge::EDGEFUNCTION_NORMAL;
        string functionS = getStringSecure(attrs, SUMO_ATTR_FUNCTION, "normal");
        if (functionS=="source") {
            function = NBEdge::EDGEFUNCTION_SOURCE;
        } else if (functionS=="sink") {
            function = NBEdge::EDGEFUNCTION_SINK;
        } else if (functionS!="normal"&&functionS!="") {
            MsgHandler::getErrorInstance()->inform("Unknown function '" + functionS + "' occured.");
        }
        // build the type
        if (!MsgHandler::getErrorInstance()->wasInformed()) {
            NBType *type = new NBType(id, noLanes, speed, priority, function);
            if (!myTypeCont.insert(type)) {
                MsgHandler::getErrorInstance()->inform("Duplicate type occured. ID='" + id + "'");
                delete type;
            }
        }
    } catch (EmptyData &) {
        WRITE_WARNING("No id given... Skipping.");
    }
}



/****************************************************************************/

