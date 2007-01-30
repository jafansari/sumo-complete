#ifndef GUIPerspectiveChanger_h
#define GUIPerspectiveChanger_h
//---------------------------------------------------------------------------//
//                        GUIPerspectiveChanger.h -
//  A virtual class that allows to steer the visual output in dependence to
//      user interaction
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
// Revision 1.6  2006/07/06 05:54:11  dkrajzew
// refactoring
//
// Revision 1.5  2005/10/07 11:46:08  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/23 06:11:14  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.3  2005/09/15 12:20:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/05/04 09:24:43  dkrajzew
// entries for viewport definition added; popups now popup faster
//
// Revision 1.1  2004/11/23 10:38:32  dkrajzew
// debugging
//
// Revision 1.2  2004/10/29 06:01:56  dksumo
// renamed boundery to boundary
//
// Revision 1.1  2004/10/22 12:50:57  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.7  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.6  2003/07/07 08:11:16  dkrajzew
// documentation patched to fit into a 80-char display
//
// Revision 1.5  2003/05/20 09:23:54  dkrajzew
// some statistics added; some debugging done
//
// Revision 1.4  2003/04/04 08:37:50  dkrajzew
// view centering now applies net size; closing problems debugged; comments added; tootip button added
//
// Revision 1.3  2003/03/12 16:55:18  dkrajzew
// centering of objects debugged
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

#include <fx.h>
#include "GUISUMOAbstractView.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUISUMOAbstractView;
class Position2D;
class Boundary;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUIPerspectiveChanger
 * This is the interface for implementation of own classes that handle the
 * interaction between the user and a display field.
 * While most of our (IVF) interfaces allow zooming by choosing the rectangle
 * to show, other types of interaction are possible and have been implemented.
 * To differ between the behaviours, all types of interaction between the
 * user and the canvas are send to this class: mouse moving, mouse button
 * pressing and releasing.
 */
class GUIPerspectiveChanger {
public:
    enum MouseState {
        MOUSEBTN_NONE = 0,
        MOUSEBTN_LEFT = 1,
        MOUSEBTN_RIGHT = 2,
        MOUSEBTN_MIDDLE = 4
    };

    /// Constructor
    GUIPerspectiveChanger(GUISUMOAbstractView &callBack);

    /// Destructor
    virtual ~GUIPerspectiveChanger();

    virtual void onLeftBtnPress(void *data);
    virtual void onLeftBtnRelease(void *data);
    virtual void onRightBtnPress(void *data);
    virtual bool onRightBtnRelease(void *data);
    virtual void onMouseMove(void *data);
//    virtual void onMouseLeft();

    /// Returns the rotation of the canvas stored in this changer
    virtual SUMOReal getRotation() const = 0;

    /// Returns the x-offset of the field to show stored in this changer
    virtual SUMOReal getXPos() const = 0;

    /// Returns the y-offset of the field to show stored in this changer
    virtual SUMOReal getYPos() const = 0;

    /// Returns the zoom factor computed stored in this changer
    virtual SUMOReal getZoom() const = 0;

    /// Returns the information whether the view has chnaged
    bool changed() const;

    /** @brief Informs the changer about other chnages (window scaling etc.)
        This only resets the information that "change" has to return true */
    void otherChange();

    /// recenters the view to display the whole network
    virtual void recenterView() = 0;

    /// Informs the changer that the view has beend adapted to changes
    void applied();

    /** @brief Centers the view to the given position,
        setting it to a size that covers the radius.
        Used for: Centering of vehicles and junctions */
    virtual void centerTo(const Boundary &netBoundary,
        const Position2D &pos, SUMOReal radius, bool applyZoom=true) = 0;

    /** @brief Centers the view to show the given boundary
        Used for: Centering of lanes */
    virtual void centerTo(const Boundary &netBoundary,
        Boundary bound, bool applyZoom=true) = 0;

    /** @brief Sets the viewport
        Used for: Adapting a new viewport */
    virtual void setViewport(SUMOReal zoom, SUMOReal xPos, SUMOReal yPos) = 0;

    /// Returns the last mouse x-position an event occured at
    int getMouseXPosition() const;

    /// Returns the last mouse y-position an event occured at
    int getMouseYPosition() const;

    /// Sets the sizes of the network
    void setNetSizes(size_t width, size_t height);

    /// Informs the changer aboud the size of the canvas
    void applyCanvasSize(size_t width, size_t height);

protected:
    /// The parent window (canvas to scale)
    GUISUMOAbstractView &myCallback;

    /// Information whether the view has changed
    bool myHaveChanged;

    /// The sizes of the network
    size_t myNetWidth, myNetHeight;

    /// The sizes of the canvas
    size_t myCanvasWidth, myCanvasHeight;

    /// the current mouse position
    int myMouseXPosition, myMouseYPosition;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

