#ifndef Distribution_Points_h
#define Distribution_Points_h
//---------------------------------------------------------------------------//
//                        Distribution_Points.h -
//      The description of a distribution by a curve
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
// Revision 1.9  2006/05/15 05:59:57  dkrajzew
// debugged splitting of matrices
//
// Revision 1.8  2006/04/07 05:30:53  dkrajzew
// distribution spread computation added (undocumented)
//
// Revision 1.7  2005/10/07 11:43:51  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/23 06:06:37  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 12:17:45  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2004/01/28 12:35:22  dkrajzew
// retrival of a distribution maximum value added; documentation added
//
// Revision 1.3  2003/06/06 11:01:09  dkrajzew
// windows eol removed
//
// Revision 1.2  2003/06/05 14:33:44  dkrajzew
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

#include "Distribution.h"
#include <utils/geom/Position2DVector.h>


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class Distribution_Points
 * A description of a distribution that uses a set of points in a 2d-space
 *  to describe the values (each points x-value) and theri possibilities
 *  (each points y-value)
 */
class Distribution_Points :
        public Distribution {
public:
    /// Constructor
    Distribution_Points(const std::string &id,
        const Position2DVector &points, bool interpolating=false);

    /// Destructor
    virtual ~Distribution_Points();

    /// Returns the maximum value of this distribution
    SUMOReal getMax() const;

    size_t getAreaNo() const;
    SUMOReal getAreaBegin(size_t index) const;
    SUMOReal getAreaEnd(size_t index) const;
    SUMOReal getAreaPerc(size_t index) const;


protected:
    /// The list of points that describe the distribution
    Position2DVector myPoints;

    mutable bool myProbabilitiesAreComputed;

    mutable std::vector<SUMOReal> myProbabilities;

    bool myInterpolateDist;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

