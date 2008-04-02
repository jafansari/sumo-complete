/****************************************************************************/
/// @file    GUIPerspectiveChanger.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A class that allows to steer the visual output in dependence to user
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "GUISUMOAbstractView.h"
#include "GUIPerspectiveChanger.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUIPerspectiveChanger::GUIPerspectiveChanger(GUISUMOAbstractView &callBack)
        : myCallback(callBack), myHaveChanged(true)
{}


GUIPerspectiveChanger::~GUIPerspectiveChanger()
{}


bool
GUIPerspectiveChanger::changed() const
{
    return myHaveChanged;
}


void
GUIPerspectiveChanger::otherChange()
{
    myHaveChanged = true;
}


void
GUIPerspectiveChanger::applied()
{
    myHaveChanged = false;
}


void
GUIPerspectiveChanger::setNetSizes(size_t width, size_t height)
{
    myNetWidth = width;
    myNetHeight = height;
}


void
GUIPerspectiveChanger::applyCanvasSize(size_t width, size_t height)
{
    myCanvasWidth = width;
    myCanvasHeight = height;
    otherChange();
}


void
GUIPerspectiveChanger::onLeftBtnPress(void*)
{}


void
GUIPerspectiveChanger::onLeftBtnRelease(void*)
{}


void
GUIPerspectiveChanger::onRightBtnPress(void*)
{}


bool
GUIPerspectiveChanger::onRightBtnRelease(void*)
{
    return false;
}


void
GUIPerspectiveChanger::onMouseMove(void *)
{}


FXint
GUIPerspectiveChanger::getMouseXPosition() const
{
    return myMouseXPosition;
}


FXint
GUIPerspectiveChanger::getMouseYPosition() const
{
    return myMouseYPosition;
}



/****************************************************************************/

