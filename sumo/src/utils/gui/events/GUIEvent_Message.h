/****************************************************************************/
/// @file    GUIEvent_Message.h
/// @author  Daniel Krajzewicz
/// @date    Wed 18 Jun 2003
/// @version $Id$
///
// Event send when a message (message, warning, error) has to besubmitted
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
#ifndef GUIEvent_Message_h
#define GUIEvent_Message_h
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

#include "GUIEvent.h"
#include <string>
#include <utils/common/MsgHandler.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * GUIEvent_Message
 * Throw from GUIRunThread to GUIApplicationWindow and then further to all
 * displays after a step has been performed
 */
class GUIEvent_Message : public GUIEvent
{
public:
    /// constructor
    GUIEvent_Message(MsgHandler::MsgType type, const std::string &msg)
            : GUIEvent(EVENT_MESSAGE_OCCURED), myMsg(msg)
    {
        switch (type) {
        case MsgHandler::MT_MESSAGE:
            myType = EVENT_MESSAGE_OCCURED;
            break;
        case MsgHandler::MT_WARNING:
            myType = EVENT_WARNING_OCCURED;
            break;
        case MsgHandler::MT_ERROR:
            myType = EVENT_ERROR_OCCURED;
            break;
        default:
            throw 1;
        }
    }

    /// destructor
    ~GUIEvent_Message()
{ }

    /// Returns the message
    const std::string &getMsg() const
    {
        return myMsg;
    }

protected:

    /// The message
    std::string myMsg;

};


#endif

/****************************************************************************/

