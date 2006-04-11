#ifndef GUITrafficLightLogicWrapper_h
#define GUITrafficLightLogicWrapper_h
//---------------------------------------------------------------------------//
//                        GUITrafficLightLogicWrapper.h -
//  A wrapper for tl-logics to allow their visualisation and interaction
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Oct/Nov 2003
//  copyright            : (C) 2003 by Daniel Krajzewicz
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
// Revision 1.9  2006/04/11 10:56:32  dkrajzew
// microsimID() now returns a const reference
//
// Revision 1.8  2006/02/23 11:27:56  dkrajzew
// tls may have now several programs
//
// Revision 1.7  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/01/27 14:20:26  dkrajzew
// ability to open the complete phase definition added
//
// Revision 1.4  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.3  2004/03/19 12:57:55  dkrajzew
// porting to FOX
//
// Revision 1.2  2003/11/26 09:48:58  dkrajzew
// time display added to the tl-logic visualisation
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

#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/tracker/GUITLLogicPhasesTrackerWindow.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIGlObjectStorage;
class MSTrafficLightLogic;
class GUIMainWindow;


/* =========================================================================
 * class definition
 * ======================================================================= */
/**
 * @class GUITrafficLightLogicWrapper
 * This class is responsible for the visualisation of tl-logics and the
 *  interaction with them.
 */
class GUITrafficLightLogicWrapper
    : public GUIGlObject {
public:
    /// Constructor
    GUITrafficLightLogicWrapper(GUIGlObjectStorage &idStorage,
        MSTLLogicControl &control, MSTrafficLightLogic &tll);

    /// Destructor
    ~GUITrafficLightLogicWrapper();

    /// Returns an own popup-menu
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// Returns an own parameter window
    GUIParameterTableWindow *getParameterWindow(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// Returns the type of the object as coded in GUIGlObjectType
    GUIGlObjectType getType() const;

    /// returns the id of the object as known to microsim
    const std::string &microsimID() const;

    /// Returns the information whether this object is still active
	bool active() const;

    /// Returns the current phase definition
    CompletePhaseDef getPhaseDef() const;

    /// Builds a GUITLLogicPhasesTrackerWindow which will receive new phases
    void begin2TrackPhases();

    /// Builds a GUITLLogicPhasesTrackerWindow which displays the phase diagram
    void showPhases();

    /// Builds a GUITLLogicPhasesTrackerWindow which displays the phase diagram
    void switchTLSLogic(int to);

	//{
	Boundary getCenteringBoundary() const;
	//}

public:
    /**
     * @class GUITrafficLightLogicWrapperPopupMenu
     * The popup-menu for a TLS-logic. Adds the functionality to open a
     *  view on the tls-logic and to start tracking of the tls-logic.
     */
    class GUITrafficLightLogicWrapperPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUITrafficLightLogicWrapperPopupMenu)
    public:
        /// Constructor
        GUITrafficLightLogicWrapperPopupMenu(GUIMainWindow &app,
            GUISUMOAbstractView &parent, GUIGlObject &o);

        /// Destructor
        ~GUITrafficLightLogicWrapperPopupMenu();

        /// Called if the phases shall be shown
        long onCmdShowPhases(FXObject*,FXSelector,void*);

        /// Called if the phases shall be begun to track
        long onCmdBegin2TrackPhases(FXObject*,FXSelector,void*);

        long onCmdSwitchTLSLogic(FXObject*,FXSelector,void*);

    protected:
        /// protected constructor for FOX
        GUITrafficLightLogicWrapperPopupMenu() { }

    };

private:
    /// The wrapped tl-logic
    MSTrafficLightLogic &myTLLogic;

    MSTLLogicControl &myTLLogicControl;

    /// The main application
    GUIMainWindow *myApp;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
