/****************************************************************************/
/// @file    NWWriter_DlrNavteq.h
/// @author  Jakob Erdmann
/// @date    26.10.2012
/// @version $Id: NWWriter_DlrNavteq.h 12012 2012-03-05 09:41:53Z namdre $
///
// Exporter writing networks using DlrNavteq (Elmar) format
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
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
#include <ctime>
#include <cmath>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>
#include "NWFrame.h"
#include "NWWriter_DlrNavteq.h"

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
NWWriter_DlrNavteq::writeNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether a matsim-file shall be generated
    if (!oc.isSet("dlr-navteq-output")) {
        return;
    }
    writeNodesUnsplitted(oc, nb.getNodeCont(), nb.getEdgeCont());
    writeLinksUnsplitted(oc, nb.getNodeCont(), nb.getEdgeCont());
}


void NWWriter_DlrNavteq::writeHeader(OutputDevice& device, const OptionsCont& oc) {
    time_t rawtime;
    time(&rawtime);
    char buffer [80];
    strftime(buffer, 80, "on %c", localtime(&rawtime));
    device << "# Created with SUMO-NETCONVERT " << buffer << "\n";
    std::stringstream tmp;
    oc.writeConfiguration(tmp, true, false, false);
    tmp.seekg(std::ios_base::beg);
    std::string line;
    while (!tmp.eof()) {
        std::getline(tmp, line);
        device << "# " << line << "\n";
    }
}

void
NWWriter_DlrNavteq::writeNodesUnsplitted(const OptionsCont& oc, NBNodeCont& nc, NBEdgeCont& ec) {
    // For "real" nodes we simply use the node id
    // for internal nodes (geometry vectors describing edge geometry in the parlance of this format) 
    // we use the id of the edge and do not bother with
    // compression (each direction gets its own internal node)
    // XXX add option for generating numerical ids in case the input network has string ids and the target process needs integers
    OutputDevice& device = OutputDevice::getDevice(oc.getString("dlr-navteq-output") + "_nodes_unsplitted.txt");
    writeHeader(device, oc);
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    const bool haveGeo = gch.usingGeoProjection();
    const SUMOReal geoScale = pow(10.0f, haveGeo ? 5 : 2); // see NIImporter_DlrNavteq::GEO_SCALE
    device.setPrecision(0);
    if (!haveGeo) {
        WRITE_WARNING("DlrNavteq node data will be written in (floating point) cartesian coordinates");
    }
    // write format specifier
    device << "#\n# NODE_ID   IS_BETWEEN_NODE amount_of_geocoordinates    x1  y1  [x2 y2  ... xn  yn]\n";
    // write normal nodes
    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        NBNode* n = (*i).second;
        Position pos = n->getPosition();
        gch.cartesian2geo(pos);
        pos.mul(geoScale);
        device << n->getID() << "\t0\t1\t" << pos.x() << "\t" << pos.y() << "\n";
    }
    // write "internal" nodes
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        NBEdge* e = (*i).second;
        const PositionVector& geom = e->getGeometry();
        if (geom.size() > 2) {
            device << e->getID() << "\t1\t" << geom.size() - 2;  
            for (size_t ii = 1; ii < geom.size() - 1; ++ii) {
                Position pos = geom[ii];
                gch.cartesian2geo(pos);
                pos.mul(geoScale);
                device << "\t" << pos.x() << "\t" << pos.y();
            }
            device << "\n";
        }
    }
    device.close();
}


void
NWWriter_DlrNavteq::writeLinksUnsplitted(const OptionsCont& oc, NBNodeCont& nc, NBEdgeCont& ec) {
}


/****************************************************************************/

