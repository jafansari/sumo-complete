/****************************************************************************/
/// @file    DFDetFlowLoader.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// }
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
#pragma warning(disable: 4503)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <iostream> // !!! debug only
#include <fstream>
#include <sstream>
#include <utils/importio/LineHandler.h>
#include <utils/gfx/RGBColor.h>
#include <utils/options/OptionsCont.h>
#include <utils/router/IDSupplier.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/TplConvert.h>
#include <utils/common/UtilExceptions.h>
#include "DFDetFlowLoader.h"

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
DFDetFlowLoader::DFDetFlowLoader(RODFDetectorCon &dets,
                                 RODFDetectorFlows &into,
                                 SUMOTime startTime, SUMOTime endTime,
                                 int timeOffset)
        : myStorage(into), myTimeOffset(timeOffset),
        myStartTime(startTime), myEndTime(endTime), myDetectorContainer(dets)
{}



DFDetFlowLoader::~DFDetFlowLoader()
{}


void
DFDetFlowLoader::read(const std::string &file, bool fast)
{
    if (fast) {
        parseFast(file);
    } else {
        myFirstLine = true;
        myReader.setFileName(file);
        myReader.readAll(*this);
    }
}


bool
DFDetFlowLoader::parseFast(const std::string &file)
{
    ifstream strm(file.c_str());
    if (!strm.good()) {
        MsgHandler::getErrorInstance()->inform("Could not open '" + file + "'.");
        return false;
    }
    while (strm.good()) {
        FlowDef fd;
        string detName;
        int time;
        fd.isLKW = 0;
        strm >> time;
        time -= myTimeOffset;
        strm >> detName;
        strm >> fd.qPKW;
        strm >> fd.qLKW;
        strm >> fd.vPKW;
        strm >> fd.vLKW;
        if (time<myStartTime||time>myEndTime) {
            continue;
        }
        if (fd.qLKW<0) {
            fd.qLKW = 0;
        }
        if (fd.qPKW<0) {
            fd.qPKW = 0;
        }
        myStorage.addFlow(detName, time, fd);
    }
    return true;
}


bool
DFDetFlowLoader::report(const std::string &result)
{
    // parse the flows
    if (myFirstLine) {
        myLineHandler.reinit(result, ";", ";", true, true);
        myFirstLine = false;
        return true;
    } else {
        myLineHandler.parseLine(result);
        try {
            string detName = myLineHandler.get("detector");
            int time = TplConvert<char>::_2int((myLineHandler.get("time").c_str()));
            time -= myTimeOffset;
            if (time<myStartTime||time>myEndTime) {
                return true;
            }
            FlowDef fd;
            fd.isLKW = 0;
            fd.qPKW = TplConvert<char>::_2SUMOReal(myLineHandler.get("qpkw").c_str());
            fd.vPKW = TplConvert<char>::_2SUMOReal(myLineHandler.get("vpkw").c_str());
            fd.qLKW = 0;
            if (myLineHandler.know("qLKW")) {
                fd.qLKW = TplConvert<char>::_2SUMOReal(myLineHandler.get("qlkw").c_str());
            }
            fd.vLKW = 0;
            if (myLineHandler.know("vLKW")) {
                fd.vLKW = TplConvert<char>::_2SUMOReal(myLineHandler.get("vlkw").c_str());
            }
            if (fd.qLKW<0) {
                fd.qLKW = 0;
            }
            if (fd.qPKW<0) {
                fd.qPKW = 0;
            }
            if (false) { // !!!!
                assert(fd.qPKW>=fd.qLKW);
                fd.qPKW -= fd.qLKW;
            }
            myStorage.addFlow(detName, time, fd);
            return true;
        } catch (UnknownElement &) {} catch (OutOfBoundsException &) {}
        throw ProcessError("The detector-flow-file '" + myReader.getFileName() + "' is corrupt;\n"
                           + " The following values must be supplied : 'Detector', 'Time', 'qPKW', 'vPKW'\n"
                           + " The according column names must be given in the first line of the file.");
        return false;
    }
}



/****************************************************************************/

