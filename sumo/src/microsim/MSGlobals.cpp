/***************************************************************************
                          MSGlobals.cpp  -
    Some static variables for faster access
                             -------------------
    project              : SUMO
    begin                : late summer 2003
    copyright            : (C) 2003 by DLR/IVF http://ivf.dlr.de/
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.12  2006/11/02 11:44:50  dkrajzew
// added Danilo Teta-Boyom's changes to car2car-communication
//
// Revision 1.11  2006/09/18 10:06:04  dkrajzew
// removed deprecated c2c functions, added new made by Danilot Boyom
//
// Revision 1.10  2006/05/15 05:52:55  dkrajzew
// debugging saving/loading of states
//
// Revision 1.10  2006/05/08 11:06:59  dkrajzew
// debugging loading/saving of states
//
// Revision 1.9  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/07/12 12:22:50  dkrajzew
// made checking for accidents optional
//
// Revision 1.5  2005/05/04 08:25:29  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.4  2004/11/23 10:20:09  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.3  2003/12/04 13:30:41  dkrajzew
// work on internal lanes
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

#include "MSGlobals.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * static member variable definitions
 * ======================================================================= */
bool MSGlobals::gOmitEmptyEdgesOnDump;

bool MSGlobals::gUsingInternalLanes;

size_t MSGlobals::gTimeToGridlock = 300;

SUMOReal MSGlobals::gMinLaneVMax4FalseLaneTeleport;

SUMOReal MSGlobals::gMaxVehV4FalseLaneTeleport;

SUMOReal MSGlobals::gMinVehDist4FalseLaneTeleport;

bool MSGlobals::gCheck4Accidents;

bool MSGlobals::gStateLoaded;



bool MSGlobals::gUsingC2C = false;

SUMOReal MSGlobals::gLANRange = 100;

SUMOReal MSGlobals::gNumberOfSendingPos = 732;

SUMOReal MSGlobals::gInfoPerPaket = 14;

SUMOReal MSGlobals::gLANRefuseOldInfosOffset = 15 * 60;

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
