#ifndef ROVehicleType_h
#define ROVehicleType_h
//---------------------------------------------------------------------------//
//                        ROVehicleType.h -
//  Basic class for vehicle types
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
// Revision 1.4  2003/11/11 08:04:47  dkrajzew
// avoiding emissions of vehicles on too short edges
//
// Revision 1.3  2003/07/16 15:36:50  dkrajzew
// vehicles and routes may now have colors
//
// Revision 1.2  2003/02/07 10:45:07  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <iostream>
#include <string>
#include <utils/gfx/RGBColor.h>
#include "ReferencedItem.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class ROVehicleType
 * The base class for vehicle types.
 */
class ROVehicleType : public ReferencedItem {
public:
    /// Constructor
    ROVehicleType(const std::string &id, const RGBColor &col,
        double length);

    /// Destructor
	virtual ~ROVehicleType();

    /// Saves the type using SUMO-XML
	virtual std::ostream &xmlOut(std::ostream &os);

    /// Returns the name of the type
    std::string getID() const;

    // Returns the length the vehicles of this type have
    double getLength() const;

protected:
    /// The name of the type
    std::string myID;

    /// The color of the type
    RGBColor myColor;

    /// The length of the vehicle
    double myLength;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifndef DISABLE_INLINE
//#include "ROVehicleType.icc"
//#endif

#endif

// Local Variables:
// mode:C++
// End:

