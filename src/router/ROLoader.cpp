/****************************************************************************/
/// @file    ROLoader.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Loader for networks and route imports
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

#include <iostream>
#include <string>
#include <iomanip>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/xml/WeightsHandler.h>
#include <utils/importio/LineReader.h>
#include "RONet.h"
#include "RONetHandler.h"
#include "ROLoader.h"
#include "RORDLoader_TripDefs.h"
#include "RORDLoader_SUMORoutes.h"
#include "RORDLoader_Cell.h"
#include "RORDLoader_SUMOAlt.h"
#include "RORDLoader_Artemis.h"
#include "RORDGenerator_Random.h"
#include "RORDGenerator_ODAmounts.h"
#include "ROAbstractRouteDefLoader.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// xerces 2.2 compatibility
// ===========================================================================
#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace XERCES_CPP_NAMESPACE;
#endif


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight - methods
// ---------------------------------------------------------------------------
ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight::EdgeFloatTimeLineRetriever_EdgeWeight(
    RONet *net)
        : myNet(net)
{}


ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight::~EdgeFloatTimeLineRetriever_EdgeWeight()
{}


void
ROLoader::EdgeFloatTimeLineRetriever_EdgeWeight::addEdgeWeight(const std::string &id,
        SUMOReal val,
        SUMOTime beg,
        SUMOTime end)
{
    ROEdge *e = myNet->getEdge(id);
    if (e!=0) {
        e->addWeight(val, beg, end);
    } else {
        MsgHandler::getErrorInstance()->inform("Trying to set a weight for the unknown edge '" + id + "'.");
    }
}



// ---------------------------------------------------------------------------
// ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever - methods
// ---------------------------------------------------------------------------
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever::SingleWeightRetriever(
    Type type, EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight *parent)
        : myType(type), myParent(parent)
{}


ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever::~SingleWeightRetriever()
{}


void
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever::addEdgeWeight(
    const std::string &id, SUMOReal val, SUMOTime beg, SUMOTime end)
{
    myParent->addTypedWeight(myType, id, val, beg, end);
}



// ---------------------------------------------------------------------------
// ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight - methods
// ---------------------------------------------------------------------------
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight(RONet *net)
        : myNet(net)
{
    myAbsoluteRetriever = new SingleWeightRetriever(ABSOLUTE, this);
    myAddRetriever = new SingleWeightRetriever(ADD, this);
    myMultRetriever = new SingleWeightRetriever(MULT, this);
}


ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::~EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight()
{
    std::map<ROEdge*, SuppWeights>::iterator i;
    for (i=myWeights.begin(); i!=myWeights.end(); ++i) {
        (*i).first->setSupplementaryWeights(
            (*i).second.absolute, (*i).second.add, (*i).second.mult);
    }
    delete myAbsoluteRetriever;
    delete myAddRetriever;
    delete myMultRetriever;
}


void
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::addTypedWeight(Type type, const std::string &id,
        SUMOReal val, SUMOTime beg, SUMOTime end)
{
    ROEdge *e = myNet->getEdge(id);
    if (e==0) {
        MsgHandler::getErrorInstance()->inform("Trying to set a weight for the unknown edge '" + id + "'.");
        return;
    }
    if (myWeights.find(e)==myWeights.end()) {
        SuppWeights weights;
        weights.absolute = new FloatValueTimeLine();
        weights.add = new FloatValueTimeLine();
        weights.mult = new FloatValueTimeLine();
    }
    switch (type) {
    case ABSOLUTE:
        myWeights[e].absolute->add(beg, end, val);
        break;
    case ADD:
        myWeights[e].add->add(beg, end, val);
        break;
    case MULT:
        myWeights[e].mult->add(beg, end, val);
        break;
    default:
        break;
    }
}


ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever &
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::getAbsoluteRetriever()
{
    return *myAbsoluteRetriever;
}


ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever &
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::getAddRetriever()
{
    return *myAddRetriever;
}


ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::SingleWeightRetriever &
ROLoader::EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight::getMultRetriever()
{
    return *myMultRetriever;
}



// ---------------------------------------------------------------------------
// ROLoader - methods
// ---------------------------------------------------------------------------
ROLoader::ROLoader(OptionsCont &oc, ROVehicleBuilder &vb,
                   bool emptyDestinationsAllowed)
        : _options(oc), myEmptyDestinationsAllowed(emptyDestinationsAllowed),
        myVehicleBuilder(vb)
{}


ROLoader::~ROLoader()
{
    for (RouteLoaderCont::iterator i=_handler.begin(); i!=_handler.end(); i++) {
        delete(*i);
    }
}


RONet *
ROLoader::loadNet(ROAbstractEdgeBuilder &eb)
{
    std::string file = _options.getString("net-file");
    if (file=="") {
        MsgHandler::getErrorInstance()->inform("Missing definition of network to load!");
        return 0;
    }
    if (!FileHelpers::exists(file)) {
        MsgHandler::getErrorInstance()->inform("The network file '" + file + "' could not be found.");
        return 0;
    }
    MsgHandler::getMessageInstance()->beginProcessMsg("Loading net...");
    RONet *net = new RONet(_options.isSet("sumo-input"));
    RONetHandler handler(_options, *net, eb);
    handler.setFileName(file);
    if(!XMLSubSys::runParser(handler, file)) {
        MsgHandler::getErrorInstance()->inform("failed.");
        delete net;
        return 0;
    } else {
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
    }
    // build and prepare the parser
    return net;
}


size_t
ROLoader::openRoutes(RONet &net, SUMOReal /*gBeta*/, SUMOReal /*gA*/)
{
    // build loader
    // load additional precomputed sumo-routes when wished
    openTypedRoutes("sumo-input", net);
    // load the XML-trip definitions when wished
    openTypedRoutes("trip-defs", net);
    // load the cell-routes when wished
    openTypedRoutes("cell-input", net);
    // load artemis routes when wished
    openTypedRoutes("artemis-input", net);
    // load the sumo-alternative file when wished
    openTypedRoutes("alternatives", net);
    // build generators
    // load the amount definitions if wished
    openTypedRoutes("flows", net);
    // check whether random routes shall be build, too
    if (_options.isSet("R")) {
        RORDGenerator_Random *randGen =
            new RORDGenerator_Random(myVehicleBuilder, net,
                                     _options.getInt("begin"), _options.getInt("end"),
                                     _options.getBool("prune-random"));
        randGen->init(_options);
        _handler.push_back(randGen);
    }
    if (!_options.getBool("unsorted")) {
        skipUntilBegin();
    }
    return _handler.size();
}

void
ROLoader::skipUntilBegin()
{
    MsgHandler::getMessageInstance()->inform("Skipping...");
    for (RouteLoaderCont::iterator i=_handler.begin(); i!=_handler.end(); i++) {
        (*i)->skipUntilBegin();
    }
    MsgHandler::getMessageInstance()->inform("Skipped until: " + toString<SUMOTime>(getMinTimeStep()));
}


void
ROLoader::processRoutesStepWise(SUMOTime start, SUMOTime end,
                                RONet &net, ROAbstractRouter &router)
{
    SUMOTime absNo = end - start;
    // skip routes that begin before the simulation's begin
    // loop till the end
    bool endReached = false;
    bool errorOccured = false;
    SUMOTime time = getMinTimeStep();
    SUMOTime firstStep = time;
    SUMOTime lastStep = time;
    for (; time<end&&!errorOccured&&!endReached; time++) {
        writeStats(time, start, absNo);
        makeSingleStep(time, net, router);
        // check whether further data exist
        endReached = !net.furtherStored();
        lastStep = time;
        for (RouteLoaderCont::iterator i=_handler.begin(); endReached&&i!=_handler.end(); i++) {
            if (!(*i)->ended()) {
                endReached = false;
            }
        }
        errorOccured =
            MsgHandler::getErrorInstance()->wasInformed()
            &&
            !_options.getBool("continue-on-unbuild");
    }
    time = end;
    writeStats(time, start, absNo);
    MsgHandler::getMessageInstance()->inform("Routes found between time steps " + toString<int>(firstStep) + " and " + toString<int>(lastStep) + ".");
}


bool
ROLoader::makeSingleStep(SUMOTime end, RONet &net, ROAbstractRouter &router)
{
    RouteLoaderCont::iterator i;
    // go through all handlers
    if (_handler.size()!= 0) {
        for (i=_handler.begin(); i!=_handler.end(); i++) {
            // load routes until the time point is reached
            (*i)->readRoutesAtLeastUntil(end);
            // save the routes
            net.saveAndRemoveRoutesUntil(_options, router, end);
        }
        return MsgHandler::getErrorInstance()->wasInformed();
    } else {
        return false;
    }
}


SUMOTime
ROLoader::getMinTimeStep() const
{
    SUMOTime ret = LONG_MAX;
    for (RouteLoaderCont::const_iterator i=_handler.begin(); i!=_handler.end(); i++) {
        SUMOTime akt = (*i)->getCurrentTimeStep();
        if (akt<ret) {
            ret = akt;
        }
    }
    return ret;
}


void
ROLoader::processAllRoutes(SUMOTime start, SUMOTime end,
                           RONet &net,
                           ROAbstractRouter &router)
{
    long absNo = end - start;
    bool ok = true;
    for (RouteLoaderCont::iterator i=_handler.begin(); ok&&i!=_handler.end(); i++) {
        (*i)->readRoutesAtLeastUntil(INT_MAX);
    }
    // save the routes
    SUMOTime time = start;
    for (; time<end; time++) {
        writeStats(time, start, absNo);
        net.saveAndRemoveRoutesUntil(_options, router, time);
    }
    writeStats(time, start, absNo);
}


void
ROLoader::closeReading()
{
    // close the reading
    for (RouteLoaderCont::iterator i=_handler.begin(); i!=_handler.end(); i++) {
        (*i)->closeReading();
    }
}


void
ROLoader::openTypedRoutes(const std::string &optionName,
                          RONet &net)
{
    // check whether the current loader is wished
    if (!_options.isSet(optionName)) {
        return;
    }
    // allocate a reader and add it to the list
    addToHandlerList(optionName, net);
}


void
ROLoader::addToHandlerList(const std::string &optionName,
                           RONet &net)
{
    vector<string> files = _options.getStringVector(optionName);
    for(vector<string>::const_iterator fileIt=files.begin(); fileIt!=files.end(); ++fileIt) {
        // check whether the file can be used
        //  necessary due to the extensions within cell-import
        checkFile(optionName, *fileIt);
        // build the instance when everything's all right
        ROAbstractRouteDefLoader *instance =
            buildNamedHandler(optionName, *fileIt, net);
        if (!instance->init(_options)) {
            delete instance;
            throw ProcessError("The loader for " + optionName + " from file '" + *fileIt + "' could not be initialised.");
        }
        _handler.push_back(instance);
    }
}


ROAbstractRouteDefLoader*
ROLoader::buildNamedHandler(const std::string &optionName,
                            const std::string &file,
                            RONet &net)
{
    if (optionName=="sumo-input") {
        return new RORDLoader_SUMORoutes(myVehicleBuilder, net,
                                         _options.getInt("begin"), _options.getInt("end"), file);
    }
    if (optionName=="trip-defs") {
        return new RORDLoader_TripDefs(myVehicleBuilder, net,
                                       _options.getInt("begin"), _options.getInt("end"),
                                       myEmptyDestinationsAllowed, file);
    }
    if (optionName=="cell-input") {
        return new RORDLoader_Cell(myVehicleBuilder, net,
                                   _options.getInt("begin"), _options.getInt("end"),
                                   _options.getFloat("gBeta"), _options.getFloat("gA"),
                                   _options.getInt("max-alternatives"), file);
    }
    if (optionName=="artemis-input") {
        return new RORDLoader_Artemis(myVehicleBuilder, net,
                                      _options.getInt("begin"), _options.getInt("end"), file);
    }
    if (optionName=="alternatives") {
        return new RORDLoader_SUMOAlt(myVehicleBuilder, net,
                                      _options.getInt("begin"), _options.getInt("end"),
                                      _options.getFloat("gBeta"), _options.getFloat("gA"),
                                      _options.getInt("max-alternatives"), file);
    }
    if (optionName=="flows") {
        return new RORDGenerator_ODAmounts(myVehicleBuilder, net,
                                           _options.getInt("begin"), _options.getInt("end"),
                                           myEmptyDestinationsAllowed, _options.getBool("randomize-flows"), file);
    }
    throw 1;
}


