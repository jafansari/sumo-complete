/****************************************************************************/
/// @file    NBNetBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    20 Nov 2001
/// @version $Id: $
///
// The instance responsible for building networks
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
#include <fstream>
#include <iostream>
#include <iomanip>
#include "NBNetBuilder.h"
#include "nodes/NBNodeCont.h"
#include "NBEdgeCont.h"
#include "NBTrafficLightLogicCont.h"
#include "NBJunctionLogicCont.h"
#include "NBDistrictCont.h"
#include "NBDistribution.h"
#include "NBRequest.h"
#include "NBTypeCont.h"
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/ToString.h>
#include "NBJoinedEdgesMap.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
NBNetBuilder::NBNetBuilder()
        : myHaveBuildNet(false), myTypeCont()
{}


NBNetBuilder::~NBNetBuilder()
{}


void
NBNetBuilder::buildLoaded()
{
    myTypeCont.report();
    myEdgeCont.report();
    myNodeCont.report();
    // perform the computation
    OptionsCont &oc = OptionsSubSys::getOptions();
    compute(oc);
    // save network
    bool ok = true;
    if (!oc.getBool("binary-output")) {
        ofstream res(oc.getString("o").c_str());
        if (res.good()) {
            save(res, oc);
        } else {
            ok = false;
        }
    } else {}
    if (!ok) {
        MsgHandler::getErrorInstance()->inform("Could not save net to '" + oc.getString("o") + "'.");
        throw ProcessError();
    }
    // save the mapping information when wished
    if (oc.isSet("map-output")) {
        saveMap(oc.getString("map-output"));
    }
    // save the tls positions as a list of pois
    if (oc.isSet("tls-poi-output")) {
        myNodeCont.writeTLSasPOIs(oc.getString("tls-poi-output"));
    }
}



void
NBNetBuilder::inform(int &step, const std::string &about)
{
    WRITE_MESSAGE("Computing step " + toString<int>(step)+ ": " + about);
    step++;
}


bool
NBNetBuilder::removeDummyEdges(int &step)
{
    // Removes edges that are connecting the same node
    inform(step, "Removing dummy edges");
    return myNodeCont.removeDummyEdges(myDistrictCont, myEdgeCont, myTLLCont);
}


bool
NBNetBuilder::joinEdges(int &step)
{
    inform(step, "Joining double connections");
    return myNodeCont.recheckEdges(myDistrictCont, myTLLCont, myEdgeCont);
}


bool
NBNetBuilder::removeUnwishedNodes(int &step, OptionsCont &oc)
{
    if (!oc.getBool("remove-geometry")) {
        return true;
    }
    inform(step, "Removing empty nodes and geometry nodes.");
    return myNodeCont.removeUnwishedNodes(myDistrictCont, myEdgeCont, myTLLCont);
}


bool
NBNetBuilder::splitGeometry(int &step, OptionsCont &oc)
{
    if (!oc.getBool("split-geometry")) {
        return true;
    }
    inform(step, "Splitting geometry edges.");
    return myEdgeCont.splitGeometry(myNodeCont);
}


bool
NBNetBuilder::removeUnwishedEdges(int &step, OptionsCont &oc)
{
    if (!OptionsSubSys::getOptions().isSet("keep-edges")) {
        return true;
    }
    inform(step, "Removing unwished edges.");
    return myEdgeCont.removeUnwishedEdges(myDistrictCont, oc);
}


bool
NBNetBuilder::guessRamps(int &step, OptionsCont &oc)
{
    if (!oc.getBool("guess-ramps")&&!oc.getBool("guess-obscure-ramps")) {
        return true;
    }
    inform(step, "Guessing and setting on-/off-ramps");
    return myNodeCont.guessRamps(oc, myEdgeCont, myDistrictCont);
}


bool
NBNetBuilder::guessTLs(int &step, OptionsCont &oc)
{
    inform(step, "Guessing and setting TLs");
    if (oc.isSet("explicite-tls")) {
        StringTokenizer st(oc.getString("explicite-tls"), ";");
        while (st.hasNext()) {
            myNodeCont.setAsTLControlled(st.next(), myTLLCont);
        }
    }
    return myNodeCont.guessTLs(oc, myTLLCont);
}


bool
NBNetBuilder::computeTurningDirections(int &step)
{
    inform(step, "Computing turning directions");
    return myEdgeCont.computeTurningDirections();
}


