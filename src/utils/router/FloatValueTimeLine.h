#ifndef FloatValueTimeLine_h
#define FloatValueTimeLine_h
//---------------------------------------------------------------------------//
//                        FloatValueTimeLine.h -
//  A list of time ranges with float values
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Thu, 22 Jan 2004
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
// $Log$
// Revision 1.2  2004/11/23 10:36:20  dkrajzew
// debugging
//
// Revision 1.1  2004/01/26 10:01:44  dkrajzew
// derived class implemented for the computation of interpolated values when a
//  value is not given (not yet implemented)
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "ValueTimeLine.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class FloatValueTimeLine
 * A time line being a list of time period with assigned float values.
 * Derived from the template time line to allow the computation of
 *  non-existing values through interpolation.
 */
class FloatValueTimeLine : public ValueTimeLine<float> {
public:
    /// Constructor
    FloatValueTimeLine();

    /// Destructor
    ~FloatValueTimeLine();

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

