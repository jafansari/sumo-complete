/****************************************************************************/
/// @file    NIXMLConnectionsHandler.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 17 Oct 2002
/// @version $Id$
///
// Used to parse the XML-descriptions of connections between
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
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
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
#include "NIXMLConnectionsHandler.h"
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/nodes/NBNode.h>
#include <utils/common/StringTokenizer.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/ToString.h>
#include <utils/common/TplConvert.h>
#include <utils/common/TplConvertSec.h>
#include <utils/xml/XMLBuildingExceptions.h>
#include <utils/common/UtilExceptions.h>
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
NIXMLConnectionsHandler::NIXMLConnectionsHandler(NBEdgeCont &ec)
        : SUMOSAXHandler("xml-connection-description"), myEdgeCont(ec)
{}


NIXMLConnectionsHandler::~NIXMLConnectionsHandler()
{}


NBConnection
NIXMLConnectionsHandler::parseConnection(const std::string &defRole, const string &def)
{
    // split from/to
    size_t div = def.find("->");
    if (div==string::npos) {
        addError("Missing connection divider in " + defRole + " '" + def + "'");
        return NBConnection(0, 0);
    }
    string fromDef = def.substr(0, div);
    string toDef = def.substr(div+2);

    // retrieve the edges
    // check whether the definition includes a lane information (do not process it)
    if (fromDef.find('_')!=string::npos) {
        fromDef = fromDef.substr(0, fromDef.find('_'));
    }
    if (toDef.find('_')!=string::npos) {
        toDef = fromDef.substr(0, toDef.find('_'));
    }
    // retrieve them now
    NBEdge *fromE = myEdgeCont.retrieve(fromDef);
    NBEdge *toE = myEdgeCont.retrieve(toDef);
    // check
    if (fromE==0) {
        addError("Could not find edge '" + fromDef + "' in " + defRole + " '" + def + "'");
        return NBConnection(0, 0);
    }
    if (toE==0) {
        addError("Could not find edge '" + toDef + "' in " + defRole + " '" + def + "'");
        return NBConnection(0, 0);
    }
    return NBConnection(fromE, toE);
}


NBNode *
NIXMLConnectionsHandler::getNode(const string &def)
{
    // split from/to (we can omit some checks as they already have been done in parseConnection)
    size_t div = def.find("->");
    string fromDef = def.substr(0, div);
    if (fromDef.find('_')!=string::npos) {
        fromDef = fromDef.substr(0, fromDef.find('_'));
    }
    return myEdgeCont.retrieve(fromDef)->getToNode();
}


void
NIXMLConnectionsHandler::myStartElement(SumoXMLTag /*element*/, const std::string &name,
                                        const Attributes &attrs)
{
    if (name=="reset") {
        string from = getStringSecure(attrs, SUMO_ATTR_FROM, "");
        string to = getStringSecure(attrs, SUMO_ATTR_TO, "");
        if (from.length()==0) {
            addError("A from-edge is not specified within one of the connections-resets.");
            return;
        }
        if (to.length()==0) {
            addError("A to-edge is not specified within one of the connection-resets.");
            return;
        }
        NBEdge *fromEdge = myEdgeCont.retrieve(from);
        NBEdge *toEdge = myEdgeCont.retrieve(to);
        if (fromEdge==0) {
            addError("The connection-source edge '" + from + "' to reset is not known.");
            return;
        }
        if (toEdge==0) {
            addError("The connection-destination edge '" + to + "' to reset is not known.");
            return;
        }
        fromEdge->removeFromConnections(toEdge);
    }

    if (name=="connection") {
        string from = getStringSecure(attrs, SUMO_ATTR_FROM, "");
        string to = getStringSecure(attrs, SUMO_ATTR_TO, "");
        if (from.length()==0) {
            addError("A from-edge is not specified within one of the connections");
            return;
        }
        // extract edges
        NBEdge *fromEdge = myEdgeCont.retrieve(from);
        NBEdge *toEdge = to.length()!=0 ? myEdgeCont.retrieve(to) : 0;
        // check whether they are valid
        if (fromEdge==0) {
            addError("The connection-source edge '" + from + "' is not known.");
            return;
        }
        if (toEdge==0 && to.length()!=0) {
            addError("The connection-destination edge '" + to + "' is not known.");
            return;
        }
        // parse the id
        string type = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
        string laneConn = getStringSecure(attrs, SUMO_ATTR_LANE, "");
        if (type=="edgebound"||laneConn=="") {
            parseEdgeBound(attrs, fromEdge, toEdge);
        } else if (type=="lanebound"||laneConn.size()!=0) {
            parseLaneBound(attrs, fromEdge, toEdge);
        } else {
            addError("Unknown type of connection");
        }
    }
    if (name=="prohibition") {
        string prohibitor = getStringSecure(attrs, SUMO_ATTR_PROHIBITOR, "");
        string prohibited = getStringSecure(attrs, SUMO_ATTR_PROHIBITED, "");
        NBConnection prohibitorC = parseConnection("prohibitor", prohibitor);
        NBConnection prohibitedC = parseConnection("prohibited", prohibited);
        if (prohibitorC.getFrom()==0||prohibitedC.getFrom()==0) {
            // something failed
            return;
        }
        NBNode *n = getNode(prohibitor);
        n->addSortedLinkFoes(prohibitorC, prohibitedC);
    }
}