bool
NBNetBuilder::sortNodesEdges(int &step, ofstream *strm)
{
    inform(step, "Sorting nodes' edges");
    return myNodeCont.sortNodesEdges(myTypeCont, strm);
}


bool
NBNetBuilder::normaliseNodePositions(int &step)
{
    inform(step, "Normalising node positions");
    bool ok = myNodeCont.normaliseNodePositions();
    if (ok) {
        ok = myEdgeCont.normaliseEdgePositions(myNodeCont);
    }
    return ok;
}


bool
NBNetBuilder::computeEdge2Edges(int &step)
{
    inform(step, "Computing Approached Edges");
    return myEdgeCont.computeEdge2Edges();
}


bool
NBNetBuilder::computeLanes2Edges(int &step)
{
    inform(step, "Computing Approaching Lanes");
    return myEdgeCont.computeLanes2Edges();
}


bool
NBNetBuilder::computeLanes2Lanes(int &step)
{
    inform(step, "Dividing of Lanes on Approached Lanes");
    bool ok = myNodeCont.computeLanes2Lanes();
    if (ok) {
        return myEdgeCont.sortOutgoingLanesConnections();
    }
    return ok;
}

bool
NBNetBuilder::recheckLanes(int &step)
{
    inform(step, "Rechecking of lane endings");
    return myEdgeCont.recheckLanes();
}


bool
NBNetBuilder::computeNodeShapes(int &step, OptionsCont &oc)
{
    inform(step, "Computing node shapes");
    return myNodeCont.computeNodeShapes(oc);
}


bool
NBNetBuilder::computeEdgeShapes(int &step)
{
    inform(step, "Computing edge shapes");
    return myEdgeCont.computeEdgeShapes();
}


bool
NBNetBuilder::appendTurnarounds(int &step, OptionsCont &oc)
{
    if (oc.getBool("no-turnarounds")) {
        return true;
    }
    inform(step, "Appending Turnarounds");
    return myEdgeCont.appendTurnarounds();
}


bool
NBNetBuilder::setTLControllingInformation(int &step)
{
    inform(step, "Computing tls logics");
    return myTLLCont.setTLControllingInformation(myEdgeCont);
}


bool
NBNetBuilder::computeLogic(int &step, OptionsCont &oc)
{
    inform(step, "Computing node logics");
    return myNodeCont.computeLogics(myEdgeCont, myJunctionLogicCont, oc);
}


bool
NBNetBuilder::computeTLLogic(int &step, OptionsCont &oc)
{
    inform(step, "Computing traffic light logics");
    return myTLLCont.computeLogics(myEdgeCont, oc);
}


bool
NBNetBuilder::reshiftRotateNet(int &step, OptionsCont &oc)
{
    if (oc.isDefault("x-offset-to-apply")) {
        return true;
    }
    inform(step, "Transposing network");
    SUMOReal xoff = oc.getFloat("x-offset-to-apply");
    SUMOReal yoff = oc.getFloat("y-offset-to-apply");
    SUMOReal rot = oc.getFloat("rotation-to-apply");
    inform(step, "Normalising node positions");
    bool ok = myNodeCont.reshiftNodePositions(xoff, yoff, rot);
    if (ok) {
        ok = myEdgeCont.reshiftEdgePositions(xoff, yoff, rot);
    }
    return ok;
}


