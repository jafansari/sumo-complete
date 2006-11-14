#ifndef GUIVehicle_h
#define GUIVehicle_h
//---------------------------------------------------------------------------//
//                        GUIVehicle.h -
//  A MSVehicle extended by some values for usage within the gui
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
// Revision 1.30  2006/11/14 13:01:53  dkrajzew
// warnings removed
//
// Revision 1.29  2006/09/18 10:02:18  dkrajzew
// removed deprecated c2c functions, added new made by Danilot Boyom
//
// Revision 1.28  2006/07/06 06:40:38  dkrajzew
// applied current microsim-APIs
//
// Revision 1.27  2006/04/18 08:12:04  dkrajzew
// consolidation of interaction with gl-objects
//
// Revision 1.26  2006/04/11 10:56:32  dkrajzew
// microsimID() now returns a const reference
//
// Revision 1.25  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.24  2005/12/01 07:33:44  dkrajzew
// introducing bus stops: eased building vehicles; vehicles may now have nested elements
//
// Revision 1.23  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.22  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.21  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.20  2005/05/04 08:05:24  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.19  2005/02/01 10:10:39  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.18  2004/12/16 12:20:09  dkrajzew
// debugging
//
// Revision 1.17  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.16  2004/04/02 11:20:35  dkrajzew
// changes needed to visualise the selection status
//
// Revision 1.15  2004/03/19 12:57:55  dkrajzew
// porting to FOX
//
// Revision 1.14  2004/01/26 15:53:21  dkrajzew
// added some yet unset display variables
//
// Revision 1.13  2004/01/26 07:00:50  dkrajzew
// reinserted the building of repeating vehicles
//
// Revision 1.12  2003/12/11 06:24:55  dkrajzew
// implemented MSVehicleControl as the instance responsible for vehicles
//
// Revision 1.11  2003/11/20 13:06:30  dkrajzew
// loading and using of predefined vehicle colors added
//
// Revision 1.10  2003/10/22 07:07:06  dkrajzew
// patching of lane states on force vehicle removal added
//
// Revision 1.9  2003/08/04 11:35:52  dkrajzew
// only GUIVehicles need a color definition; process of building cars changed
//
// Revision 1.8  2003/07/30 08:54:14  dkrajzew
// the network is capable to display the networks state, now
//
// Revision 1.7  2003/06/05 11:40:28  dkrajzew
// class templates applied; documentation added
//
// Revision 1.6  2003/06/05 06:29:50  dkrajzew
// first tries to build under linux: warnings removed;
//  moc-files included Makefiles added
//
// Revision 1.5  2003/05/20 09:26:57  dkrajzew
// data retrieval for new views added
//
// Revision 1.4  2003/04/14 08:27:18  dkrajzew
// new globject concept implemented
//
// Revision 1.3  2003/03/20 16:19:28  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.2  2003/02/07 10:39:17  dkrajzew
// updated
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

#include <vector>
#include <set>
#include <string>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gfx/RGBColor.h>
#include <microsim/MSVehicle.h>
#include "GUIVehicleType.h"
#include "GUIRoute.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUISUMOAbstractView;
class GUIGLObjectPopupMenu;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * A visualisable MSVehicle. Extended by the possibility to retrieve names
 * of all available vehicles (static) and the possibility to retrieve the
 * color of the vehicle which is available in different forms allowing an
 * easier recognition of done actions such as lane changing.
 */
class GUIVehicle :
                public MSVehicle,
                public GUIGlObject {
public:
    /// destructor
    ~GUIVehicle();

    inline void setOwnDefinedColor() const {
        if(hasCORNDoubleValue(MSCORN::CORN_VEH_OWNCOL_RED)) {
            glColor3d(
                getCORNDoubleValue(MSCORN::CORN_VEH_OWNCOL_RED),
                getCORNDoubleValue(MSCORN::CORN_VEH_OWNCOL_GREEN),
                getCORNDoubleValue(MSCORN::CORN_VEH_OWNCOL_BLUE));
            return;
        }
        glColor3d(1,1,0);
    }

    inline void setOwnTypeColor() const { static_cast<const GUIVehicleType&>(getVehicleType()).setColor(); }
    inline void setOwnRouteColor() const { static_cast<const GUIRoute&>(getRoute()).setColor(); }

    //@{ From GUIGlObject
    /// returns the popup-menu for vehicles
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    // Returns the parameter window
    GUIParameterTableWindow *getParameterWindow(
        GUIMainWindow &app, GUISUMOAbstractView &parent);

    /// Returns the type of the object as coded in GUIGlObjectType
    GUIGlObjectType getType() const;

    /// returns the id of the object as known to microsim
    const std::string &microsimID() const;

    /// Returns the information whether this object is still active
    bool active() const;

    /// Returns the boundary to which the object shall be centered
    Boundary getCenteringBoundary() const;
    //@}


    /** returns a random color based on the vehicle's name
        (should stay the same across simulations */
    const RGBColor &getRandomColor1() const;

    /** retunrs a random color
        (this second random color is fully randomly computed) */
    const RGBColor &getRandomColor2() const;

    /** returns a color that describes how long ago the vehicle has
        changed the lane (is white after a lane change and becomes darker
        with each timestep */
    SUMOReal getTimeSinceLastLaneChangeAsReal() const;

    /** @brief Returns the next "periodical" vehicle with the same route
        We have to duplicate the vehicle if a further has to be emitted with
        the same settings */
    virtual MSVehicle *getNextPeriodical() const;

    friend class GUIVehicleControl;

	void setRemoved();

    int getRepetitionNo() const;
    int getPeriod() const;
    size_t getLastLaneChangeOffset() const;
    size_t getDesiredDepart() const;

    /**
     * @class GUIVehiclePopupMenu
     * A popup-menu for vehicles. In comparison to the normal popup-menu, this one
     *  also allows to:
     * - show/hide the vehicle route
     */
    class GUIVehiclePopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUIVehiclePopupMenu)
    public:
        /// Constructor
        GUIVehiclePopupMenu(GUIMainWindow &app,
            GUISUMOAbstractView &parent, GUIGlObject &o);

        /// Destructor
        ~GUIVehiclePopupMenu();

        /// Called if all routes of the vehicle shall be shown
        long onCmdShowAllRoutes(FXObject*,FXSelector,void*);

        /// Called if all routes of the vehicle shall be hidden
        long onCmdHideAllRoutes(FXObject*,FXSelector,void*);

        /// Called if the current route of the vehicle shall be shown
        long onCmdShowCurrentRoute(FXObject*,FXSelector,void*);

        /// Called if the current route of the vehicle shall be hidden
        long onCmdHideCurrentRoute(FXObject*,FXSelector,void*);

        /// Called if the vehicle shall be tracked
        long onCmdStartTrack(FXObject*,FXSelector,void*);

        /// Called if the current shall not be tracked any longer
        long onCmdStopTrack(FXObject*,FXSelector,void*);

    protected:
        GUIVehiclePopupMenu() { }

    };


protected:
    /// Use this constructor only.
    GUIVehicle( GUIGlObjectStorage &idStorage,
        std::string id, MSRoute* route, SUMOTime departTime,
        const MSVehicleType* type, int repNo, int repOffset);

private:
    /// random color #1 (build from name)
    RGBColor _randomColor1;

    /// random color #2 (completely random)
    RGBColor _randomColor2;

};

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

