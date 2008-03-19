/****************************************************************************/
/// @file    GUIDetectorWrapper.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The base class for detector wrapper
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

#include "GUIDetectorWrapper.h"
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
GUIDetectorWrapper::GUIDetectorWrapper(GUIGlObjectStorage &idStorage,
                                       std::string id) throw()
        : GUIGlObject_AbstractAdd(idStorage, id, GLO_DETECTOR)
{}


GUIDetectorWrapper::GUIDetectorWrapper(GUIGlObjectStorage &idStorage,
                                       std::string id,
                                       GLuint glID) throw()
        : GUIGlObject_AbstractAdd(idStorage, id, glID, GLO_DETECTOR)
{}


GUIDetectorWrapper::~GUIDetectorWrapper() throw()
{}


GUIGLObjectPopupMenu *
GUIDetectorWrapper::getPopUpMenu(GUIMainWindow &app,
                                 GUISUMOAbstractView &parent) throw()
{
    GUIGLObjectPopupMenu *ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret, false);
    return ret;
}


GUIGlObjectType
GUIDetectorWrapper::getType() const throw()
{
    return GLO_DETECTOR;
}



/****************************************************************************/

