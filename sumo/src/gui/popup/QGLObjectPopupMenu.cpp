//---------------------------------------------------------------------------//
//                        QGLObjectPopupMenu.cpp -
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
// Revision 1.2  2003/07/18 12:30:14  dkrajzew
// removed some warnings
//
// Revision 1.1  2003/06/06 10:24:36  dkrajzew
// new subfolder holding popup-menus was added due to link-dependencies under linux; QGLObjectPopupMenu*-classes were moved to "popup"
//
// Revision 1.3  2003/06/05 11:37:31  dkrajzew
// class templates applied
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <iostream>
#include <qpopupmenu.h>
#include <gui/GUISUMOAbstractView.h>
#include <gui/GUIGlObject.h>
#include "QGLObjectPopupMenu.h"
#include <gui/partable/GUIParameterTableWindow.h>

#ifndef WIN32
#include "QGLObjectPopupMenu.moc"
#endif


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
QGLObjectPopupMenu::QGLObjectPopupMenu(GUIApplicationWindow *app,
                                       GUISUMOAbstractView *parent,
                                       GUIGlObject *o)
    : QPopupMenu(parent), myParent(parent), myObject(o),
    myApplication(app)
{
}


QGLObjectPopupMenu::~QGLObjectPopupMenu()
{
}


void
QGLObjectPopupMenu::center()
{
    myParent->centerTo(myObject->getType(), myObject->microsimID());
}


void
QGLObjectPopupMenu::showPars()
{
    // the application gets responsible for this object on building
    new GUIParameterTableWindow(myApplication, myObject);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "QGLObjectPopupMenu.icc"
//#endif

// Local Variables:
// mode:C++
// End:


