/****************************************************************************/
/// @file    NWWriter_XML.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 11.05.2011
/// @version $Id$
///
// Exporter writing networks using XML (native input) format
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
#include <algorithm>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>
#include "NWFrame.h"
#include "NWWriter_SUMO.h"
#include "NWWriter_XML.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS



// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NWWriter_XML::writeNetwork(const OptionsCont &oc, NBNetBuilder &nb) {
    // check whether a matsim-file shall be generated
    if (!oc.isSet("plain-output-prefix")) {
        return;
    }
    // write nodes
    OutputDevice& device = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".nod.xml");
    device.writeXMLHeader("nodes", " encoding=\"iso-8859-1\"", "version=\"0.13\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://sumo.sf.net/xsd/nodes_file.xsd\"");
    NBNodeCont &nc = nb.getNodeCont();
    for (std::map<std::string, NBNode*>::const_iterator i=nc.begin(); i!=nc.end(); ++i) {
        NBNode *n = (*i).second;
        device.openTag(SUMO_TAG_NODE);
        device.writeAttr(SUMO_ATTR_ID, n->getID());
        if (GeoConvHelper::getDefaultInstance().usingInverseGeoProjection()) {
            device.setPrecision(GEO_OUTPUT_ACCURACY);
        }
        NWFrame::writePositionLong(n->getPosition(), device);
        device.setPrecision();
        device.writeAttr(SUMO_ATTR_TYPE, toString(n->getType()));
        if (n->isTLControlled()) {
            const std::set<NBTrafficLightDefinition*> &tlss = n->getControllingTLS();
            // set may contain multiple programs for the same id.
            // make sure ids are unique and sorted
            std::set<std::string> tlsIDs;
            for (std::set<NBTrafficLightDefinition*>::const_iterator it_tl =tlss.begin(); it_tl!=tlss.end(); it_tl++) {
                tlsIDs.insert((*it_tl)->getID());
            }
            std::vector<std::string> sortedIDs(tlsIDs.begin(), tlsIDs.end());
            sort(sortedIDs.begin(), sortedIDs.end());
            device.writeAttr(SUMO_ATTR_TLID, sortedIDs);
        }
        device.closeTag(true);
    }
    device.close();
    // write edges / connections
    OutputDevice& edevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".edg.xml");
    edevice.writeXMLHeader("edges", " encoding=\"iso-8859-1\"", "version=\"0.13\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://sumo.sf.net/xsd/edges_file.xsd\"");
    OutputDevice& cdevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".con.xml");
    cdevice.writeXMLHeader("connections", " encoding=\"iso-8859-1\"", "version=\"0.13\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://sumo.sf.net/xsd/connections_file.xsd\"");
    NBEdgeCont &ec = nb.getEdgeCont();
    bool noNames = !oc.getBool("output.street-names");
    for (std::map<std::string, NBEdge*>::const_iterator i=ec.begin(); i!=ec.end(); ++i) {
        // write the edge itself to the edges-files
        NBEdge *e = (*i).second;
        edevice.openTag(SUMO_TAG_EDGE);
        edevice.writeAttr(SUMO_ATTR_ID, e->getID());
        edevice.writeAttr(SUMO_ATTR_FROM, e->getFromNode()->getID());
        edevice.writeAttr(SUMO_ATTR_TO, e->getToNode()->getID());
        if (!noNames && e->getStreetName() != "") {
            edevice.writeAttr(SUMO_ATTR_NAME, e->getStreetName());
        }
        edevice.writeAttr(SUMO_ATTR_PRIORITY, e->getPriority());
        // write the type if given
        if (e->getTypeID() != "") {
            edevice.writeAttr(SUMO_ATTR_TYPE, e->getTypeID());
        }
        edevice.writeAttr(SUMO_ATTR_NUMLANES, e->getNumLanes());
        if (!e->hasLaneSpecificSpeed()) {
            edevice.writeAttr(SUMO_ATTR_SPEED, e->getSpeed());
        }
        // write non-default geometry
        if (!e->hasDefaultGeometry()) {
            edevice.writeAttr(SUMO_ATTR_SHAPE, e->getGeometry());
        }
        // write the spread type if not default ("right")
        if (e->getLaneSpreadFunction()!=LANESPREAD_RIGHT) {
            edevice.writeAttr(SUMO_ATTR_SPREADTYPE, toString(e->getLaneSpreadFunction()));
        }
        // write the length if it was specified
        if (e->hasLoadedLength()) {
            edevice.writeAttr(SUMO_ATTR_LENGTH, e->getLoadedLength());
        }
        // some attributes can be set by edge default or per lane. Write as default if possible (efficiency)
        if (e->getWidth() != NBEdge::UNSPECIFIED_WIDTH && !e->hasLaneSpecificWidth()) {
            edevice.writeAttr(SUMO_ATTR_WIDTH, e->getWidth());
        }
        if (e->getOffset() != NBEdge::UNSPECIFIED_OFFSET && !e->hasLaneSpecificOffset()) {
            edevice.writeAttr(SUMO_ATTR_OFFSET, e->getOffset());
        }
        if (!e->needsLaneSpecificOutput()) {
            edevice.closeTag(true);
        } else {
            edevice << ">\n";
            for (unsigned int i=0; i<e->getLanes().size(); ++i) {
                const NBEdge::Lane &lane = e->getLanes()[i];
                edevice.openTag(SUMO_TAG_LANE);
                edevice.writeAttr(SUMO_ATTR_INDEX, i);
                // write allowed lanes
                if (lane.allowed.size()!=0) {
                    edevice.writeAttr(SUMO_ATTR_ALLOW, getVehicleClassNames(lane.allowed));
                }
                if (lane.notAllowed.size()!=0) {
                    edevice.writeAttr(SUMO_ATTR_DISALLOW, getVehicleClassNames(lane.notAllowed));
                }
                if (lane.preferred.size()!=0) {
                    edevice.writeAttr(SUMO_ATTR_PREFER, getVehicleClassNames(lane.preferred));
                }
                if (lane.width != NBEdge::UNSPECIFIED_WIDTH && e->hasLaneSpecificWidth()) {
                    edevice.writeAttr(SUMO_ATTR_WIDTH, lane.width);
                }
                if (lane.offset != NBEdge::UNSPECIFIED_OFFSET && e->hasLaneSpecificOffset()) {
                    edevice.writeAttr(SUMO_ATTR_OFFSET, lane.offset);
                }
                if (e->hasLaneSpecificSpeed()) {
                    edevice.writeAttr(SUMO_ATTR_SPEED, lane.speed);
                }
                edevice.closeTag(true);
            }
            edevice.closeTag();
        }
        // write this edge's connections to the connections-files
        unsigned int noLanes = e->getNumLanes();
        e->sortOutgoingConnectionsByIndex();
        const std::vector<NBEdge::Connection> connections = e->getConnections();
        for (std::vector<NBEdge::Connection>::const_iterator c=connections.begin(); c!=connections.end(); ++c) {
            NWWriter_SUMO::writeConnection(cdevice, *e, *c, false, true);
        }
        if (connections.size() > 0) {
            cdevice << "\n";
        }
    }

    // write loaded prohibitions to the connections-file
    for (std::map<std::string, NBNode*>::const_iterator i=nc.begin(); i!=nc.end(); ++i) {
        NWWriter_SUMO::writeProhibitions(cdevice, i->second->getProhibitions());
    }
    edevice.close();
    cdevice.close();
}


/****************************************************************************/

