#ifndef GUIEdgeCont_h
#define GUIEdgeCont_h
//---------------------------------------------------------------------------//
//                        GUIEdgeCont.h -
//  A container for gui-edges
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
// $Log$
// Revision 1.6  2006/05/16 08:11:36  dkrajzew
// spelling patched
//
// Revision 1.5  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2005/05/04 07:59:59  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.2  2003/02/07 10:39:17  dkrajzew
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

#include <vector>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * This is simply a std::vector of GUIEdges; It probably should be changed
 * to "GUIEdgeVector" !!!
 */
typedef std::vector<GUIEdge*> GUIEdgeCont;


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

