//---------------------------------------------------------------------------//
//                        PreStartInitialised.cpp -
//  Basic class for artifacts that must be reinitialised before a simulation
//      may be restarted
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
// Revision 1.1  2003/02/07 10:40:13  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <microsim/MSNet.h>
#include "PreStartInitialised.h"


/* =========================================================================
 * member definitions
 * ======================================================================= */
PreStartInitialised::PreStartInitialised(MSNet &net)
{
    net.addPreStartInitialisedItem(this);
}

PreStartInitialised::~PreStartInitialised()
{
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "PreStartInitialised.icc"
//#endif

// Local Variables:
// mode:C++
// End:


