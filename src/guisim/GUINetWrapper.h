/****************************************************************************/
/// @file    GUINetWrapper.h
/// @author  Daniel Krajzewicz
/// @date
/// @version $Id$
///
// No geometrical information is hold, here. Still, some methods for
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef GUINetWrapper_h
#define GUINetWrapper_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <utility>
#include <utils/geom/Position2DVector.h>
#include <utils/geom/HaveBoundary.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/globjects/GUIGlObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUINet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 */
class GUINetWrapper :
            public GUIGlObject,
            public HaveBoundary
{
public:
    /// constructor
    GUINetWrapper(GUIGlObjectStorage &idStorage,
                  GUINet &net);

    /// destructor
    virtual ~GUINetWrapper();

    /// @name inherited from GUIGlObject
    //@{
    /// Returns the popup-menu
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
                                       GUISUMOAbstractView &parent);

    /// Returns the parameter window
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

    Boundary getBoundary() const;

    GUINet &getNet() const;

protected:
    GUINet &myNet;

};


#endif

/****************************************************************************/

