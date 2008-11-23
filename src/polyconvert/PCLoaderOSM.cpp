/****************************************************************************/
/// @file    PCLoaderOSM.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 19.11.2008
/// @version $Id: PCLoaderOSM.cpp 6350 2008-11-15 12:59:02Z dkrajzew $
///
// A reader of pois and polygons stored in OSM-format
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
#include <map>
#include <fstream>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/StdDefs.h>
#include <polyconvert/PCPolyContainer.h>
#include "PCLoaderOSM.h"
#include <utils/common/RGBColor.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Position2D.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/common/FileHelpers.h>

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
// ---------------------------------------------------------------------------
// static interface
// ---------------------------------------------------------------------------
void
PCLoaderOSM::loadIfSet(OptionsCont &oc, PCPolyContainer &toFill,
                       PCTypeMap &tm) throw(ProcessError)
{
    if (!oc.isSet("osm-files")) {
        return;
    }
    // parse file(s)
    vector<string> files = oc.getStringVector("osm-files");
    // load nodes, first
    std::map<int, PCOSMNode*> nodes;
    NodesHandler nodesHandler(nodes);
    for (vector<string>::const_iterator file=files.begin(); file!=files.end(); ++file) {
        // nodes
        if (!FileHelpers::exists(*file)) {
            MsgHandler::getErrorInstance()->inform("Could not open osm-file '" + *file + "'.");
            return;
        }
        nodesHandler.setFileName(*file);
        MsgHandler::getMessageInstance()->beginProcessMsg("Parsing nodes from osm-file '" + *file + "'...");
        if(!XMLSubSys::runParser(nodesHandler, *file)) {
            throw ProcessError();
        }
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
    }
    // load edges, then
    std::map<std::string, PCOSMEdge*> edges;
    EdgesHandler edgesHandler(nodes, edges);
    for (vector<string>::const_iterator file=files.begin(); file!=files.end(); ++file) {
        // edges
        edgesHandler.setFileName(*file);
        MsgHandler::getMessageInstance()->beginProcessMsg("Parsing edges from osm-file '" + *file + "'...");
        XMLSubSys::runParser(edgesHandler, *file);
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
    }
    // build all
    RGBColor c = RGBColor::parseColor(oc.getString("color"));
    // instatiate polygons
    for (std::map<std::string, PCOSMEdge*>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        PCOSMEdge *e = (*i).second;
        if (!e->myIsAdditional) {
            continue;
        }
        // compute shape
            Position2DVector vec;
            for(vector<int>::iterator j=e->myCurrentNodes.begin(); j!=e->myCurrentNodes.end(); ++j) {
                PCOSMNode *n = nodes.find(*j)->second;
                Position2D pos(n->lon, n->lat);
                GeoConvHelper::x2cartesian(pos);
                vec.push_back_noDoublePos(pos);
            }
            // set type etc.
            string name = e->id;
        string type;
        RGBColor color;
        bool fill = vec.getBegin()==vec.getEnd();
        bool discard = false;
        int layer = oc.getInt("layer");
        if (tm.has(e->myType)) {
            const PCTypeMap::TypeDef &def = tm.get(e->myType);
            name = def.prefix + name;
            type = def.id;
            color = RGBColor::parseColor(def.color);
            fill = fill && def.allowFill;
            discard = def.discard;
            layer = def.layer;
        } else if (e->myType.find(".")!=string::npos&&tm.has(e->myType.substr(0, e->myType.find(".")))) {
            const PCTypeMap::TypeDef &def = tm.get(e->myType.substr(0, e->myType.find(".")));
            name = def.prefix + name;
            type = def.id;
            color = RGBColor::parseColor(def.color);
            fill = fill && def.allowFill;
            discard = def.discard;
            layer = def.layer;
        } else {
            name = oc.getString("prefix") + name;
            type = oc.getString("type");
            color = c;
        }
        if (!discard) {
            if(oc.getBool("osm.keep-full-type")) {
                type = e->myType;
            }
            Polygon2D *poly = new Polygon2D(name, type, color, vec, fill);
            toFill.insert(name, poly, layer);
        }
    }
    // instatiate pois
    for(map<int, PCOSMNode*>::iterator i=nodes.begin(); i!=nodes.end(); ++i) {
        PCOSMNode *n = (*i).second;
        if(!n->myIsAdditional) {
            continue;
        }

        // patch the values
        bool discard = false;
        int layer = oc.getInt("layer");
        string name = toString(n->id);
        string type;
        RGBColor color;
        if (tm.has(n->myType)) {
            const PCTypeMap::TypeDef &def = tm.get(n->myType);
            name = def.prefix + name;
            type = def.id;
            color = RGBColor::parseColor(def.color);
            discard = def.discard;
            layer = def.layer;
        } else if (type.find(".")!=string::npos&&tm.has(type.substr(0, type.find(".")))) {
            const PCTypeMap::TypeDef &def = tm.get(type.substr(0, type.find(".")));
            name = def.prefix + name;
            type = def.id;
            color = RGBColor::parseColor(def.color);
            discard = def.discard;
            layer = def.layer;
        } else {
            name = oc.getString("prefix") + name;
            type = oc.getString("type");
            color = c;
        }
        if (!discard) {
            if(oc.getBool("osm.keep-full-type")) {
                type = n->myType;
            }
            bool ignorePrunning = false;
            if (OptionsCont::getOptions().isInStringVector("prune.ignore", name)) {
                ignorePrunning = true;
            }
                Position2D pos(n->lon, n->lat);
                GeoConvHelper::x2cartesian(pos);
                PointOfInterest *poi = new PointOfInterest(name, type, pos, color);
            toFill.insert(name, poi, layer, ignorePrunning);
        }
    }


    // delete nodes
    for (std::map<int, PCOSMNode*>::const_iterator i=nodes.begin(); i!=nodes.end(); ++i) {
        delete(*i).second;
    }
    // delete edges
    for (std::map<std::string, PCOSMEdge*>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        delete(*i).second;
    }
}