void
NIXMLConnectionsHandler::parseEdgeBound(const Attributes &/*attrs*/,
                                        NBEdge *from,
                                        NBEdge *to)
{
    from->addEdge2EdgeConnection(to);
    /*    int noLanes;
        try {
            noLanes = getIntSecure(attrs, SUMO_ATTR_NOLANES, -1);
        } catch (NumberFormatException e) {
            addError("Not numeric lane in connection");
            return;
        }
        if(noLanes<0) {
            // !!! (what to do??)
        } else {
            // add connection
            for(size_t i=0; i<noLanes; i++) {
                from->addLane2LaneConnection(i, to, i);
            }
        }*/
}


void
NIXMLConnectionsHandler::parseLaneBound(const Attributes &attrs,
                                        NBEdge *from,
                                        NBEdge *to)
{
    if(to==0) {
        // do nothing if it's a dead end
        return;
    }
    string laneConn = getStringSecure(attrs, SUMO_ATTR_LANE, "");
    if (laneConn.length()==0) {
        addError("Not specified lane to lane connection");
        return;
    } else {
        // split the information
        StringTokenizer st(laneConn, ':');
        if (st.size()!=2) {
            addError("False lane to lane connection occured.");
            return;
        }
        // get the begin and the end lane
        int fromLane;
        int toLane;
        try {
            fromLane = TplConvertSec<char>::_2intSec(st.next().c_str(), -1);
            toLane = TplConvertSec<char>::_2intSec(st.next().c_str(), -1);
            if (!from->addLane2LaneConnection(fromLane, to, toLane, false, true)) {
                NBEdge *nFrom = from;
                bool toNext = true;
                do {
                    if (nFrom->getToNode()->getOutgoingEdges().size()!=1) {
                        toNext = false;
                        break;
                    }
                    NBEdge *t = nFrom->getToNode()->getOutgoingEdges()[0];
                    if (t->getID().substr(0, t->getID().find('/'))!=nFrom->getID().substr(0, nFrom->getID().find('/'))) {
                        toNext = false;
                        break;
                    }
                    if (toNext) {
                        nFrom = t;
                    }
                } while (toNext);
                if (nFrom==0||!nFrom->addLane2LaneConnection(fromLane, to, toLane, false)) {
                    WRITE_WARNING("Could not set loaded connection from '" + from->getID() + "_" + toString<int>(fromLane) + "' to '" + to->getID() + "_" + toString<int>(toLane) + "'.");
                } else {
                    from = nFrom;
                }
            }
        } catch (NumberFormatException) {
            addError("At least one of the defined lanes was not numeric");
        }
        //
        bool keepUncontrolled = getBoolSecure(attrs, SUMO_ATTR_UNCONTROLLED, false);
        if(keepUncontrolled) {
            from->disableConnection4TLS(fromLane, to, toLane);
        }
    }
}


void
NIXMLConnectionsHandler::myCharacters(SumoXMLTag /*element*/,
                                      const std::string &/*name*/,
                                      const std::string &/*chars*/)
{}


void
NIXMLConnectionsHandler::myEndElement(SumoXMLTag /*element*/,
                                      const std::string &/*name*/)
{}



/****************************************************************************/

