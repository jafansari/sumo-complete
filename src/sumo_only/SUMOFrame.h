#ifndef SUMOFrame_h
#define SUMOFrame_h
//---------------------------------------------------------------------------//
//                        SUMOFrame.h -
//  Some helping methods for usage within sumo and sumo-gui
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
// Revision 1.4  2004/02/16 13:44:27  dkrajzew
// dump output generating function renamed in order to add vehicle dump ability in the future
//
// Revision 1.3  2003/06/24 08:09:29  dkrajzew
// implemented SystemFrame and applied the changes to all applications
//
// Revision 1.2  2003/02/07 11:19:37  dkrajzew
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


/* =========================================================================
 * class declarations
 * ======================================================================= */
class OptionsCont;
class MSNet;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class SUMOFrame
 * This class holds some helping methods needed both by the sumo and the
 * sumo-gui module.
 */
class SUMOFrame {
public:
    /// Builds teh simulation options
    static void fillOptions(OptionsCont &oc);

    /// Build the network dump output stream
    static std::ostream *buildNetDumpStream(OptionsCont &oc);

    /// Build the trip description output stream
    static std::ostream *buildTripDescStream(OptionsCont &oc);

    /// Performs the post-initialisation of the net (!!! should be somewhere else)
    static void postbuild(MSNet &net);

    /**
     * Checks the build settings. The following constraints must be valid:
     * - the network-file was specified (otherwise no simulation is existing)
     * - a junction folder must be given
     *   (otherwise no junctions can be loaded)
     * - the begin and the end of the simulation must be given
     * Returns true when all constraints are valid
     */
    static bool checkOptions(OptionsCont &oc);

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifndef DISABLE_INLINE
//#include "SUMOFrame.icc"
//#endif

#endif

// Local Variables:
// mode:C++
// End:

