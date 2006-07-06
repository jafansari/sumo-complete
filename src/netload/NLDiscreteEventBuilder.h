#ifndef NLDiscreteEventBuilder_h
#define NLDiscreteEventBuilder_h
/***************************************************************************
                          NLDiscreteEventBuilder .h
                          A building helper for simulation actions
                             -------------------
    begin                : Sep, 2003
    copyright            : (C) 2003 by DLR http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// $Log$
// Revision 1.7  2006/07/06 12:22:06  dkrajzew
// tls switches added
//
// Revision 1.6  2005/10/10 12:10:59  dkrajzew
// reworking the tls-API: made tls-control non-static; made net an element of traffic lights
//
// Revision 1.5  2005/10/07 11:41:49  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/15 12:04:36  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2005/05/04 08:39:46  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.2  2004/01/12 14:36:21  dkrajzew
// removed some dead code; documentation added
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

#include <sax2/Attributes.hpp>
#include <string>
#include <map>
#include <utils/xml/AttributesHandler.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class Command;
class MSNet;


/* =========================================================================
 * xerces 2.2 compatibility
 * ======================================================================= */
#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace XERCES_CPP_NAMESPACE;
#endif


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class NLDiscreteEventBuilder
 * This class is responsible for building event-handling actions which
 *  the simulation shall execute.
 */
class NLDiscreteEventBuilder : public AttributesHandler {
public:
	/// Known action types
    enum ActionType {
        EV_SAVETLSTATE,
        EV_SAVETLSWITCHES
    };

	/// Constructor
    NLDiscreteEventBuilder(MSNet &net);

	/// Destructor
    ~NLDiscreteEventBuilder();

	/// Builds an action and saves it for further use
    void addAction(const Attributes &attrs,
        const std::string &basePath);

private:
	/// Builds an action which save the state of a certain tls into a file
    Command *buildSaveTLStateCommand(const Attributes &attrs,
        const std::string &basePath);

	/// Builds an action which save the state of a certain tls into a file
    Command *buildSaveTLSwitchesCommand(const Attributes &attrs,
        const std::string &basePath);

protected:
	/// Definitions of a storage for build actions
    typedef std::map<std::string, ActionType> KnownActions;

	/// Build actions that shall be executed during the simulation
    KnownActions myActions;

    MSNet &myNet;

};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
//
