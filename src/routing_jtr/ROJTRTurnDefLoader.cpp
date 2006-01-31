//---------------------------------------------------------------------------//
//                        ROJTRTurnDefLoader.cpp -
//      Loader for the description of turning percentages
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 20 Jan 2004
//  copyright            : (C) 2004 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.2  2006/01/31 11:01:40  dkrajzew
// patching incoherences; added possibility to end on non-sink-edges
//
// Revision 1.1  2005/10/10 12:09:36  dkrajzew
// renamed ROJP*-classes to ROJTR*
//
// Revision 1.8  2005/10/07 11:42:39  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/23 06:04:58  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.6  2005/09/15 12:05:34  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/05/04 08:57:12  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.4  2004/12/16 12:26:52  dkrajzew
// debugging
//
// Revision 1.3  2004/11/23 10:26:59  dkrajzew
// debugging
//
// Revision 1.2  2004/07/02 09:40:36  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived for an online-routing (lane index added)
//
// Revision 1.1  2004/02/06 08:43:46  dkrajzew
// new naming applied to the folders (jp-router is now called jtr-router)
//
// Revision 1.1  2004/01/26 06:09:11  dkrajzew
// initial commit for jp-classes
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <set>
#include <string>
#include <utils/common/FileHelpers.h>
#include <utils/common/XMLHelpers.h>
#include <utils/importio/CSVHelpers.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/TplConvert.h>
#include <utils/sumoxml/SUMOXMLDefinitions.h>
#include <router/RONet.h>
#include "ROJTRHelpers.h"
#include "ROJTREdge.h"
#include "ROJTRTurnDefLoader.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
ROJTRTurnDefLoader::ROJTRTurnDefLoader(RONet &net)
    : SUMOSAXHandler("turn-definitions"), myNet(net),
    myAmInitialised(false)
{
}


ROJTRTurnDefLoader::~ROJTRTurnDefLoader()
{
}


std::set<ROJTREdge*>
ROJTRTurnDefLoader::load(const std::string &file)
{
    _file = file;
    FileHelpers::FileType type = FileHelpers::checkFileType(file);
    switch(type) {
    case FileHelpers::XML:
        XMLHelpers::runParser(*this, file);
        return mySinks;
    case FileHelpers::CSV:
        CSVHelpers::runParser(*this, file);
        return mySinks;
    default:
        throw 1;
    }
}


void
ROJTRTurnDefLoader::myStartElement(int element, const std::string &name,
                                  const Attributes &attrs)
{
    switch(element) {
    case SUMO_TAG_INTERVAL:
        beginInterval(attrs);
        break;
    case SUMO_TAG_FROMEDGE:
        beginFromEdge(attrs);
        break;
    case SUMO_TAG_TOEDGE:
        addToEdge(attrs);
        break;
    }
}


void
ROJTRTurnDefLoader::myCharacters(int element, const std::string &name,
                                const std::string &chars)
{
    switch(element) {
    case SUMO_TAG_SINK:
        addSink(chars);
        break;
    }
}


void
ROJTRTurnDefLoader::myEndElement(int element, const std::string &name)
{
    switch(element) {
    case SUMO_TAG_INTERVAL:
        endInterval();
        break;
    case SUMO_TAG_FROMEDGE:
        endFromEdge();
        break;
    }
}


bool
ROJTRTurnDefLoader::report(const std::string &line)
{
    if(!myAmInitialised) {
        myColumnsParser.reinit(line, ";");
        myAmInitialised = true;
    } else {
        try {
            myColumnsParser.parseLine(line);
            try {
                myIntervalBegin =
                    TplConvert<char>::_2int(getSecure("begin").c_str());
            } catch (NumberFormatException &) {
                MsgHandler::getErrorInstance()->inform("The attribute 'from' is not numeric.");
                return false;
            }
            try {
                myIntervalEnd =
                    TplConvert<char>::_2int(getSecure("end").c_str());
            } catch (NumberFormatException &) {
                MsgHandler::getErrorInstance()->inform("The attribute 'to' is not numeric.");
                return false;
            }
            string id = getSecure("from");
            myEdge = static_cast<ROJTREdge*>(myNet.getEdge(id));
            if(myEdge==0) {
                MsgHandler::getErrorInstance()->inform(
                    "The edge '" + id + "' is not known within the network (within a 'from-edge' tag).");
                return false;
            }
            id = getSecure("to");
            ROJTREdge *edge = static_cast<ROJTREdge*>(myNet.getEdge(id));
            if(edge==0) {
                MsgHandler::getErrorInstance()->inform(
                    "The edge '" + id + "' is not known within the network (within a 'to-edge' tag).");
                return false;
            }
            SUMOReal probability;
            try {
                probability =
                    TplConvert<char>::_2SUMOReal(getSecure("split").c_str());
            } catch (NumberFormatException &) {
                MsgHandler::getErrorInstance()->inform("The attribute 'perc' is not numeric.");
                return false;
            }
            myEdge->addFollowerProbability(edge, myIntervalBegin, myIntervalEnd, probability);
        } catch (InvalidArgument &) {
            return false;
        }
    }
    return true;
}


std::string
ROJTRTurnDefLoader::getSecure(const std::string &name)
{
    try {
        return myColumnsParser.get(name);
    } catch (UnknownElement &) {
        MsgHandler::getErrorInstance()->inform("The definition of '" + name + "' is missing within the file.");
        throw InvalidArgument("");
    } catch (OutOfBoundsException &) {
        MsgHandler::getErrorInstance()->inform("The definition of '" + name + "' is missing within the file.");
        throw InvalidArgument("");
    }
}


void
ROJTRTurnDefLoader::beginInterval(const Attributes &attrs)
{
    try {
        myIntervalBegin = getInt(attrs, SUMO_ATTR_BEGIN);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform(
            "The attribute 'from' is not numeric ('" + getString(attrs, SUMO_ATTR_FROM) + "').");
        return;
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("The 'from'-attribute is not given.");
        return;
    }
    try {
        myIntervalEnd = getInt(attrs, SUMO_ATTR_END);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform(
            "The attribute 'to' is not numeric ('" + getString(attrs, SUMO_ATTR_FROM) + "').");
        return;
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("The 'to'-attribute is not given.");
        return;
    }
}


void
ROJTRTurnDefLoader::beginFromEdge(const Attributes &attrs)
{
    string id;
    try {
        id = getString(attrs, SUMO_ATTR_ID);
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("The id of an edge is missing within a 'from-edge' tag.");
        return;
    }
    //
    myEdge = static_cast<ROJTREdge*>(myNet.getEdge(id));
    if(myEdge==0) {
        MsgHandler::getErrorInstance()->inform(
            "The edge '" + id + "' is not known within the network (within a 'from-edge' tag).");
        return;
    }
}


void
ROJTRTurnDefLoader::addToEdge(const Attributes &attrs)
{
    string id;
    try {
        id = getString(attrs, SUMO_ATTR_ID);
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("The id of an edge is missing within a 'to-edge' tag.");
        return;
    }
    //
    ROJTREdge *edge = static_cast<ROJTREdge*>(myNet.getEdge(id));
    if(edge==0) {
        MsgHandler::getErrorInstance()->inform(
            "The edge '" + id + "' is not known within the network (within a 'to-edge' tag).");
        return;
    }
    try {
        SUMOReal probability = getFloat(attrs, SUMO_ATTR_PROB);
        myEdge->addFollowerProbability(edge, myIntervalBegin, myIntervalEnd, probability);
    } catch (NumberFormatException &) {
        MsgHandler::getErrorInstance()->inform(
            "The attribute 'probability' is not numeric ('" + getString(attrs, SUMO_ATTR_PROB) + "').");
        return;
    } catch (EmptyData &) {
        MsgHandler::getErrorInstance()->inform("The 'probability'-attribute is not given.");
        return;
    }
}


void
ROJTRTurnDefLoader::addSink(const std::string &chars)
{
    try {
        ROJTRHelpers::parseROJTREdges(myNet, mySinks, chars);
    } catch(ProcessError &) {
    }
}


void
ROJTRTurnDefLoader::endInterval()
{
}


void
ROJTRTurnDefLoader::endFromEdge()
{
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:




