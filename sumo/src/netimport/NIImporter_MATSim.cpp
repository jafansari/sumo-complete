/****************************************************************************/
/// @file    NIImporter_MATSim.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 26.04.2011
/// @version $Id$
///
// Importer for networks stored in MATSim format
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
#include <set>
#include <functional>
#include <sstream>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/TplConvert.h>
#include <utils/xml/XMLSubSys.h>
#include "NILoader.h"
#include "NIImporter_MATSim.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS



// ===========================================================================
// static variables
// ===========================================================================
StringBijection<int>::Entry NIImporter_MATSim::matsimTags[] = {
    { "network",          NIImporter_MATSim::MATSIM_TAG_NETWORK },
    { "node",             NIImporter_MATSim::MATSIM_TAG_NODE },
    { "link",             NIImporter_MATSim::MATSIM_TAG_LINK },
    { "links",            NIImporter_MATSim::MATSIM_TAG_LINKS },
    { "",                 NIImporter_MATSim::MATSIM_TAG_NOTHING }
};


StringBijection<int>::Entry NIImporter_MATSim::matsimAttrs[] = {
    { "id",             NIImporter_MATSim::MATSIM_ATTR_ID },
    { "x",              NIImporter_MATSim::MATSIM_ATTR_X },
    { "y",              NIImporter_MATSim::MATSIM_ATTR_Y },
    { "from",           NIImporter_MATSim::MATSIM_ATTR_FROM },
    { "to",             NIImporter_MATSim::MATSIM_ATTR_TO },
    { "length",         NIImporter_MATSim::MATSIM_ATTR_LENGTH },
    { "freespeed",      NIImporter_MATSim::MATSIM_ATTR_FREESPEED },
    { "capacity",       NIImporter_MATSim::MATSIM_ATTR_CAPACITY },
    { "permlanes",      NIImporter_MATSim::MATSIM_ATTR_PERMLANES },
    { "oneway",         NIImporter_MATSim::MATSIM_ATTR_ONEWAY },
    { "modes",          NIImporter_MATSim::MATSIM_ATTR_MODES },
    { "origid",         NIImporter_MATSim::MATSIM_ATTR_ORIGID },
    { "capperiod",      NIImporter_MATSim::MATSIM_ATTR_CAPPERIOD },
    { "capDivider",     NIImporter_MATSim::MATSIM_ATTR_CAPDIVIDER },

    { "",               NIImporter_MATSim::MATSIM_ATTR_NOTHING }
};


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NIImporter_MATSim::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isSet("matsim-files")) {
        return;
    }
    /* Parse file(s)
     * Each file is parsed twice: first for nodes, second for edges. */
    std::vector<std::string> files = oc.getStringVector("matsim-files");
    // load nodes, first
    NodesHandler nodesHandler(nb.getNodeCont());
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        // nodes
        if (!FileHelpers::exists(*file)) {
            WRITE_ERROR("Could not open matsim-file '" + *file + "'.");
            return;
        }
        nodesHandler.setFileName(*file);
        PROGRESS_BEGIN_MESSAGE("Parsing nodes from matsim-file '" + *file + "'");
        if (!XMLSubSys::runParser(nodesHandler, *file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();
    }
    // load edges, then
    EdgesHandler edgesHandler(nb.getNodeCont(), nb.getEdgeCont(), oc.getBool("matsim.keep-length"),
                              oc.getBool("matsim.lanes-from-capacity"), NBCapacity2Lanes(oc.getFloat("lanes-from-capacity.norm")));
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        // edges
        edgesHandler.setFileName(*file);
        PROGRESS_BEGIN_MESSAGE("Parsing edges from matsim-file '" + *file + "'");
        XMLSubSys::runParser(edgesHandler, *file);
        PROGRESS_DONE_MESSAGE();
    }
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_MATSim::NodesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_MATSim::NodesHandler::NodesHandler(NBNodeCont& toFill) throw()
    : GenericSAXHandler(matsimTags, MATSIM_TAG_NOTHING,
                        matsimAttrs, MATSIM_ATTR_NOTHING,
                        "matsim - file"), myNodeCont(toFill) {
}


NIImporter_MATSim::NodesHandler::~NodesHandler() throw() {}


void
NIImporter_MATSim::NodesHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) throw(ProcessError) {
    if (element != MATSIM_TAG_NODE) {
        return;
    }
    // get the id, report a warning if not given or empty...
    bool ok = true;
    std::string id = attrs.getStringReporting(MATSIM_ATTR_ID, 0, ok);
    SUMOReal x = attrs.getSUMORealReporting(MATSIM_ATTR_X, id.c_str(), ok);
    SUMOReal y = attrs.getSUMORealReporting(MATSIM_ATTR_Y, id.c_str(), ok);
    if (!ok) {
        return;
    }
    Position pos(x, y);
    if (!NILoader::transformCoordinates(pos)) {
        WRITE_ERROR("Unable to project coordinates for node '" + id + "'.");
    }
    NBNode* node = new NBNode(id, pos);
    if (!myNodeCont.insert(node)) {
        delete node;
        WRITE_ERROR("Could not add node '" + id + "'. Probably declared twice.");
    }
}



// ---------------------------------------------------------------------------
// definitions of NIImporter_MATSim::EdgesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_MATSim::EdgesHandler::EdgesHandler(const NBNodeCont& nc, NBEdgeCont& toFill,
        bool keepEdgeLengths, bool lanesFromCapacity,
        NBCapacity2Lanes capacity2Lanes) throw()
    : GenericSAXHandler(matsimTags, MATSIM_TAG_NOTHING,
                        matsimAttrs, MATSIM_ATTR_NOTHING, "matsim - file"),
    myNodeCont(nc), myEdgeCont(toFill), myCapacityNorm(3600),
    myKeepEdgeLengths(keepEdgeLengths), myLanesFromCapacity(lanesFromCapacity),
    myCapacity2Lanes(capacity2Lanes) {
}


NIImporter_MATSim::EdgesHandler::~EdgesHandler() throw() {
}


void
NIImporter_MATSim::EdgesHandler::myStartElement(int element,
        const SUMOSAXAttributes& attrs) throw(ProcessError) {
    bool ok = true;
    if (element == MATSIM_TAG_NETWORK) {
        if (attrs.hasAttribute(MATSIM_ATTR_CAPDIVIDER)) {
            int capDivider = attrs.getIntReporting(MATSIM_ATTR_CAPDIVIDER, "network", ok);
            if (ok) {
                myCapacityNorm = (SUMOReal)(capDivider * 3600);
            }
        }
    }
    if (element == MATSIM_TAG_LINKS) {
        bool ok = true;
        std::string capperiod = attrs.getStringReporting(MATSIM_ATTR_CAPPERIOD, "links", ok);
        StringTokenizer st(capperiod, ":");
        if (st.size() != 3) {
            WRITE_ERROR("Bogus capacity period format; requires 'hh:mm:ss'.");
            return;
        }
        try {
            int hours = TplConvert<char>::_2int(st.next().c_str());
            int minutes = TplConvert<char>::_2int(st.next().c_str());
            int seconds = TplConvert<char>::_2int(st.next().c_str());
            myCapacityNorm = (SUMOReal)(hours * 3600 + minutes * 60 + seconds);
        } catch (NumberFormatException&) {
        } catch (EmptyData&) {
        }
        return;
    }

    // parse "link" elements
    if (element != MATSIM_TAG_LINK) {
        return;
    }
    std::string id = attrs.getStringReporting(MATSIM_ATTR_ID, 0, ok);
    std::string fromNodeID = attrs.getStringReporting(MATSIM_ATTR_FROM, id.c_str(), ok);
    std::string toNodeID = attrs.getStringReporting(MATSIM_ATTR_TO, id.c_str(), ok);
    SUMOReal length = attrs.getSUMORealReporting(MATSIM_ATTR_LENGTH, id.c_str(), ok); // override computed?
    SUMOReal freeSpeed = attrs.getSUMORealReporting(MATSIM_ATTR_FREESPEED, id.c_str(), ok); //
    SUMOReal capacity = attrs.getSUMORealReporting(MATSIM_ATTR_CAPACITY, id.c_str(), ok); // override permLanes?
    SUMOReal permLanes = attrs.getSUMORealReporting(MATSIM_ATTR_PERMLANES, id.c_str(), ok);
    //bool oneWay = attrs.getOptBoolReporting(MATSIM_ATTR_ONEWAY, id.c_str(), ok, true); // mandatory?
    std::string modes = attrs.getOptStringReporting(MATSIM_ATTR_MODES, id.c_str(), ok, ""); // which values?
    std::string origid = attrs.getOptStringReporting(MATSIM_ATTR_ORIGID, id.c_str(), ok, "");
    NBNode* fromNode = myNodeCont.retrieve(fromNodeID);
    NBNode* toNode = myNodeCont.retrieve(toNodeID);
    if (fromNode == 0) {
        WRITE_ERROR("Could not find from-node for edge '" + id + "'.");
    }
    if (toNode == 0) {
        WRITE_ERROR("Could not find to-node for edge '" + id + "'.");
    }
    if (fromNode == 0 || toNode == 0) {
        return;
    }
    if (myLanesFromCapacity) {
        permLanes = myCapacity2Lanes.get(capacity);
    }
    NBEdge* edge = new NBEdge(id, fromNode, toNode, "", freeSpeed, (unsigned int) permLanes, -1, -1, -1);
    if (myKeepEdgeLengths) {
        edge->setLoadedLength(length);
    }
    if (!myEdgeCont.insert(edge)) {
        delete edge;
        WRITE_ERROR("Could not add edge '" + id + "'. Probably declared twice.");
    }
}


/****************************************************************************/

