#ifndef Command_SaveTLCoupledLaneDet_h
#define Command_SaveTLCoupledLaneDet_h
//---------------------------------------------------------------------------//
//                        Command_SaveTLCoupledLaneDet.h -
//  Realises the output of a lane's detector values if the lane has green light
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : 15 Feb 2004
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
// Revision 1.7  2006/03/17 08:53:17  dkrajzew
// "Action" removed - was the same as Command; changed the Event-interface (execute now gets the current simulation time)
//
// Revision 1.6  2006/02/27 12:03:23  dkrajzew
// variants container named properly
//
// Revision 1.5  2006/02/23 11:27:57  dkrajzew
// tls may have now several programs
//
// Revision 1.4  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 11:07:14  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2004/11/23 10:18:24  dkrajzew
// new detectors usage applied
//
// Revision 1.1  2004/02/16 14:03:46  dkrajzew
// e2-link-dependent detectors added
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

#include <microsim/MSLink.h>
#include "Command_SaveTLCoupledDet.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class Command_SaveTLCoupledLaneDet
 * Called on every tls-switch, the "execute" checks whether the state of the
 *  assigned link is green. If so, the detector values of the assigned lane
 *  detectors are written to a file.
 * This action is build only if the user wants and describes it within the
 *  additional-files.
 */
class Command_SaveTLCoupledLaneDet : public Command_SaveTLCoupledDet
{
public:
    /// Constructor
    Command_SaveTLCoupledLaneDet(
        const MSTLLogicControl::TLSLogicVariants &tlls,
        MSDetectorFileOutput *dtf,
        unsigned int begin, OutputDevice *device, MSLink *link);

    /// Destructor
    ~Command_SaveTLCoupledLaneDet();

    /// Executes the command (see above)
    bool execute(SUMOTime currentTime);

private:
    /// The link to check
    MSLink *myLink;

    /// The state the link had the last time
    MSLink::LinkState myLastState;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

