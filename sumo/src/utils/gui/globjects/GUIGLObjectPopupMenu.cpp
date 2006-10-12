//---------------------------------------------------------------------------//
//                        GUIGLObjectPopupMenu.cpp -
//  The popup menu which is displayed when pressing the right mouse button over
//  a gl-object
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.6  2006/10/12 07:57:14  dkrajzew
// added the possibility to copy an artefact's (gl-object's) name to clipboard (windows)
//
// Revision 1.5  2006/04/18 08:08:21  dkrajzew
// added Danilot Tete-Boyoms poi-interaction
//
// Revision 1.4  2005/10/07 11:45:32  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 12:19:44  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/05/04 09:19:27  dkrajzew
// adding of surrounding lanes to selection
//
// Revision 1.1  2004/11/23 10:38:31  dkrajzew
// debugging
//
// Revision 1.2  2004/10/29 06:20:47  dksumo
// patched some false dependencies
//
// Revision 1.1  2004/10/22 12:50:50  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.2  2004/07/02 08:25:32  dkrajzew
// possibility to manipulate objects added
//
// Revision 1.1  2004/03/19 12:41:13  dkrajzew
// porting to FOX
//
// Revision 1.4  2003/11/12 14:09:39  dkrajzew
// clean up after recent changes; comments added
//
// Revision 1.3  2003/07/30 08:49:26  dkrajzew
// changed the responsibility of a GLObject
//
// Revision 1.2  2003/07/18 12:30:14  dkrajzew
// removed some warnings
//
// Revision 1.1  2003/06/06 10:24:36  dkrajzew
// new subfolder holding popup-menus was added due to link-dependencies
//  under linux; GUIGLObjectPopupMenu*-classes were moved to "popup"
//
// Revision 1.3  2003/06/05 11:37:31  dkrajzew
// class templates applied
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

#include <iostream>
#include <cassert>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <guisim/GUITrafficLightLogicWrapper.h>
#include "GUIGLObjectPopupMenu.h"
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/drawer/GUILaneRepresentation.h>
#include <utils/gui/div/GUIUserIO.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * FOX callback mapping
 * ======================================================================= */
FXDEFMAP(GUIGLObjectPopupMenu) GUIGLObjectPopupMenuMap[]=
{
    FXMAPFUNC(SEL_COMMAND,  MID_CENTER,         GUIGLObjectPopupMenu::onCmdCenter),
    FXMAPFUNC(SEL_COMMAND,  MID_COPY_NAME,       GUIGLObjectPopupMenu::onCmdCopyName),
    FXMAPFUNC(SEL_COMMAND,  MID_COPY_TYPED_NAME, GUIGLObjectPopupMenu::onCmdCopyTypedName),
    FXMAPFUNC(SEL_COMMAND,  MID_SHOWPARS,       GUIGLObjectPopupMenu::onCmdShowPars),
    FXMAPFUNC(SEL_COMMAND,  MID_ADDSELECT,      GUIGLObjectPopupMenu::onCmdAddSelected),
    FXMAPFUNC(SEL_COMMAND,  MID_ADDSELECT_SUCC, GUIGLObjectPopupMenu::onCmdAddSuccessorsSelected),
    FXMAPFUNC(SEL_COMMAND,  MID_REMOVESELECT,   GUIGLObjectPopupMenu::onCmdRemoveSelected),
	FXMAPFUNC(SEL_COMMAND,  MID_RENAME,         GUIGLObjectPopupMenu::onCmdRename),
    FXMAPFUNC(SEL_COMMAND,  MID_MOVETO,         GUIGLObjectPopupMenu::onCmdMoveTo),
    FXMAPFUNC(SEL_COMMAND,  MID_CHANGECOL,      GUIGLObjectPopupMenu::onCmdChangeCol),
    FXMAPFUNC(SEL_COMMAND,  MID_CHANGETYPE,     GUIGLObjectPopupMenu::onCmdChangeTyp),
    FXMAPFUNC(SEL_COMMAND,  MID_DELETE,         GUIGLObjectPopupMenu::onCmdDelete)
};

// Object implementation
FXIMPLEMENT(GUIGLObjectPopupMenu, FXMenuPane, GUIGLObjectPopupMenuMap, ARRAYNUMBER(GUIGLObjectPopupMenuMap))


/* =========================================================================
 * method definitions
 * ======================================================================= */
GUIGLObjectPopupMenu::GUIGLObjectPopupMenu(GUIMainWindow &app,
                                           GUISUMOAbstractView &parent,
                                           GUIGlObject &o)
    : FXMenuPane(&parent), myParent(&parent), myObject(&o),
    myApplication(&app)
{
}


GUIGLObjectPopupMenu::~GUIGLObjectPopupMenu()
{
}


long
GUIGLObjectPopupMenu::onCmdCenter(FXObject*,FXSelector,void*)
{
    myParent->centerTo(myObject);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdCopyName(FXObject*,FXSelector,void*)
{
    GUIUserIO::copyToClipboard(myParent->getApp(), myObject->microsimID());
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdCopyTypedName(FXObject*,FXSelector,void*)
{
    GUIUserIO::copyToClipboard(myParent->getApp(), myObject->getFullName());
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdShowPars(FXObject*,FXSelector,void*)
{
    GUIParameterTableWindow *w =
        myObject->getParameterWindow(*myApplication, *myParent);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdAddSelected(FXObject*,FXSelector,void*)
{
    gSelected.select(myObject->getType(), myObject->getGlID());
    myParent->update();
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdAddSuccessorsSelected(FXObject*,FXSelector,void*)
{
    GUILaneRepresentation *lane = static_cast<GUILaneRepresentation*>(myObject);
    lane->selectSucessors();
    myParent->update();
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdRemoveSelected(FXObject*,FXSelector,void*)
{
    gSelected.deselect(myObject->getType(), myObject->getGlID());
    myParent->update();
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdRename(FXObject*,FXSelector,void*)
{
    myParent->rename(myObject);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdMoveTo(FXObject*,FXSelector,void*)
{
    myParent->moveTo(myObject);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdChangeCol(FXObject*,FXSelector,void*)
{
    myParent->changeCol(myObject);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdChangeTyp(FXObject*,FXSelector,void*)
{
    myParent->changeTyp(myObject);
    return 1;
}


long
GUIGLObjectPopupMenu::onCmdDelete(FXObject*,FXSelector,void*)
{
    myParent->deleteObj(myObject);
    return 1;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


