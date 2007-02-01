/****************************************************************************/
/// @file    GUIGradients.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 29.04.2005
/// @version $Id: $
///
//
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
#ifndef GUIGradients_h
#define GUIGradients_h

// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <utils/gfx/RGBColor.h>
#include <utils/gui/drawer/GUIGradientStorage.h>
// ===========================================================================
// variable declarations
// ===========================================================================

/// The gradient used
extern std::vector<RGBColor> myDensityGradient;

/// A storage for available gradients
extern GUIGradientStorage *gGradients;


#endif

/****************************************************************************/

