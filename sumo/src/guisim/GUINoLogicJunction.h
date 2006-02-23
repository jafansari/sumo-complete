#ifndef GUINoLogicJunction_H
#define GUINoLogicJunction_H
//---------------------------------------------------------------------------//
//                        GUINoLogicJunction.h -
//  A MSNoLogicJunction with a graphical representation
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
// $Log$
// Revision 1.8  2006/02/23 11:27:56  dkrajzew
// tls may have now several programs
//
// Revision 1.7  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2004/08/02 11:58:14  dkrajzew
// using Position2D instead of two SUMOReals
//
// Revision 1.3  2003/12/04 13:38:16  dkrajzew
// usage of internal links added
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
#include <vector>
#include <bitset>
#include <utils/geom/Position2DVector.h>
#include <microsim/MSNoLogicJunction.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIJunctionWrapper;
class GUIGlObjectStorage;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUINoLogicJunction
 * This class extends the MSNoLogicJunction by a graphical representation and
 *  the ability to build a wrapper that displays this representation
 */
class GUINoLogicJunction
    : public MSNoLogicJunction
{
public:
    /// Destructor.
    ~GUINoLogicJunction();

    /** Use this constructor only. */
    GUINoLogicJunction( const std::string &id, const Position2D &position,
        LaneCont incoming,
#ifdef HAVE_INTERNAL_LANES
        LaneCont internal,
#endif
        const Position2DVector &shape);

    /// Builds the wrapper of the graphical representation of this junction
    GUIJunctionWrapper *buildJunctionWrapper(GUIGlObjectStorage &idStorage);

private:
    /// The shape of the junction
    Position2DVector myShape;

private:
    /// Invalidated copy constructor.
    GUINoLogicJunction( const GUINoLogicJunction& );

    /// Invalidated assignment operator.
    GUINoLogicJunction& operator=( const GUINoLogicJunction& );

};

/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