void
NBNetBuilder::compute(OptionsCont &oc)
{
    if (myHaveBuildNet) {
        return;
    }
    bool ok = true;
    int step = 1;
    //
    if (ok) ok = removeDummyEdges(step);
    gJoinedEdges.init(myEdgeCont);
    if (ok) ok = joinEdges(step);
    if (ok) ok = removeUnwishedNodes(step, oc);
    if (ok&&oc.getBool("keep-edges.postload")) {
        if (ok) ok = removeUnwishedEdges(step, oc);
        if (ok) ok = removeUnwishedNodes(step, oc);
    }
    if (ok) ok = splitGeometry(step, oc);
    if (ok&&!oc.getBool("disable-normalize-node-positions")) ok = normaliseNodePositions(step);
    if (ok) ok = myEdgeCont.recomputeLaneShapes();
    if (ok) ok = guessRamps(step, oc);
    if (ok) ok = guessTLs(step, oc);
    if (ok) ok = computeTurningDirections(step);

    ofstream *strm = 0;
    if (oc.isSet("node-type-output")) {
        strm = new ofstream(oc.getString("node-type-output").c_str());
        (*strm) << "<pois>" << endl;
    }
    if (ok) ok = sortNodesEdges(step, strm);
    if (strm!=0) {
        (*strm) << "</pois>" << endl;
        delete strm;
    }

    if (ok) ok = computeEdge2Edges(step);
    if (ok) ok = computeLanes2Edges(step);
    if (ok) ok = computeLanes2Lanes(step);
    if (ok) ok = appendTurnarounds(step, oc);
    if (ok) ok = recheckLanes(step);
    // save plain nodes/edges/connections
    if (oc.isSet("plain-output")) {
        myNodeCont.savePlain(oc.getString("plain-output") + ".nod.xml");
        myEdgeCont.savePlain(oc.getString("plain-output") + ".edg.xml");
    }
    if (ok) ok = computeNodeShapes(step, oc);
    if (ok) ok = computeEdgeShapes(step);
//    if(ok) ok = computeLinkPriorities(step++);
    if (ok) ok = setTLControllingInformation(step);
    if (ok) ok = computeLogic(step, oc);
    if (ok) ok = computeTLLogic(step, oc);

    if (ok) ok = reshiftRotateNet(step, oc);

    NBNode::reportBuild();
    NBRequest::reportWarnings();
    checkPrint(oc);
    if (!ok) {
        throw ProcessError();
    }
    myHaveBuildNet = true;
}


void
NBNetBuilder::checkPrint(OptionsCont &oc)
{
    if (oc.getBool("print-node-positions")) {
        myNodeCont.printNodePositions();
    }
}


bool
NBNetBuilder::save(ostream &res, OptionsCont &oc)
{
    if (!oc.getBool("binary-output")) {
        // print the computed values
        res << "<net>" << endl << endl;
        res.setf(ios::fixed , ios::floatfield);
        // write network offsets
        res << "   <net-offset>" << myNodeCont.getNetworkOffset() << "</net-offset>" << endl;
        res << "   <conv-boundary>" << myNodeCont.getConvBoundary() << "</conv-boundary>" << endl;
        res << "   <orig-boundary>" << myNodeCont.getOrigBoundary() << "</orig-boundary>" << endl;
        if (!oc.getBool("use-projection")) {
            res << "   <orig-proj>!</orig-proj>" << endl;
        } else if (oc.getBool("proj.simple")) {
            res << "   <orig-proj>-</orig-proj>" << endl;
        } else {
            res << "   <orig-proj>" << oc.getString("proj") << "</orig-proj>" << endl;
        }
        res << endl;
        res << setprecision(2);

        // write the numbers of some elements
        // edges
        std::vector<std::string> ids;
        if (oc.getBool("add-internal-links")) {
            ids = myNodeCont.getInternalNamesList();
        }
        myEdgeCont.writeXMLEdgeList(res, ids);
        if (oc.getBool("add-internal-links")) {
            myNodeCont.writeXMLInternalLinks(res);
        }

        // write the number of nodes
        myNodeCont.writeXMLNumber(res);
        res << endl;
        // write the districts
        myDistrictCont.writeXML(res);
        // write edges with lanes and connected edges
        myEdgeCont.writeXMLStep1(res);
        // write the logics
        myJunctionLogicCont.writeXML(res);
        myTLLCont.writeXML(res);
        // write the nodes
        myNodeCont.writeXML(res);
        // write internal nodes
        if (oc.getBool("add-internal-links")) {
            myNodeCont.writeXMLInternalNodes(res);
        }
        // write the successors of lanes
        myEdgeCont.writeXMLStep2(res, oc.getBool("add-internal-links"));
        if (oc.getBool("add-internal-links")) {
            myNodeCont.writeXMLInternalSuccInfos(res);
        }
        res << "</net>" << endl;
    } else {
//        res << setmode(os::binary);
    }
    return true;
}


bool
NBNetBuilder::saveMap(const string &path)
{
    // try to build the output file
    ofstream res(path.c_str());
    if (!res.good()) {
        MsgHandler::getErrorInstance()->inform("Map output '" + path + "' could not be opened.");
        return false;
    }
    // write map
    res << gJoinedEdges;
    return true;
}


