//---------------------------------------------------------------------------//
//                        GUIJunctionWrapper.h -
//  Holds geometrical values for a junction
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 1 Jul 2003
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
// Revision 1.18  2006/03/28 06:12:54  dkrajzew
// unneeded string wrapping removed
//
// Revision 1.17  2006/03/17 11:03:04  dkrajzew
// made access to positions in Position2DVector c++ compliant
//
// Revision 1.16  2006/01/31 10:55:27  dkrajzew
// unneeded inclusions removed
//
// Revision 1.15  2005/11/09 06:35:34  dkrajzew
// debugging
//
// Revision 1.14  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.13  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.12  2005/05/04 07:59:59  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.11  2004/12/20 14:00:23  dkrajzew
// debugging
//
// Revision 1.10  2004/12/12 17:23:58  agaubatz
// Editor Tool Widgets included
//
// Revision 1.9  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.8  2004/07/02 08:54:11  dkrajzew
// some design issues
//
// Revision 1.7  2004/04/02 11:18:37  dkrajzew
// recenter view - icon added to the popup menu
//
// Revision 1.6  2004/03/19 12:57:54  dkrajzew
// porting to FOX
//
// Revision 1.5  2003/12/09 11:27:50  dkrajzew
// removed some dead code
//
// Revision 1.4  2003/08/14 13:47:44  dkrajzew
// false usage of function-pointers patched; false inclusion of .moc-files
//  removed
//
// Revision 1.3  2003/07/30 08:54:14  dkrajzew
// the network is capable to display the networks state, now
//
// Revision 1.2  2003/07/16 15:24:55  dkrajzew
// GUIGrid now handles the set of things to draw in another manner than
//  GUIEdgeGrid did; Further things to draw implemented
//
// Revision 1.1  2003/07/07 08:14:48  dkrajzew
// first steps towards the usage of a real lane and junction geometry
//  implemented
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
#include <utility>
#include <utils/foxtools/FXMutex.h>
#include <microsim/MSLane.h>
#include <utils/geom/Position2D.h>
#include <microsim/MSNet.h>
#include <gui/GUIApplicationWindow.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include "GUIJunctionWrapper.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/foxtools/MFXMenuHeader.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIParameterTableWindow.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
GUIJunctionWrapper::GUIJunctionWrapper( GUIGlObjectStorage &idStorage,
        MSJunction &junction,
        const Position2DVector &shape)
    : GUIGlObject(idStorage, "junction:"+junction.id()),
    myJunction(junction), myShape(shape)
{
}


GUIJunctionWrapper::~GUIJunctionWrapper()
{
}


GUIGLObjectPopupMenu *
GUIJunctionWrapper::getPopUpMenu(GUIMainWindow &app,
                                 GUISUMOAbstractView &parent)
{
    GUIGLObjectPopupMenu *ret = new GUIGLObjectPopupMenu(app, parent, *this);
    new MFXMenuHeader(ret, app.getBoldFont(), getFullName().c_str(), 0, 0, 0);
    new FXMenuSeparator(ret);
    //
    new FXMenuCommand(ret, "Center",
        GUIIconSubSys::getIcon(ICON_RECENTERVIEW), ret, MID_CENTER);
    new FXMenuSeparator(ret);
    //
    if(gSelected.isSelected(GLO_JUNCTION, getGlID())) {
        new FXMenuCommand(ret, "Remove From Selected",
            GUIIconSubSys::getIcon(ICON_FLAG_MINUS), ret, MID_REMOVESELECT);
    } else {
        new FXMenuCommand(ret, "Add To Selected",
            GUIIconSubSys::getIcon(ICON_FLAG_PLUS), ret, MID_ADDSELECT);
    }
    return ret;
}


GUIParameterTableWindow *
GUIJunctionWrapper::getParameterWindow(GUIMainWindow &app,
                                       GUISUMOAbstractView &parent)
{
	std::vector<std::string> s=myJunction.getNames();
	int NumberOfRows =s.size();
	GUIParameterTableWindow *ret =
		new GUIParameterTableWindow(app, *this, 2+NumberOfRows);
    // add items
	//ret->mkItem("length [m]", false, myJunction.length());
    ret->mkItem("x-Position", false, myJunction.getPosition().x());
	ret->mkItem("y-Position", false, myJunction.getPosition().y());

	//std::vector<string>::iterator p=s.begin();
	/*
	while(p !=s.end())
	{
		ret->mkItem("name", false, s[]);
		p++;
	}
	*/
    for(unsigned int i=0 ; i<s.size(); i++)
	{
		ret->mkItem("name", false, s[i]);
	}

    // close building
    ret->closeBuilding();
    return ret;
}



GUIGlObjectType
GUIJunctionWrapper::getType() const
{
    return GLO_JUNCTION;
}


std::string
GUIJunctionWrapper::microsimID() const
{
    return myJunction.id();
}


Boundary
GUIJunctionWrapper::getBoundary() const
{
    Boundary boundary;
    size_t shapeLength = myShape.size();
    for(size_t i=0; i<shapeLength; i++) {
        const Position2D &pos = myShape[i];
        boundary.add(pos.x(), pos.y());
    }
    return boundary;
}


const Position2DVector &
GUIJunctionWrapper::getShape() const
{
    return myShape;
}


Boundary
GUIJunctionWrapper::getCenteringBoundary() const
{
	Boundary b = getBoundary();
	b.grow(20);
	return b;
}

MSJunction &
GUIJunctionWrapper::getJunction() const
{
	return myJunction;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