void
ROLoader::checkFile(const std::string &optionName,
                    const std::string &file)
{
    if (optionName=="sumo-input"&&FileHelpers::exists(file)) {
        return;
    }
    if (optionName=="trip-defs"&&FileHelpers::exists(file)) {
        return;
    }
    if (optionName=="cell-input"&&FileHelpers::exists(file+".driver")&&FileHelpers::exists(file+".rinfo")) {
        return;
    }
    if (optionName=="artemis-input"&&FileHelpers::exists(file + "/HVdests.txt")&&FileHelpers::exists(file + "/Flows.txt")) {
        return;
    }
    if (optionName=="alternatives"&&FileHelpers::exists(file)) {
        return;
    }
    if (optionName=="flows"&&FileHelpers::exists(file)) {
        return;
    }
    throw ProcessError("File '" + file + "' used as " + optionName + " not found.");
}


bool
ROLoader::loadWeights(RONet &net, const std::string &file,
                      bool useLanes)
{
    // check whether the file exists
    if (!FileHelpers::exists(file)) {
        MsgHandler::getErrorInstance()->inform("The weights file '" + file + "' does not exist!");
        return false;
    }
    // build and prepare the weights handler
    EdgeFloatTimeLineRetriever_EdgeWeight retriever(&net);
    WeightsHandler::ToRetrieveDefinition *def = new WeightsHandler::ToRetrieveDefinition("edge", "traveltime", !useLanes, retriever);
    WeightsHandler handler(def, file);
    MsgHandler::getMessageInstance()->beginProcessMsg("Loading precomputed net weights...");
    // build and prepare the parser
    if(XMLSubSys::runParser(handler, file)) {
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
        return true;
    } else {
        MsgHandler::getMessageInstance()->endProcessMsg("failed.");
        return false;
    }
}


void
ROLoader::loadSupplementaryWeights(RONet& net)
{
    string filename = _options.getString("supplementary-weights");
    if (! FileHelpers::exists(filename)) {
        throw ProcessError("Could not open the supplementary-weights file '" + filename + "'.");
    }
    EdgeFloatTimeLineRetriever_SupplementaryEdgeWeight retriever(&net);
    std::vector<WeightsHandler::ToRetrieveDefinition*> defs;
    defs.push_back(new WeightsHandler::ToRetrieveDefinition("edge", "absolute", true, retriever.getAbsoluteRetriever()));
    defs.push_back(new WeightsHandler::ToRetrieveDefinition("edge", "summand", true, retriever.getAddRetriever()));
    defs.push_back(new WeightsHandler::ToRetrieveDefinition("edge", "factor", true, retriever.getMultRetriever()));
    WeightsHandler handler(defs, filename);
    MsgHandler::getMessageInstance()->beginProcessMsg("Loading precomputed supplementary net-weights.");
    if(XMLSubSys::runParser(handler, filename)) {
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
    } else {
        MsgHandler::getMessageInstance()->endProcessMsg("failed.");
        throw ProcessError();
    }
}


void
ROLoader::writeStats(SUMOTime time, SUMOTime start, int absNo)
{
    if (_options.getBool("verbose")) {
        SUMOReal perc = (SUMOReal)(time-start) / (SUMOReal) absNo;
        cout.setf(ios::fixed , ios::floatfield) ;    // use decimal format
        cout.setf(ios::showpoint) ;    // print decimal point
        cout << setprecision(OUTPUT_ACCURACY);
        MsgHandler::getMessageInstance()->progressMsg("Reading time step: " + toString(time) + "  (" + toString(time-start) + "/" + toString(absNo) + " = " + toString(perc * 100) + "% done)       ");
    }
}



/****************************************************************************/

