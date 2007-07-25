/****************************************************************************/
/// @file    NLBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Container for MSNet during its building
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

#include "NLBuilder.h"
#include <microsim/MSNet.h>
#include <microsim/MSEmitControl.h>
#include <microsim/MSGlobals.h>
#include <iostream>
#include <vector>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <string>
#include <map>
#include "NLHandler.h"
#include "NLEdgeControlBuilder.h"
#include "NLJunctionControlBuilder.h"
#include "NLDetectorBuilder.h"
#include "NLTriggerBuilder.h"
#include <microsim/MSVehicleControl.h>
#include <microsim/MSVehicleTransfer.h>
#include <microsim/trigger/MSTriggerControl.h>
#include <microsim/output/meandata/MSMeanData_Net_Utils.h>
#include <microsim/MSRouteLoaderControl.h>
#include <microsim/MSRouteLoader.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/TplConvert.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <microsim/output/MSDetector2File.h>
#include <microsim/output/MSDetectorControl.h>
#include <sumo_only/SUMOFrame.h>
#include <utils/iodevices/BinaryInputDevice.h>
#include "NLGeomShapeBuilder.h"

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
// NLBuilder::EdgeFloatTimeLineRetriever_EdgeWeight - methods
// ---------------------------------------------------------------------------
NLBuilder::EdgeFloatTimeLineRetriever_EdgeWeight::EdgeFloatTimeLineRetriever_EdgeWeight(
    MSNet *net)
        : myNet(net)
{}


NLBuilder::EdgeFloatTimeLineRetriever_EdgeWeight::~EdgeFloatTimeLineRetriever_EdgeWeight()
{}


void
NLBuilder::EdgeFloatTimeLineRetriever_EdgeWeight::addEdgeWeight(const std::string &id,
        SUMOReal val,
        SUMOTime beg,
        SUMOTime end)
{
    MSEdge *e = MSEdge::dictionary(id);
    if (e!=0) {
        e->addWeight(val, beg, end);
    } else {
        MsgHandler::getErrorInstance()->inform("Trying to set a weight for the unknown edge '" + id + "'.");
    }
}



// ---------------------------------------------------------------------------
// NLBuilder - methods
// ---------------------------------------------------------------------------
NLBuilder::NLBuilder(const OptionsCont &oc,
                     MSNet &net,
                     NLEdgeControlBuilder &eb,
                     NLJunctionControlBuilder &jb,
                     NLDetectorBuilder &db,
                     NLTriggerBuilder &tb,
                     NLGeomShapeBuilder &sb,
                     NLHandler &xmlHandler)
        : m_pOptions(oc), myEdgeBuilder(eb), myJunctionBuilder(jb),
        myDetectorBuilder(db), myTriggerBuilder(tb), myShapeBuilder(sb),
        myNet(net), myXMLHandler(xmlHandler)
{}


NLBuilder::~NLBuilder()
{}



bool
NLBuilder::build()
{
    SAX2XMLReader* parser = XMLSubSys::getSAXReader(myXMLHandler);
    // try to build the net
    if(!load("net-file", LOADFILTER_ALL, *parser)) {
        delete parser;
        return false;
    }
    buildNet();
    // load the previous state if wished
    if (m_pOptions.isSet("load-state")) {
        BinaryInputDevice strm(m_pOptions.getString("load-state"));
        if (!strm.good()) {
            delete parser;
            MsgHandler::getErrorInstance()->inform("Could not read state from '" + m_pOptions.getString("load-state") + "'!");
            return false;
        } else {
            MsgHandler::getMessageInstance()->beginProcessMsg("Loading state from '" + m_pOptions.getString("load-state") + "'...");
            myNet.loadState(strm, (long) 0xfffffff);
            MsgHandler::getMessageInstance()->endProcessMsg("done.");
        }
    }
    // load weights if wished
    if (m_pOptions.isSet("weight-files")) {
        if (!m_pOptions.isUsableFileList("weight-files")) {
            delete parser;
            return false;
        }
        // start parsing; for each file in the list
        StringTokenizer st(m_pOptions.getString("weight-files"), ';');
        while (st.hasNext()) {
            string tmp = st.next();
            // report about loading when wished
            WRITE_MESSAGE("Loading weights from '" + tmp + "'...");
            // check whether the file exists
            if (!FileHelpers::exists(tmp)) {
                // report error if not
                MsgHandler::getErrorInstance()->inform("The weights file '" + tmp + "' does not exist!");
                return false;
            } else {
                EdgeFloatTimeLineRetriever_EdgeWeight retriever(&myNet);
                SAXWeightsHandler::ToRetrieveDefinition *def = new SAXWeightsHandler::ToRetrieveDefinition("traveltime", true, retriever);
                SAXWeightsHandler wh(def, tmp);
                // parse the file
                if(!XMLSubSys::runParser(wh, tmp)) {
                    return false;
                }
            }
        }
    }
    // load routes
    if (m_pOptions.isSet("route-files")&&m_pOptions.getInt("route-steps")<=0) {
        if(!load("route-files", LOADFILTER_DYNAMIC, *parser)) {
            delete parser;
            return false;
        }
    }
    // load additional net elements (sources, detectors, ...)
    if (m_pOptions.isSet("additional-files")) {
        if(!load("additional-files",
                  (NLLoadFilter)((int) LOADFILTER_NETADD|(int) LOADFILTER_DYNAMIC),
                  *parser)) {
            delete parser;
            return false;
        }
    }
    delete parser;
    WRITE_MESSAGE("Loading done.");
    return true;
}


void
NLBuilder::buildNet()
{
    myJunctionBuilder.closeJunctions(myDetectorBuilder, myXMLHandler.getContinuations());
    MSEdgeControl *edges = myEdgeBuilder.build();
    std::vector<OutputDevice*> streams = SUMOFrame::buildStreams(m_pOptions);
    MSMeanData_Net_Cont meanData = MSMeanData_Net_Utils::buildList(
                                       myNet.getDetectorControl().getDet2File(), *edges,
                                       m_pOptions.getIntVector("dump-intervals"),
                                       m_pOptions.getString("dump-basename"),
                                       m_pOptions.getIntVector("lanedump-intervals"),
                                       m_pOptions.getString("lanedump-basename"),
                                       m_pOptions.getIntVector("dump-begins"), m_pOptions.getIntVector("dump-ends"));
    myNet.closeBuilding(
        edges,
        myJunctionBuilder.build(),
        buildRouteLoaderControl(m_pOptions),
        myJunctionBuilder.buildTLLogics(),
        streams, meanData,
        m_pOptions.getIntVector("save-state.times"),
        m_pOptions.getString("save-state.prefix"));
}

bool
NLBuilder::load(const std::string &mmlWhat,
                NLLoadFilter what,
                SAX2XMLReader &parser)
{
    // initialise the handler for the current type of data
    myXMLHandler.setWanted(what);
    // start parsing
    parser.setContentHandler(&myXMLHandler);
    parser.setErrorHandler(&myXMLHandler);
    parse(mmlWhat, parser);
    // report about loaded structures
    if(MsgHandler::getErrorInstance()->wasInformed()) {
        WRITE_MESSAGE("Loading of " + mmlWhat + " failed.");
        return false;
    } else {
        WRITE_MESSAGE("Loading of " + mmlWhat + " done.");
        return true;
    }
}


bool
NLBuilder::parse(const std::string &mmlWhat,
                 SAX2XMLReader &parser)
{
    if (!OptionsCont::getOptions().isUsableFileList(mmlWhat)) {
        return false;
    }
    vector<string> files = OptionsCont::getOptions().getStringVector(mmlWhat);
    for(vector<string>::const_iterator fileIt=files.begin(); fileIt!=files.end(); ++fileIt) {
        WRITE_MESSAGE("Loading " + mmlWhat + " from '" + *fileIt + "'...");
        myXMLHandler.setFileName(*fileIt);
        parser.parse(fileIt->c_str());
        if (MsgHandler::getErrorInstance()->wasInformed()) {
            return false;
        }
    }
    return true;
}


MSRouteLoaderControl *
NLBuilder::buildRouteLoaderControl(const OptionsCont &oc)
{
    // build the loaders
    MSRouteLoaderControl::LoaderVector loaders;
    // check whether a list is existing
    if (oc.isSet("route-files")&&oc.getInt("route-steps")>0) {
        vector<string> files = oc.getStringVector("route-files");
        for(vector<string>::const_iterator fileIt=files.begin(); fileIt!=files.end(); ++fileIt) {
            if (!FileHelpers::exists(*fileIt)) {
                throw ProcessError("The route file '" + *fileIt + "' does not exist.");
            }
        }
        // open files for reading
        for(vector<string>::const_iterator fileIt=files.begin(); fileIt!=files.end(); ++fileIt) {
            loaders.push_back(myNet.buildRouteLoader(*fileIt, oc.getInt("incremental-dua-base"), oc.getInt("incremental-dua-step")));
        }
    }
    // build the route control
    return new MSRouteLoaderControl(myNet, oc.getInt("route-steps"), loaders);
}



/****************************************************************************/

