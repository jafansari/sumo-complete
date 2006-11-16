#ifndef GUILoadThread_h
#define GUILoadThread_h
//---------------------------------------------------------------------------//
//                        GUILoadThread.h -
//  Class describing the thread that performs the loading of a simulation
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
// Revision 1.15  2006/11/16 10:50:42  dkrajzew
// warnings removed
//
// Revision 1.14  2006/05/15 05:45:18  dkrajzew
// beautifying: dead code removed
//
// Revision 1.14  2006/05/08 10:48:50  dkrajzew
// beautifying: removed dead code
//
// Revision 1.13  2006/04/18 07:54:32  dkrajzew
// unifying threads
//
// Revision 1.12  2005/10/07 11:36:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.11  2005/09/15 11:05:28  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.10  2005/05/04 07:47:23  dkrajzew
// level 3 warnings removed
//
// Revision 1.9  2004/11/23 10:11:33  dkrajzew
// adapted the new class hierarchy
//
// Revision 1.8  2004/07/02 08:28:50  dkrajzew
// some changes needed to derive the threading classes more easily added
//
// Revision 1.7  2004/04/02 11:10:20  dkrajzew
// simulation-wide output files are now handled by MSNet directly
//
// Revision 1.6  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.5  2003/11/26 09:39:13  dkrajzew
// added a logging windows to the gui (the passing of more than a single lane to come makes it necessary)
//
// Revision 1.4  2003/09/22 14:54:22  dkrajzew
// some refactoring on GUILoadThread-usage
//
// Revision 1.3  2003/06/18 11:04:53  dkrajzew
// new error processing adapted
//
// Revision 1.2  2003/02/07 10:34:14  dkrajzew
// files updated
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

#include <string>
#include <vector>
#include <fx.h>
#include <FXThread.h>
#include <utils/gui/windows/GUIAbstractLoadThread.h>
#include <utils/foxtools/FXThreadEvent.h>


 /* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIEdgeControlBuilder;
class OptionsCont;
class GUIVehicleControl;
class GUINet;


/* =========================================================================
 * class definitions
 * ======================================================================= */
class GUILoadThread : public GUIAbstractLoadThread
{
public:
    /// constructor
    GUILoadThread(MFXInterThreadEventClient *mw, MFXEventQue &eq,
        FXEX::FXThreadEvent &ev);

    /// destructor
    virtual ~GUILoadThread();

    /** starts the thread
        the thread ends after the net has been loaded */
    FXint run();

protected:
    virtual GUIEdgeControlBuilder *buildEdgeBuilder();
    virtual GUIVehicleControl *buildVehicleControl();
    virtual bool initOptions();
    virtual void initDevices();
    virtual void closeNetLoadingDependent(OptionsCont &, GUINet &) { }


    /** @brief Closes the loading process
     *
     * This method is called both on success and failure.
     * All message callbacks to this instance are removed and the parent
     * application is informed about the loading */
    void submitEndAndCleanup(GUINet *net, int simStartTime, int simEndTime);

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

