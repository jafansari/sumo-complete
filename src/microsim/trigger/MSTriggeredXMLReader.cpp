//---------------------------------------------------------------------------//
//                        MSTriggeredXMLReader.cpp -
//  The basic class for classes that read XML-triggers
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
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
// Revision 1.9  2006/05/15 05:52:00  dkrajzew
// the reader does not shut down the application if an error occures
//
// Revision 1.9  2006/05/08 11:05:30  dkrajzew
// made false emitter definitions not shutting down the application
//
// Revision 1.8  2006/04/18 08:05:44  dkrajzew
// beautifying: output consolidation
//
// Revision 1.7  2006/04/05 05:27:37  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.6  2006/03/17 08:58:36  dkrajzew
// changed the Event-interface (execute now gets the current simulation time, event handlers are non-static)
//
// Revision 1.5  2005/11/09 06:37:52  dkrajzew
// trigger reworked
//
// Revision 1.4  2005/10/17 08:58:24  dkrajzew
// trigger rework#1
//
// Revision 1.3  2005/10/07 11:37:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.2  2005/09/22 13:45:52  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.1  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/01/06 10:48:07  dksumo
// 0.8.2.1 patches
//
// Revision 1.1  2004/10/22 12:49:27  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.7  2004/07/02 09:56:40  dkrajzew
// debugging while implementing the vss visualisation
//
// Revision 1.6  2004/01/26 07:50:43  dkrajzew
// using the xmlhelpers instead of building the parser by the object itself
//
// Revision 1.5  2003/09/23 14:18:15  dkrajzew
// hierarchy refactored; user-friendly implementation
//
// Revision 1.4  2003/09/22 14:56:07  dkrajzew
// base debugging
//
// Revision 1.3  2003/06/18 11:12:51  dkrajzew
// new message and error processing: output to user may be a message,
//  warning or an error now; it is reported to a Singleton (MsgHandler);
//  this handler puts it further to output instances.
//  changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.2  2003/02/07 10:41:50  dkrajzew
// updated
//
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

#include <string>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <sax/SAXException.hpp>
#include <sax/SAXParseException.hpp>
#include <utils/common/TplConvert.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/sumoxml/SUMOSAXHandler.h>
#include <utils/common/XMLHelpers.h>
#include <microsim/MSEventControl.h>
#include "MSTriggeredReader.h"
#include "MSTriggeredXMLReader.h"

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
MSTriggeredXMLReader::MSTriggeredXMLReader(MSNet &net,
                                           const std::string &filename)
    : MSTriggeredReader(net),
    SUMOSAXHandler("sumo-trigger values", filename),
    myHaveMore(true), myParser(0)
{
    MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(
        new MSTriggerCommand(*this), net.getCurrentTimeStep(),
            MSEventControl::NO_CHANGE);
}


MSTriggeredXMLReader::~MSTriggeredXMLReader()
{
}


bool
MSTriggeredXMLReader::readNextTriggered()
{
    try {
        while(myHaveMore&&myParser->parseNext(myToken)) {
            if(nextRead()) {
                return true;
            }
        }
    } catch (ProcessError &) {
    }
    myHaveMore = false;
    return false;
}


void
MSTriggeredXMLReader::myInit()
{
    try {
        myParser = XMLHelpers::getSAXReader(*this);
        if(!myParser->parseFirst(_file.c_str(), myToken)) {
            MsgHandler::getErrorInstance()->inform("Can not read XML-file '" + _file + "'.");
            throw ProcessError();
        }
    } catch (SAXException &e) {
        MsgHandler::getErrorInstance()->inform(TplConvert<XMLCh>::_2str(e.getMessage()));
        throw ProcessError();
    } catch (XMLException &e) {
        MsgHandler::getErrorInstance()->inform(TplConvert<XMLCh>::_2str(e.getMessage()));
        throw ProcessError();
    }

    if(readNextTriggered()) {
        if(_offset<MSNet::getInstance()->getCurrentTimeStep()) {
            _offset = MSNet::getInstance()->getCurrentTimeStep() + 1;
            // !!! Warning?
        }
        /*
        MSEventControl::getBeginOfTimestepEvents()->addEvent(
            new MSTriggerCommand(*this), _offset, MSEventControl::ADAPT_AFTER_EXECUTION);
            */
    } else {
        myHaveMore = false;
    }
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


