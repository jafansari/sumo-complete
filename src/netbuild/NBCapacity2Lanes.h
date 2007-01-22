#ifndef NBCapacity2Lanes_h
#define NBCapacity2Lanes_h
/***************************************************************************
                          NBCapacity2Lanes.h
			  A helper class for the computation of the number of lanes an
                edge has in dependence to this edge's capacity
                             -------------------
    project              : SUMO
    begin                : Fri, 19 Jul 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.4  2005/09/23 06:01:05  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.3  2005/04/27 11:48:25  dkrajzew
// level3 warnings removed; made containers non-static
//
// Revision 1.2  2003/02/07 10:43:44  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:48:13  dkrajzew
// initial commit for net building classes
//
// Revision 1.1  2002/07/25 08:41:44  dkrajzew
// Visum7.5 and Cell import added
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


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * NBCapacity2Lanes
 * A helper class for the computation of the number of lanes an edge has
 * in dependence to this edge's capacity
 */
class NBCapacity2Lanes {
public:
    /// constructor
    NBCapacity2Lanes(SUMOReal divider) : _divider(divider) { }

    /// destructor
    ~NBCapacity2Lanes() { }

    /// returns the number of lanes computed from the given capacity
    int get(SUMOReal capacity) {
        capacity /= _divider;
        if(capacity>(int) capacity) {
            capacity += 1;
        }
        // just assure that the number of lanes is not zero
        if(capacity==0) {
            capacity = 1;
        }
        return (int) capacity;
    }

private:
    /// the norming factor
    SUMOReal _divider;
};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