// ---------------------------------------------------------------------------
// definitions of PCLoaderOSM::NodesHandler-methods
// ---------------------------------------------------------------------------
PCLoaderOSM::NodesHandler::NodesHandler(std::map<int, PCOSMNode*> &toFill) throw()
        : SUMOSAXHandler("osm - file"), myToFill(toFill), myLastNodeID(-1)
{}


PCLoaderOSM::NodesHandler::~NodesHandler() throw()
{}


void
PCLoaderOSM::NodesHandler::myStartElement(SumoXMLTag element, const SUMOSAXAttributes &attrs) throw(ProcessError)
{
    myParentElements.push_back(element);
    if (element==SUMO_TAG_NODE) {
        bool ok = true;
        int id = attrs.getIntReporting(SUMO_ATTR_ID, "node", 0, ok);
        if (!ok) {
            return;
        }
        myLastNodeID = -1;
        if (myToFill.find(id)==myToFill.end()) {
            myLastNodeID = id;
            // assume we are loading multiple files...
            //  ... so we won't report duplicate nodes
            PCOSMNode *toAdd = new PCOSMNode();
            toAdd->id = id;
            toAdd->myIsAdditional = false;
            try {
                toAdd->lon = attrs.getFloat(SUMO_ATTR_LON);
            } catch (EmptyData &) {
                MsgHandler::getErrorInstance()->inform("Node '" + toString(id) + "' has no lon information.");
                delete toAdd;
                return;
            } catch (NumberFormatException &) {
                MsgHandler::getErrorInstance()->inform("Node's '" + toString(id) + "' lon information is not numeric.");
                delete toAdd;
                return;
            }
            try {
                toAdd->lat = attrs.getFloat(SUMO_ATTR_LAT);
            } catch (EmptyData &) {
                MsgHandler::getErrorInstance()->inform("Node '" + toString(id) + "' has no lat information.");
                delete toAdd;
                return;
            } catch (NumberFormatException &) {
                MsgHandler::getErrorInstance()->inform("Node's '" + toString(id) + "' lat information is not numeric.");
                delete toAdd;
                return;
            }
            myToFill[toAdd->id] = toAdd;
        }
    }
    if (element==SUMO_TAG_TAG&&myParentElements.size()>2&&myParentElements[myParentElements.size()-2]==SUMO_TAG_NODE) {
        string key, value;
        try {
            // retrieve the id of the (geometry) node
            key = attrs.getString(SUMO_ATTR_K);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("'tag' in node '" + toString(myLastNodeID) + "' misses a value.");
            return;
        }
        try {
            // retrieve the id of the (geometry) node
            value = attrs.getString(SUMO_ATTR_V);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("'value' in node '" + toString(myLastNodeID) + "' misses a value.");
            return;
        }
        if (key=="waterway"||key=="aeroway"||key=="aerialway"||key=="power"||key=="man_made"||key=="building"||key=="leisure"||key=="amenity"||key=="shop"
            ||key=="tourism"||key=="historic"||key=="landuse"||key=="natural"||key=="military"||key=="boundary"||key=="sport") {
            if (myLastNodeID>=0) {
                myToFill[myLastNodeID]->myType = key + "." + value;
                myToFill[myLastNodeID]->myIsAdditional = true;
            }
        }
    }
}


void
PCLoaderOSM::NodesHandler::myEndElement(SumoXMLTag element) throw(ProcessError)
{
    if (element==SUMO_TAG_NODE) {
        myLastNodeID = -1;
    }
    myParentElements.pop_back();
}


// ---------------------------------------------------------------------------
// definitions of PCLoaderOSM::EdgesHandler-methods
// ---------------------------------------------------------------------------
PCLoaderOSM::EdgesHandler::EdgesHandler(
    const std::map<int, PCOSMNode*> &osmNodes,
    std::map<std::string, PCOSMEdge*> &toFill) throw()
        : SUMOSAXHandler("osm - file"),
        myOSMNodes(osmNodes), myEdgeMap(toFill)
{
}


PCLoaderOSM::EdgesHandler::~EdgesHandler() throw()
{
}


void
PCLoaderOSM::EdgesHandler::myStartElement(SumoXMLTag element,
        const SUMOSAXAttributes &attrs) throw(ProcessError)
{
    myParentElements.push_back(element);
    // parse "way" elements
    if (element==SUMO_TAG_WAY) {
        myCurrentEdge = new PCOSMEdge();
        myCurrentEdge->myIsAdditional = false;
        myCurrentEdge->myIsClosed = false;
        try {
            // retrieve the id of the edge
            myCurrentEdge->id = attrs.getString(SUMO_ATTR_ID);
        } catch (EmptyData &) {
            WRITE_WARNING("No edge id given... Skipping.");
            return;
        }
    }
    // parse "nd" (node) elements
    if (element==SUMO_TAG_ND) {
        bool ok = true;
        int ref = attrs.getIntReporting(SUMO_ATTR_REF, "nd", 0, ok);
        if (ok) {
            if (myOSMNodes.find(ref)==myOSMNodes.end()) {
                MsgHandler::getErrorInstance()->inform("The referenced geometry information (ref='" + toString(ref) + "') is not known");
                return;
            }
            myCurrentEdge->myCurrentNodes.push_back(ref);
        }
    }
    // parse values
    if (element==SUMO_TAG_TAG&&myParentElements.size()>2&&myParentElements[myParentElements.size()-2]==SUMO_TAG_WAY) {
        string key, value;
        try {
            // retrieve the id of the (geometry) node
            key = attrs.getString(SUMO_ATTR_K);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("'tag' in edge '" + myCurrentEdge->id + "' misses a value.");
            return;
        }
        try {
            // retrieve the id of the (geometry) node
            value = attrs.getString(SUMO_ATTR_V);
        } catch (EmptyData &) {
            MsgHandler::getErrorInstance()->inform("'value' in edge '" + myCurrentEdge->id + "' misses a value.");
            return;
        }
        if (key=="waterway"||key=="aeroway"||key=="aerialway"||key=="power"||key=="man_made"||key=="building"||key=="leisure"||key=="amenity"||key=="shop"
            ||key=="tourism"||key=="historic"||key=="landuse"||key=="natural"||key=="military"||key=="boundary"||key=="sport") {
            myCurrentEdge->myType = key + "." + value;
            myCurrentEdge->myIsAdditional = true;
        } else if (key=="area") {
            myCurrentEdge->myIsClosed = true;
        } 
    }
}


void
PCLoaderOSM::EdgesHandler::myEndElement(SumoXMLTag element) throw(ProcessError)
{
    myParentElements.pop_back();
    if (element==SUMO_TAG_WAY) {
        if (myCurrentEdge->myIsAdditional) {
            myEdgeMap[myCurrentEdge->id] = myCurrentEdge;
        } else {
            delete myCurrentEdge;
        }
        myCurrentEdge = 0;
    }
}


/****************************************************************************/

