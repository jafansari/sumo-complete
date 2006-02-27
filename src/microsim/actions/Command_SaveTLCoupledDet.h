#ifndef Command_SaveTLCoupledDet_h
#define Command_SaveTLCoupledDet_h
//---------------------------------------------------------------------------//
//                        Command_SaveTLCoupledDet.h -
//  Realises the output of a tls values on each switch
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
// Revision 1.7  2006/02/27 12:03:23  dkrajzew
// variants container named properly
//
// Revision 1.6  2006/02/23 11:27:57  dkrajzew
// tls may have now several programs
//
// Revision 1.5  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/15 11:07:14  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2004/11/23 10:18:24  dkrajzew
// new detectors usage applied
//
// Revision 1.2  2004/02/16 14:02:57  dkrajzew
// e2-link-dependent detectors added
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
#include <fstream>
#include "Action.h"
#include <microsim/MSNet.h>
#include <utils/helpers/DiscreteCommand.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSNet;
class FileWriter;
class MSTrafficLightLogic;
class MSDetectorFileOutput;
class OutputDevice;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class Command_SaveTLCoupledDet
 * Called on every tls-switch, the "execute" writes the current detector
 *  values into a file.
 * This action is build only if the user wants and describes it within the
 *  additional-files.
 */
class Command_SaveTLCoupledDet : public DiscreteCommand
{
public:
    /// Constructor
    Command_SaveTLCoupledDet(
        const MSTLLogicControl::TLSLogicVariants &tlls,
        MSDetectorFileOutput *dtf, unsigned int begin,
        OutputDevice *device);

    /// Destructor
    virtual ~Command_SaveTLCoupledDet();

    /// Executes the command (see above)
    virtual bool execute();

protected:
    /// The file to write the output to
    OutputDevice *myDevice;

    /// The logic to use
    const MSTLLogicControl::TLSLogicVariants &myLogics;

    /// The detector to use
    MSDetectorFileOutput *myDetector;

    /// The last time the values were written
    unsigned int myStartTime;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
