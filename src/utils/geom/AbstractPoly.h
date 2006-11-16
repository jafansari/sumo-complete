#ifndef AbstractPoly_h
#define AbstractPoly_h
//---------------------------------------------------------------------------//
//                        AbstractPoly.h -
//  The base class for polygons
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
// Revision 1.7  2006/11/16 10:50:52  dkrajzew
// warnings removed
//
// Revision 1.6  2005/10/07 11:44:16  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.5  2005/09/23 06:07:01  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.4  2005/09/15 12:18:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2004/11/23 10:34:46  dkrajzew
// debugging
//
// Revision 1.2  2003/06/05 14:33:45  dkrajzew
// class templates applied; documentation added
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

#include "Position2D.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 *
 */
class AbstractPoly {
public:
    AbstractPoly() { }
    AbstractPoly(const AbstractPoly &s) { }
    virtual ~AbstractPoly() { }
    virtual bool around(const Position2D &p, SUMOReal offset=0) const = 0;
    virtual bool overlapsWith(const AbstractPoly &poly, SUMOReal offset=0) const = 0;
    virtual bool partialWithin(const AbstractPoly &poly, SUMOReal offset=0) const = 0;
    virtual bool crosses(const Position2D &p1,
        const Position2D &p2) const = 0;
};



/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