void
NBNetBuilder::insertNetBuildOptions(OptionsCont &oc)
{
    // register additional output options
    oc.doRegister("output-file", 'o', new Option_FileName("net.net.xml"));
    oc.addSynonyme("output-file", "output");
    oc.addDescription("output-file", "Output", "The generated net will be written to FILE");

    oc.doRegister("binary-output", new Option_Bool(false));
    oc.addDescription("binary-output", "Output", "");

    oc.doRegister("plain-output", new Option_FileName());
    oc.addDescription("plain-output", "Output", "Prefix of files to write nodes and edges to");

    oc.doRegister("node-geometry-dump", new Option_FileName());
    oc.addDescription("node-geometry-dump", "Output", "Writes node corner positions to FILE");

    oc.doRegister("map-output", 'M', new Option_FileName());
    oc.addDescription("map-output", "Output", "Writes joined edges information to FILE");

    oc.doRegister("tls-poi-output", new Option_FileName());
    oc.addDescription("tls-poi-output", "Output", "Writes pois of tls positions to FILE");

    oc.doRegister("node-type-output", new Option_FileName());
    oc.addDescription("node-type-output", "Output", "Writes pois of node types to FILE"); // !!! describe, rename



    // register building defaults
    oc.doRegister("type", 'T', new Option_String("Unknown"));
    oc.addDescription("type", "Building Defaults", "The default name for an edges type");

    oc.doRegister("lanenumber", 'L', new Option_Integer(1));
    oc.addDescription("lanenumber", "Building Defaults", "The default number of lanes in an edge");

    oc.doRegister("speed", 'S', new Option_Float((SUMOReal) 13.9));
    oc.addDescription("speed", "Building Defaults", "The default speed on an edge (in m/s)");

    oc.doRegister("priority", 'P', new Option_Integer(-1));
    oc.addDescription("priority", "Building Defaults", "The default priority of an edge");


    // projection options
    oc.doRegister("use-projection", new Option_Bool(false));
    oc.addDescription("use-projection", "Projection", "Enables reprojection from geo to cartesian");

    oc.doRegister("proj.simple", new Option_Bool(false));
    oc.addDescription("proj.simple", "Projection", "Uses a simple method for projection");

    oc.doRegister("proj", new Option_String("+proj=utm +zone=33 +ellps=bessel +units=m"));
    oc.addDescription("proj", "Projection", "Uses STR as proj.4 definition for projection");


    // register the data processing options
    oc.doRegister("omit-corrupt-edges", new Option_Bool(false));
    oc.addDescription("omit-corrupt-edges", "Processing", "Continues though corrupt edges");

    oc.doRegister("flip-y", new Option_Bool(false));
    oc.addDescription("flip-y", "Processing", "Flips the y-coordinate along zero");

    oc.doRegister("dismiss-vclasses", new Option_Bool(false));
    oc.addDescription("dismiss-vclasses", "Processing", "");

    oc.doRegister("remove-geometry", 'R', new Option_Bool(false));
    oc.addDescription("remove-geometry", "Processing", "Removes geometry information from edges");

    oc.doRegister("no-turnarounds", new Option_Bool(false));
    oc.addDescription("no-turnarounds", "Processing", "Disables building turnarounds");

    oc.doRegister("add-internal-links", 'I', new Option_Bool(false)); // !!! not described
    oc.addDescription("add-internal-links", "Processing", "Adds internal links");

    oc.doRegister("split-geometry", new Option_Bool(false)); // !!!not described
    oc.addDescription("split-geometry", "Processing", "Splits edges across geometry nodes");

    oc.doRegister("disable-normalize-node-positions", new Option_Bool(false));
    oc.addDescription("disable-normalize-node-positions", "Processing", "Turn off normalizing node positions");

    oc.doRegister("x-offset-to-apply", new Option_Float(0));
    oc.addDescription("x-offset-to-apply", "Processing", "Adds FLOAT to net x-positions");

    oc.doRegister("y-offset-to-apply", new Option_Float(0));
    oc.addDescription("y-offset-to-apply", "Processing", "Adds FLOAT to net y-positions");

    oc.doRegister("rotation-to-apply", new Option_Float(0));
    oc.addDescription("rotation-to-apply", "Processing", "Rotates net around FLOAT degrees");


    // tls setting options
    // explicite tls
    oc.doRegister("explicite-tls", new Option_String());
    oc.addDescription("explicite-tls", "TLS Building", "Interprets STR as list of junctions to be controlled by TLS");

    oc.doRegister("explicite-no-tls", new Option_String());
    oc.addDescription("explicite-no-tls", "TLS Building", "Interprets STR as list of junctions to be not controlled by TLS");

    // tls-guessing
    oc.doRegister("guess-tls", new Option_Bool(false));
    oc.addDescription("guess-tls", "TLS Building", "Turns on TLS guessing");

    oc.doRegister("tls-guess.no-incoming-min", new Option_Integer(2));
    oc.addDescription("tls-guess.no-incoming-min", "TLS Building", "");

    oc.doRegister("tls-guess.no-incoming-max", new Option_Integer(5));
    oc.addDescription("tls-guess.no-incoming-max", "TLS Building", "");

    oc.doRegister("tls-guess.no-outgoing-min", new Option_Integer(1));
    oc.addDescription("tls-guess.no-outgoing-min", "TLS Building", "");

    oc.doRegister("tls-guess.no-outgoing-max", new Option_Integer(5));
    oc.addDescription("tls-guess.no-outgoing-max", "TLS Building", "Min/max of incoming/outgoing edges a junction may have in order to be tls-controlled.");

    oc.doRegister("tls-guess.min-incoming-speed", new Option_Float((SUMOReal)(40/3.6)));
    oc.addDescription("tls-guess.min-incoming-speed", "TLS Building", "");

    oc.doRegister("tls-guess.max-incoming-speed", new Option_Float((SUMOReal)(69/3.6)));
    oc.addDescription("tls-guess.max-incoming-speed", "TLS Building", "");

    oc.doRegister("tls-guess.min-outgoing-speed", new Option_Float((SUMOReal)(40/3.6)));
    oc.addDescription("tls-guess.min-outgoing-speed", "TLS Building", "");

    oc.doRegister("tls-guess.max-outgoing-speed", new Option_Float((SUMOReal)(69/3.6)));
    oc.addDescription("tls-guess.max-outgoing-speed", "TLS Building", "Min/max speeds that incoming/outgoing edges must allowed in order to make their junction TLS-controlled.");

    oc.doRegister("tls-guess.district-nodes", new Option_Bool(false));
    oc.addDescription("tls-guess.district-nodes", "TLS Building", ""); // !!! describe

    // computational
    oc.doRegister("min-decel", 'D', new Option_Float(3.0));
    oc.addDescription("min-decel", "TLS Building", "Defines smallest vehicle deceleration");

    oc.doRegister("all-logics", new Option_Bool(false));
    oc.addDescription("all-logics", "TLS Building", "");

    oc.doRegister("keep-small-tyellow", new Option_Bool(false));
    oc.addDescription("keep-small-tyellow", "TLS Building", "Given yellow times are kept even if being too short");

    oc.doRegister("traffic-light-green", new Option_Integer());
    oc.addDescription("traffic-light-green", "TLS Building", "Use INT as green phase duration");

    oc.doRegister("traffic-light-yellow", new Option_Integer());
    oc.addDescription("traffic-light-yellow", "TLS Building", "Set INT as fixed time for yellow phase durations");

    // tls-shifts
    oc.doRegister("tl-logics.half-offset", new Option_String());
    oc.addDescription("tl-logics.half-offset", "TLS Building", "TLSs in STR will be shifted by half-phase");

    oc.doRegister("tl-logics.quarter-offset", new Option_String());
    oc.addDescription("tl-logics.quarter-offset", "TLS Building", "TLSs in STR will be shifted by quarter-phase");


    // edge constraints
    oc.doRegister("edges-min-speed", new Option_Float());
    oc.addDescription("edges-min-speed", "Edge Removal", "Remove edges with speed < FLOAT");

    oc.doRegister("remove-edges", new Option_String());
    oc.addDescription("remove-edges", "Edge Removal", "Remove edges in STR");

    oc.doRegister("keep-edges", new Option_String());
    oc.addDescription("keep-edges", "Edge Removal", "Remove edges not in STR");

    oc.doRegister("keep-edges.input-file", new Option_FileName());
    oc.addDescription("keep-edges.input-file", "Edge Removal", "Removed edges not in FILE");

    oc.doRegister("keep-edges.postload", new Option_Bool(false));
    oc.addDescription("keep-edges.postload", "Edge Removal", "Remove edges after joining");

    oc.doRegister("remove-edges.by-type", new Option_String());
    oc.addDescription("remove-edges.by-type", "Edge Removal", "Remove edges where vclass def is not in STR");


    // unregulated nodes options
    oc.doRegister("keep-unregulated", new Option_Bool(false));
    oc.addDescription("keep-unregulated", "Unregulated Nodes", "All nodes will be not regulated");

    oc.doRegister("keep-unregulated.nodes", new Option_String());
    oc.addDescription("keep-unregulated.nodes", "Unregulated Nodes", "Do not regulate nodes in STR");

    oc.doRegister("keep-unregulated.district-nodes", new Option_Bool(false));
    oc.addDescription("keep-unregulated.district-nodes", "Unregulated Nodes", "Do not regulate district nodes");


    // ramp guessing options
    oc.doRegister("guess-ramps", new Option_Bool(false));
    oc.addDescription("guess-ramps", "Ramp Guessing", "Enable ramp-guessing");

    oc.doRegister("ramp-guess.max-ramp-speed", new Option_Float(-1));
    oc.addDescription("ramp-guess.max-ramp-speed", "Ramp Guessing", "Treat edges with speed > FLOAT as no ramps");

    oc.doRegister("ramp-guess.min-highway-speed", new Option_Float((SUMOReal)(79/3.6)));
    oc.addDescription("ramp-guess.min-highway-speed", "Ramp Guessing", "Treat edges with speed < FLOAT as no highways");

    oc.doRegister("ramp-guess.ramp-length", new Option_Float(100));
    oc.addDescription("ramp-guess.ramp-length", "Ramp Guessing", "Use FLOAT as ramp-length");

    oc.doRegister("guess-obscure-ramps", new Option_Bool(false)); // !!! not described
    oc.addDescription("guess-obscure-ramps", "Ramp Guessing", "");

    oc.doRegister("obscure-ramps.add-ramp", new Option_Bool(false)); // !!! not described
    oc.addDescription("obscure-ramps.add-ramp", "Ramp Guessing", "");

    oc.doRegister("obscure-ramps.min-highway-speed", new Option_Float((SUMOReal)(100/3.6)));  // !!! not described
    oc.addDescription("obscure-ramps.min-highway-speed", "Ramp Guessing", "");


    // register report options
    oc.doRegister("verbose", 'v', new Option_Bool(false));
    oc.addDescription("verbose", "Report", "Switches to verbose output");

    oc.doRegister("suppress-warnings", 'W', new Option_Bool(false));
    oc.addDescription("suppress-warnings", "Report", "Disables output of warnings");

    oc.doRegister("print-options", 'p', new Option_Bool(false));
    oc.addDescription("print-options", "Report", "Prints option values before processing");

    oc.doRegister("help", '?', new Option_Bool(false));
    oc.addDescription("help", "Report", "Prints this screen");

    oc.doRegister("log-file", 'l', new Option_FileName());
    oc.addDescription("log-file", "Report", "Writes all messages to FILE");

    oc.doRegister("print-node-positions", new Option_Bool(false)); // !!! not described
    oc.addDescription("print-node-positions", "Report", "Prints the positions of read nodes");

    oc.doRegister("netbuild.debug", new Option_Integer(0)); // !!! not described
    oc.addDescription("netbuild.debug", "Report", "Enable debug mode");

}



void
NBNetBuilder::preCheckOptions(OptionsCont &oc)
{
    // we possibly have to load the edges to keep
    if (oc.isSet("keep-edges.input-file")) {
        ifstream strm(oc.getString("keep-edges.input-file").c_str());
        if (!strm.good()) {
            MsgHandler::getErrorInstance()->inform("Could not load names of edges too keep from '" + oc.getString("keep-edges.input-file") + "'.");
            throw ProcessError();
        }
        std::ostringstream oss;
        bool first = true;
        while (strm.good()) {
            if (!first) {
                oss << ';';
            }
            string name;
            strm >> name;
            oss << name;
            first = false;
        }
        oc.set("keep-edges", oss.str());
    }
    // check whether at least one output file is given
    if (!oc.isSet("o")&&!oc.isSet("plain-output")&&!oc.isSet("map-output")) {
        MsgHandler::getErrorInstance()->inform("No output defined.");
        throw ProcessError();
    }
}


bool
NBNetBuilder::netBuild() const
{
    return myHaveBuildNet;
}


NBEdgeCont &
NBNetBuilder::getEdgeCont()
{
    return myEdgeCont;
}


NBNodeCont &
NBNetBuilder::getNodeCont()
{
    return myNodeCont;
}


NBTypeCont &
NBNetBuilder::getTypeCont()
{
    return myTypeCont;
}


NBTrafficLightLogicCont &
NBNetBuilder::getTLLogicCont()
{
    return myTLLCont;
}


NBJunctionLogicCont &
NBNetBuilder::getJunctionLogicCont()
{
    return myJunctionLogicCont;
}


NBDistrictCont &
NBNetBuilder::getDistrictCont()
{
    return myDistrictCont;
}



/****************************************************************************/

