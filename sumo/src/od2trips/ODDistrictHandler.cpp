/****************************************************************************/
/// @file    ODDistrictHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// An XML-Handler for districts
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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
#include <utility>
#include <iostream>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "ODDistrict.h"
#include "ODDistrictCont.h"
#include "ODDistrictHandler.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
ODDistrictHandler::ODDistrictHandler(ODDistrictCont &cont,
                                     const std::string &file) throw()
        : SUMOSAXHandler(file), myContainer(cont), myCurrentDistrict(0),
        myHaveWarnedAboutDeprecatedDistrict(false), myHaveWarnedAboutDeprecatedDSource(false), myHaveWarnedAboutDeprecatedDSink(false)  {}


ODDistrictHandler::~ODDistrictHandler() throw() {}


void
ODDistrictHandler::myStartElement(int element,
                                  const SUMOSAXAttributes &attrs) throw(ProcessError) {
    switch (element) {
    case SUMO_TAG_DISTRICT__DEPRECATED:
        if(!myHaveWarnedAboutDeprecatedDistrict) {
            myHaveWarnedAboutDeprecatedDistrict = true;
            WRITE_WARNING("'" + toString(SUMO_TAG_DISTRICT__DEPRECATED) + "' is deprecated, please use '" + toString(SUMO_TAG_TAZ) + "'.");
        }
    case SUMO_TAG_TAZ:
        openDistrict(attrs);
        break;
    case SUMO_TAG_DSOURCE__DEPRECATED:
        if(!myHaveWarnedAboutDeprecatedDSource) {
            myHaveWarnedAboutDeprecatedDSource = true;
            WRITE_WARNING("'" + toString(SUMO_TAG_DSOURCE__DEPRECATED) + "' is deprecated, please use '" + toString(SUMO_TAG_TAZSOURCE) + "'.");
        }
    case SUMO_TAG_TAZSOURCE:
        addSource(attrs);
        break;
    case SUMO_TAG_DSINK__DEPRECATED:
        if(!myHaveWarnedAboutDeprecatedDSink) {
            myHaveWarnedAboutDeprecatedDSink = true;
            WRITE_WARNING("'" + toString(SUMO_TAG_DSINK__DEPRECATED) + "' is deprecated, please use '" + toString(SUMO_TAG_TAZSINK) + "'.");
        }
    case SUMO_TAG_TAZSINK:
        addSink(attrs);
        break;
    default:
        break;
    }
}


void
ODDistrictHandler::myEndElement(int element) throw(ProcessError) {
    if (element==SUMO_TAG_DISTRICT__DEPRECATED||element==SUMO_TAG_TAZ) {
        closeDistrict();
    }
}


void
ODDistrictHandler::openDistrict(const SUMOSAXAttributes &attrs) throw() {
    myCurrentDistrict = 0;
    // get the id, report an error if not given or empty...
    bool ok = true;
    std::string id = attrs.getStringReporting(SUMO_ATTR_ID, 0, ok);
    if (!ok) {
        return;
    }
    myCurrentDistrict = new ODDistrict(id);
}


void
ODDistrictHandler::addSource(const SUMOSAXAttributes &attrs) throw() {
    std::pair<std::string, SUMOReal> vals = parseConnection(attrs);
    if (vals.second>=0) {
        myCurrentDistrict->addSource(vals.first, vals.second);
    }
}


void
ODDistrictHandler::addSink(const SUMOSAXAttributes &attrs) throw() {
    std::pair<std::string, SUMOReal> vals = parseConnection(attrs);
    if (vals.second>=0) {
        myCurrentDistrict->addSink(vals.first, vals.second);
    }
}



std::pair<std::string, SUMOReal>
ODDistrictHandler::parseConnection(const SUMOSAXAttributes &attrs) throw() {
    // check the current district first
    if (myCurrentDistrict==0) {
        return std::pair<std::string, SUMOReal>("", -1);
    }
    // get the id, report an error if not given or empty...
    bool ok = true;
    std::string id = attrs.getStringReporting(SUMO_ATTR_ID, 0, ok);
    if (!ok) {
        return std::pair<std::string, SUMOReal>("", -1);
    }
    // get the weight
    SUMOReal weight = attrs.getSUMORealReporting(SUMO_ATTR_WEIGHT, id.c_str(), ok);
    if (ok) {
        if (weight<0) {
            WRITE_ERROR("'probability' must be positive (in definition of " + 
                    attrs.getObjectType() + " '" + id + "').");
        } else {
            return std::pair<std::string, SUMOReal>(id, weight);
        }
    }
    return std::pair<std::string, SUMOReal>("", -1);
}


void
ODDistrictHandler::closeDistrict() throw() {
    if (myCurrentDistrict!=0) {
        myContainer.add(myCurrentDistrict->getID(), myCurrentDistrict);
    }
}



/****************************************************************************/

