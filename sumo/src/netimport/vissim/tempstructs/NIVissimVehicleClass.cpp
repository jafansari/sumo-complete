//---------------------------------------------------------------------------//
//                        NIVissimVehicleClass.cpp -  ccc
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.3  2003/10/27 10:51:55  dkrajzew
// edges speed setting implemented (only on an edges begin)
//
// Revision 1.2  2003/06/05 11:46:57  dkrajzew
// class templates applied; documentation added
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H


#include "NIVissimVehicleClass.h"

NIVissimVehicleClass::NIVissimVehicleClass(int type,
                                           double percentage,
                                           int vwish)
    : myType(type), myPercentage(percentage), myVWish(vwish)
{
}


NIVissimVehicleClass::~NIVissimVehicleClass()
{
}


int
NIVissimVehicleClass::getSpeed() const
{
    return myVWish;
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "NIVissimVehicleClass.icc"
//#endif

// Local Variables:
// mode:C++
// End:


