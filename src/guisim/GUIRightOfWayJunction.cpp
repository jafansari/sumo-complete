//---------------------------------------------------------------------------//
//                        GUIRightOfWayJunction.cpp -
//  A MSRightOfWayJunction with a graphical representation
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 1 Jul 2003
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
// Revision 1.9  2006/02/23 11:27:56  dkrajzew
// tls may have now several programs
//
// Revision 1.8  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.6  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/05/04 08:02:54  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.4  2004/08/02 11:58:14  dkrajzew
// using Position2D instead of two SUMOReals
//
// Revision 1.3  2003/12/04 13:38:16  dkrajzew
// usage of internal links added
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

#include <microsim/MSRightOfWayJunction.h>
#include <bitset>
#include <vector>
#include <string>
#include "GUIJunctionWrapper.h"
#include "GUIRightOfWayJunction.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * member method definitions
 * ======================================================================= */
GUIRightOfWayJunction::GUIRightOfWayJunction(const std::string &id,
                                             const Position2D &position,
                                             LaneCont incoming,
#ifdef HAVE_INTERNAL_LANES
                                             LaneCont internal,
#endif
                                             MSJunctionLogic* logic,
                                             const Position2DVector &shape)
    : MSRightOfWayJunction(id, position, incoming,
#ifdef HAVE_INTERNAL_LANES
    internal,
#endif
    logic),

    myShape(shape)
{
}


GUIRightOfWayJunction::~GUIRightOfWayJunction()
{
}


GUIJunctionWrapper *
GUIRightOfWayJunction::buildJunctionWrapper(GUIGlObjectStorage &idStorage)
{
    return new GUIJunctionWrapper(idStorage, *this, myShape);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
