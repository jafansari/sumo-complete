/***************************************************************************
                          DFDetectorHandler.cpp
    A handler for loading detector descriptions
                             -------------------
    project              : SUMO
    begin                : Thu, 16.03.2006
    copyright            : (C) 2006 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.9  2006/11/16 10:50:51  dkrajzew
// warnings removed
//
// Revision 1.8  2006/04/18 08:05:45  dkrajzew
// beautifying: output consolidation
//
// Revision 1.7  2006/03/17 09:04:25  dkrajzew
// class-documentation added/patched
//
// Revision 1.6  2006/02/13 07:27:06  dkrajzew
// current work on the DFROUTER added (unfinished)
//
// Revision 1.5  2006/01/31 11:00:47  dkrajzew
// added the possibility to write detector positions as pois; debugging
//
// Revision 1.4  2006/01/17 14:12:30  dkrajzew
// routes output added; debugging
//
// Revision 1.3  2006/01/16 13:21:28  dkrajzew
// computation of detector types validated for the 'messstrecke'-scenario
//
// Revision 1.2  2006/01/16 10:46:24  dkrajzew
// some initial work on  the dfrouter
//
// Revision 1.1  2006/01/13 16:40:56  ericnicolay
// base classes for the reading of the detectordefs
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

#include <string>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/sumoxml/SUMOSAXHandler.h>
#include <utils/sumoxml/SUMOXMLDefinitions.h>
#include "DFDetectorHandler.h"

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
DFDetectorHandler::DFDetectorHandler(OptionsCont &oc, DFDetectorCon &con)
    : SUMOSAXHandler("Detector-Defintion"),
    myOptions(oc),  myContainer(con)
{
}


DFDetectorHandler::~DFDetectorHandler()
{
}


void
DFDetectorHandler::myStartElement(int /*element*/, const std::string &name,
                                  const Attributes &attrs)
{
    if(name=="detector_definition") {
        string id;
        try {
            id = getString(attrs, SUMO_ATTR_ID);
        } catch (EmptyData&) {
            MsgHandler::getErrorInstance()->inform("A detector without an id occured within '" + _file + ".");
            return;
        }
        string lane;
        try {
            lane = getString(attrs, SUMO_ATTR_LANE);
        } catch (EmptyData&) {
            MsgHandler::getErrorInstance()->inform("A detector without a lane information occured within '" + _file + "' (detector id='" + id + ").");
            return;
        }
        SUMOReal pos;
        try {
            pos = getFloat(attrs, SUMO_ATTR_POS);
        } catch (EmptyData&) {
            MsgHandler::getErrorInstance()->inform("A detector without a lane position occured within '" + _file + "' (detector id='" + id + ").");
            return;
        } catch (NumberFormatException&) {
            MsgHandler::getErrorInstance()->inform("Not numeric lane position within '" + _file + "' (detector id='" + id + ").");
            return;
        }
        string mml_type = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
        dfdetector_type type = TYPE_NOT_DEFINED;
        if(mml_type=="between") {
            type = BETWEEN_DETECTOR;
        } else if(mml_type=="source") {
            type = SOURCE_DETECTOR;
        } else if(mml_type=="highway_source") {
            type = HIGHWAY_SOURCE_DETECTOR;
        } else if(mml_type=="sink") {
            type = SINK_DETECTOR;
        }
        DFDetector *detector = new DFDetector(id, lane, pos, type);
        if(!myContainer.addDetector(detector)) {
            MsgHandler::getErrorInstance()->inform("Could not add detector '" + id + "' (probably the id is already used).");
			delete detector;
        }
    }
}





void
DFDetectorHandler::myCharacters(int element, const std::string&,
                                const std::string &/*chars*/)
{
    if(element==SUMO_TAG_EDGES) {
//        preallocateEdges(chars);
    }
}


/*void
DFDetectorHandler::preallocateEdges(const std::string &chars)
{
    StringTokenizer st(chars);
    while(st.hasNext()) {
        string id = st.next();
        _net.addEdge(myEdgeBuilder.buildEdge(id)); // !!! where is the edge deleted when failing?
    }
}*/


void
DFDetectorHandler::myEndElement(int, const std::string&)
{
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


