/****************************************************************************/
/// @file    NLTriggerBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 17 Oct 2002
/// @version $Id$
///
// Builds trigger objects for microsim
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
#include <microsim/MSEventControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <microsim/trigger/MSLaneSpeedTrigger.h>
#include <microsim/trigger/MSEmitter.h>
#include <microsim/trigger/MSTriggeredRerouter.h>
#include <microsim/trigger/MSBusStop.h>
#include <microsim/trigger/MSE1VehicleActor.h>
#include <microsim/MSPhoneCell.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/UtilExceptions.h>
#include "NLHandler.h"
#include "NLTriggerBuilder.h"
#include <utils/xml/SUMOXMLDefinitions.h>
#include <microsim/trigger/MSCalibrator.h>


#ifdef HAVE_MESOSIM
#include <mesosim/METriggeredCalibrator.h>
#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>
#endif

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
NLTriggerBuilder::NLTriggerBuilder() throw()
        : myHaveInformedAboutDeprecatedTriggerDefinition(false),
        myHaveInformedAboutDeprecatedEmitter(false)
{}


NLTriggerBuilder::~NLTriggerBuilder() throw()
{}


void
NLTriggerBuilder::buildTrigger(MSNet &net,
                               const SUMOSAXAttributes &attrs,
                               const std::string &base) throw(InvalidArgument)
{
    string type = attrs.getString(SUMO_ATTR_OBJECTTYPE);
    string attr = attrs.getStringSecure(SUMO_ATTR_ATTR, "");
    // check which type of a trigger shall be build
    if (type=="lane"&&attr=="speed") {
        parseAndBuildLaneSpeedTrigger(net, attrs, base);
    } else if (type=="emitter") {
        parseAndBuildLaneEmitTrigger(net, attrs, base);
    } else if (type=="rerouter") {
        parseAndBuildRerouter(net, attrs, base);
    } else if (type=="bus_stop") {
        parseAndBuildBusStop(net, attrs);
    } else if (type=="vehicle_actor") {
        /*first check, that the depending lane realy exist. if not just forget this VehicleActor. */
        if (attrs.getInt(SUMO_ATTR_TYPE) == 3) {
            unsigned int cell_id   = attrs.getInt(SUMO_ATTR_ID);
            unsigned int interval  = attrs.getInt(SUMO_ATTR_LANE);
            unsigned int statcount = attrs.getInt(SUMO_ATTR_POSITION);
            MSPhoneNet* pPhone = MSNet::getInstance()->getMSPhoneNet();
            if (pPhone->getMSPhoneCell(cell_id) != 0)
                pPhone->getMSPhoneCell(cell_id)->setStatParams(interval, statcount);
        } else if (attrs.getInt(SUMO_ATTR_TYPE) == 4) {
            /*this is the trigger for the duration for an interval for an hour*/
            unsigned int cell_id   = attrs.getInt(SUMO_ATTR_ID);
            unsigned int interval  = attrs.getInt(SUMO_ATTR_LANE);
            unsigned int count = attrs.getInt(SUMO_ATTR_POSITION);
            float duration = attrs.getFloat(SUMO_ATTR_TO);
            float deviation  = attrs.getFloat(SUMO_ATTR_XTO);
            unsigned int entering  = (unsigned int) attrs.getFloat(SUMO_ATTR_ENTERING);
            //insert in MSPhoneNet
            MSPhoneNet* pPhone = MSNet::getInstance()->getMSPhoneNet();
            if (pPhone->getMSPhoneCell(cell_id) != 0)
                pPhone->getMSPhoneCell(cell_id)->setDynParams(interval, count, duration, deviation, entering);
        } else {
            /*check that the depending lane realy exist. if not just forget this VehicleActor. */
            MSLane *tlane = MSLane::dictionary(attrs.getString(SUMO_ATTR_LANE));
            if (tlane!=0)
                parseAndBuildVehicleActor(net, attrs);
        }
    } else if (type=="calibrator") {
        parseAndBuildCalibrator(net, attrs, base);
    }

    if (!myHaveInformedAboutDeprecatedTriggerDefinition) {
        MsgHandler::getWarningInstance()->inform("Defining '" + type + "' using a trigger definition is deprecated.");
        myHaveInformedAboutDeprecatedTriggerDefinition = true;
    }
}



void
NLTriggerBuilder::buildVaporizer(const SUMOSAXAttributes &attrs) throw()
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("vaporizer", id, false)) {
        MsgHandler::getErrorInstance()->inform("Missing or empty id in a vaporizer-object.");
        return;
    }
    MSEdge *e = MSEdge::dictionary(id);
    if (e==0) {
        MsgHandler::getErrorInstance()->inform("Unknown edge referenced in a vaporizer ('" + id + "').");
        return;
    }
    bool ok = true;
    SUMOTime begin = attrs.getIntReporting(SUMO_ATTR_BEGIN, "vaporizer", 0, ok);
    SUMOTime end = attrs.getIntReporting(SUMO_ATTR_END, "vaporizer", 0, ok);
    if (!ok) {
        return;
    }
    if (begin<0) {
        MsgHandler::getErrorInstance()->inform("A vaporization begin is negative (edge id='" + id + "').");
        return;
    }
    if (end<0) {
        MsgHandler::getErrorInstance()->inform("A vaporization end is negative (edge id='" + id + "').");
        return;
    }
    if (begin>end) {
        MsgHandler::getErrorInstance()->inform("A vaporization ends before it starts (edge id='" + id + "').");
        return;
    }
    if (begin==end) {
        MsgHandler::getWarningInstance()->inform("A vaporization starts and ends at same time (edge id='" + id + "'); discarded.");
        return;
    }
    if (end>=OptionsCont::getOptions().getInt("begin")) {
        Command* cb = new WrappingCommand< MSEdge >(e, &MSEdge::incVaporization);
        MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(cb, begin, MSEventControl::ADAPT_AFTER_EXECUTION);
        Command* ce = new WrappingCommand< MSEdge >(e, &MSEdge::decVaporization);
        MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(ce, end, MSEventControl::ADAPT_AFTER_EXECUTION);
    }
}



void
NLTriggerBuilder::parseAndBuildLaneSpeedTrigger(MSNet &net, const SUMOSAXAttributes &attrs,
        const std::string &base) throw(InvalidArgument)
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("lane speed trigger", id, false)) {
        throw InvalidArgument("A lane speed trigger definition does not contain an id");
    }
    // get the file name to read further definitions from
    string file = getFileName(attrs, base);
    string objectid;
    if(attrs.hasAttribute(SUMO_ATTR_LANES)) {
        objectid = attrs.getString(SUMO_ATTR_LANES);
    } else {
        if(attrs.hasAttribute(SUMO_ATTR_OBJECTID)) {
            objectid = attrs.getString(SUMO_ATTR_OBJECTID);
            MsgHandler::getWarningInstance()->inform("Defining the lanes using 'objectid' within a variable speed sign is depracted, use 'lanes' instead.");
        }
    }
    std::vector<MSLane*> lanes;
    StringTokenizer st(objectid, ";");
    while (st.hasNext()) {
        MSLane *lane = MSLane::dictionary(st.next());
        if (lane==0) {
            throw InvalidArgument("The lane to use within MSLaneSpeedTrigger '" + id + "' is not known.");
        }
        lanes.push_back(lane);
    }
    if (lanes.size()==0) {
        throw InvalidArgument("No lane defined for MSLaneSpeedTrigger '" + id + "'.");
    }
    try {
        buildLaneSpeedTrigger(net, id, lanes, file);
    } catch (ProcessError &e) {
        throw InvalidArgument(e.what());
    }
}


void
NLTriggerBuilder::parseAndBuildLaneEmitTrigger(MSNet &net, const SUMOSAXAttributes &attrs,
        const std::string &base) throw(InvalidArgument)
{
    if(!myHaveInformedAboutDeprecatedEmitter) {
        myHaveInformedAboutDeprecatedEmitter = true;
        MsgHandler::getWarningInstance()->inform("Emitter are deprecated; use departpos/departspeed within routes instead.");
    }
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("emitter", id, false)) {
        throw InvalidArgument("An emitter does not contain an id");
    }
    // get the file name to read further definitions from
    string file = getFileName(attrs, base);
    MSLane *lane = getLane(attrs, "emitter", id);
    SUMOReal pos = getPosition(attrs, lane, "emitter", id);
    buildLaneEmitTrigger(net, id, lane, pos, file);
}


void
NLTriggerBuilder::parseAndBuildBusStop(MSNet &net, const SUMOSAXAttributes &attrs) throw(InvalidArgument)
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("bus stop", id, false)) {
        throw InvalidArgument("A bus stop does not contain an id");
    }
    // get the lane
    MSLane *lane = getLane(attrs, "bus_stop", id);
    // get the positions
    SUMOReal frompos, topos;
    try {
        frompos = attrs.getFloat(SUMO_ATTR_FROM);
        topos = attrs.getFloat(SUMO_ATTR_TO);
    } catch (EmptyData&) {
        throw InvalidArgument("Either the begin or the end position of busstop '" + id + "' is not given.");
    } catch (NumberFormatException&) {
        throw InvalidArgument("Either the begin or the end position of busstop '" + id + "' is not numeric.");
    }
    if (frompos<0) {
        frompos = lane->length() + frompos;
    }
    if (topos<0) {
        topos = lane->length() + topos;
    }
    // get the lines
    std::string lineStr = attrs.getStringSecure(SUMO_ATTR_LINES, "");
    std::vector<std::string> lines;
    if (lineStr.length()!=0) {
        StringTokenizer st(lineStr, ";");
        lines = st.getVector();
    }
    // build the bus stop
    buildBusStop(net, id, lines, lane, frompos, topos);
}


void
NLTriggerBuilder::parseAndBuildVehicleActor(MSNet &net, const SUMOSAXAttributes &attrs) throw(InvalidArgument)
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("vehicle actor", id, false)) {
        throw InvalidArgument("A vehicle actor does not contain an id");
    }
    MSLane *lane = getLane(attrs, "vehicle_actor", id);
    SUMOReal pos = getPosition(attrs, lane, "vehicle_actor", id);
    unsigned int cellid = attrs.getInt(SUMO_ATTR_TO);
    unsigned int laid = attrs.getInt(SUMO_ATTR_XTO);
    unsigned int type = attrs.getInt(SUMO_ATTR_TYPE);
    buildVehicleActor(net, id, lane, pos, laid, cellid, type);
}


void
NLTriggerBuilder::parseAndBuildCalibrator(MSNet &net, const SUMOSAXAttributes &attrs,
        const std::string &base) throw(InvalidArgument)
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("calibrator", id, false)) {
        throw InvalidArgument("A calibrator does not contain an id");
    }
    // get the file name to read further definitions from
    string file = getFileName(attrs, base);
    MSLane *lane = getLane(attrs, "calibrator", id);
    SUMOReal pos = getPosition(attrs, lane, "calibrator", id);
#ifdef HAVE_MESOSIM
    if (MSGlobals::gUseMesoSim) {
        MESegment *s = MSGlobals::gMesoNet->getSegmentForEdge(lane->getEdge());
        MESegment *prev = s;
        SUMOReal cpos = 0;
        while (cpos<pos&&s!=0) {
            prev = s;
            cpos += s->getLength();
            s = s->getNextSegment();
        }
        SUMOReal rpos = pos-cpos-prev->getLength();
        buildCalibrator(net, id, prev, rpos, file);
    } else {
#endif
        buildLaneCalibrator(net, id, lane, pos, file);
#ifdef HAVE_MESOSIM
    }
#endif
}


void
NLTriggerBuilder::parseAndBuildRerouter(MSNet &net, const SUMOSAXAttributes &attrs,
                                        const std::string &base) throw(InvalidArgument)
{
    // get the id, throw if not given or empty...
    string id;
    if (!attrs.setIDFromAttributes("rerouter", id, false)) {
        throw InvalidArgument("A rerouter does not contain an id");
    }
    // get the file name to read further definitions from
    string file = getFileName(attrs, base);
    string objectid;
    if(attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        objectid = attrs.getString(SUMO_ATTR_EDGES);
    } else {
        if(attrs.hasAttribute(SUMO_ATTR_OBJECTID)) {
            objectid = attrs.getString(SUMO_ATTR_OBJECTID);
            MsgHandler::getWarningInstance()->inform("Defining the edges using 'objectid' within a rerouter is depracted, use 'edges' instead.");
        }
    }
    std::vector<MSEdge*> edges;
    StringTokenizer st(objectid, ";");
    while (st.hasNext()) {
        MSEdge *edge = MSEdge::dictionary(st.next());
        if (edge==0) {
            throw InvalidArgument("The edge to use within MSTriggeredRerouter '" + id + "' is not known.");
        }
        edges.push_back(edge);
    }
    if (edges.size()==0) {
        throw InvalidArgument("No edges found for MSTriggeredRerouter '" + id + "'.");
    }

    SUMOReal prob;
    try {
        prob = attrs.getFloatSecure(SUMO_ATTR_PROB, 1);
    } catch (NumberFormatException &) {
        throw InvalidArgument("Invalid probability in definition of MSTriggeredRerouter '" + id + "'.");
    }
    buildRerouter(net, id, edges, prob, file, attrs.getBoolSecure(SUMO_ATTR_OFF, false));
}


// -------------------------


void
NLTriggerBuilder::buildLaneSpeedTrigger(MSNet &net, const std::string &id,
                                        const std::vector<MSLane*> &destLanes,
                                        const std::string &file) throw(ProcessError)
{
    new MSLaneSpeedTrigger(id, net, destLanes, file);
}


void
NLTriggerBuilder::buildLaneEmitTrigger(MSNet &net, const std::string &id,
                                       MSLane *destLane, SUMOReal pos,
                                       const std::string &file) throw()
{
    new MSEmitter(id, net, destLane, pos, file);
}


void
NLTriggerBuilder::buildLaneCalibrator(MSNet &net, const std::string &id,
                                      MSLane *destLane, SUMOReal pos,
                                      const std::string &file) throw()
{
    new MSCalibrator(id, net, destLane, pos, file);
}


#ifdef HAVE_MESOSIM
void
NLTriggerBuilder::buildCalibrator(MSNet &net, const std::string &id,
                                  MESegment *edge, SUMOReal pos,
                                  const std::string &file) throw()
{
    new METriggeredCalibrator(id, edge, file);
}
#endif


void
NLTriggerBuilder::buildRerouter(MSNet &, const std::string &id,
                                std::vector<MSEdge*> &edges,
                                SUMOReal prob, const std::string &file, bool off) throw()
{
    new MSTriggeredRerouter(id, edges, prob, file, off);
}


void
NLTriggerBuilder::buildBusStop(MSNet &net, const std::string &id,
                               const std::vector<std::string> &lines,
                               MSLane *lane, SUMOReal frompos, SUMOReal topos) throw()
{
    net.addBusStop(new MSBusStop(id, lines, *lane, frompos, topos));
}


void
NLTriggerBuilder::buildVehicleActor(MSNet &, const std::string &id,
                                    MSLane *lane, SUMOReal pos, unsigned int la,
                                    unsigned int cell, unsigned int type) throw()
{
    new MSE1VehicleActor(id, lane, pos, la, cell, type);
}




std::string
NLTriggerBuilder::getFileName(const SUMOSAXAttributes &attrs,
                              const std::string &base) throw()
{
    // get the file name to read further definitions from
    string file = attrs.getString(SUMO_ATTR_FILE);
    // check whether absolute or relative filenames are given
    if (!FileHelpers::isAbsolute(file)) {
        return FileHelpers::getConfigurationRelative(base, file);
    }
    return file;
}


MSLane *
NLTriggerBuilder::getLane(const SUMOSAXAttributes &attrs,
                          const std::string &tt,
                          const std::string &tid) throw(InvalidArgument)
{
    string objectid;
    if(attrs.hasAttribute(SUMO_ATTR_LANE)) {
        objectid = attrs.getString(SUMO_ATTR_LANE);
    } else {
        if(attrs.hasAttribute(SUMO_ATTR_OBJECTID)) {
            objectid = attrs.getString(SUMO_ATTR_OBJECTID);
            MsgHandler::getWarningInstance()->inform("Defining the lane using 'objectid' within " + tt + " is depracted, use 'lane' instead.");
        }
    }
    MSLane *lane = MSLane::dictionary(objectid);
    if (lane==0) {
        throw InvalidArgument("The lane " + objectid + " to use within the " + tt + " '" + tid + "' is not known.");
    }
    return lane;
}


SUMOReal
NLTriggerBuilder::getPosition(const SUMOSAXAttributes &attrs,
                              MSLane *lane,
                              const std::string &tt, const std::string &tid) throw(InvalidArgument)
{

    SUMOReal pos = attrs.getFloat(SUMO_ATTR_POSITION);
    bool friendly_pos = attrs.getBoolSecure(SUMO_ATTR_FRIENDLY_POS, false);
    if (pos<0) {
        pos = lane->length() + pos;
    }
    if (pos>lane->length()) {
        if (friendly_pos) {
            pos = lane->length() - (SUMOReal) 0.1;
        } else {
            throw InvalidArgument("The position of " + tt + " '" + tid + "' lies beyond the lane's '" + lane->getID() + "' length.");
        }
    }
    return pos;
}



/****************************************************************************/
