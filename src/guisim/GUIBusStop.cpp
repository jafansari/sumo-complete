/****************************************************************************/
/// @file    GUIBusStop.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 07.12.2005
/// @version $Id$
///
// A lane area vehicles can halt at (gui-version)
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

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/geom/Position2DVector.h>
#include <utils/geom/Line2D.h>
#include <utils/geom/Boundary.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/common/ToString.h>
#include <utils/common/Command.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <guisim/GUINet.h>
#include <guisim/GUIEdge.h>
#include "GUIBusStop.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include <foreign/polyfonts/polyfonts.h>
#include <utils/geom/GeomHelper.h>
#include <gui/GUIApplicationWindow.h>
#include <guisim/GUIBusStop.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
GUIBusStop::GUIBusStop(const std::string &id, MSNet &,
                       const std::vector<std::string> &lines, MSLane &lane,
                       SUMOReal frompos, SUMOReal topos) throw()
        : MSBusStop(id, lines, lane, frompos, topos),
        GUIGlObject_AbstractAdd(gIDStorage, "bus_stop:" + id, GLO_TRIGGER)
{
    // full geometry
    {
        myFGShape = lane.getShape();
        myFGShape.move2side((SUMOReal) 1.65);
        myFGShape = myFGShape.getSubpart(frompos, topos);
        myFGShapeRotations.reserve(myFGShape.size()-1);
        myFGShapeLengths.reserve(myFGShape.size()-1);
        for (size_t i=0; i<myFGShape.size()-1; ++i) {
            const Position2D &f = myFGShape[i];
            const Position2D &s = myFGShape[i+1];
            myFGShapeLengths.push_back(GeomHelper::distance(f, s));
            myFGShapeRotations.push_back((SUMOReal) atan2((s.x()-f.x()), (f.y()-s.y()))*(SUMOReal) 180.0/(SUMOReal) 3.14159265);
        }
        Position2DVector tmp = myFGShape;
        tmp.move2side(1.5);
        myFGSignPos = tmp.center();
    }
}


GUIBusStop::~GUIBusStop() throw()
{}


GUIGLObjectPopupMenu *
GUIBusStop::getPopUpMenu(GUIMainWindow &app,
                         GUISUMOAbstractView &parent)
{
    GUIGLObjectPopupMenu *ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    return ret;
}


GUIParameterTableWindow *
GUIBusStop::getParameterWindow(GUIMainWindow &,
                               GUISUMOAbstractView &)
{
    return 0;
}


const std::string &
GUIBusStop::microsimID() const
{
    return getID();
}


bool
GUIBusStop::active() const
{
    return true;
}


Position2D
GUIBusStop::getPosition() const
{
    return myFGShape.center(); //!!!!
}


void
GUIBusStop::drawGL(SUMOReal scale, SUMOReal upscale)
{
    // draw the area
    glColor3f((SUMOReal)(76./255.), (SUMOReal)(170./255.), (SUMOReal)(50./255.));
    size_t i;
    GLHelper::drawBoxLines(myFGShape, myFGShapeRotations, myFGShapeLengths, 1.0);
    // draw the lines
    if (scale>=35) {
        glColor3f((SUMOReal)(76./255.), (SUMOReal)(170./255.), (SUMOReal)(50./255.));
        for (i=0; i!=myLines.size(); ++i) {
            glPushMatrix();
            glScaled(upscale, upscale, upscale);
            glTranslated(myFGSignPos.x()+1.2, myFGSignPos.y()+0.8, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            pfSetPosition(0, 0);
            pfSetScale(.5f);
            //glRotated(180, 0, 1, 0);
            glTranslated(0, -(double)i*.5, 0);
            glScaled(1, -1, 1);
            pfDrawString(myLines[i].c_str());
            glPopMatrix();
        }
    }

    // draw the sign
    glPushMatrix();
    glScaled(upscale, upscale, upscale);
    glTranslated(myFGSignPos.x(), myFGSignPos.y(), 0);
    //glRotated( rot, 0, 0, 1 );
    //glTranslated(0, -1.5, 0);
    int noPoints = 9;
    if (scale>25) {
        noPoints = (int)(9.0 + scale / 10.0);
        if (noPoints>36) {
            noPoints = 36;
        }
    }
    glColor3f((SUMOReal)(76./255.), (SUMOReal)(170./255.), (SUMOReal)(50./255.));
    GLHelper::drawFilledCircle((SUMOReal) 1.1, noPoints);
    if (scale<10) {
        glPopMatrix();
        return;
    }
    glColor3f((SUMOReal)(255./255.), (SUMOReal)(235./255.), (SUMOReal)(0./255.));
    GLHelper::drawFilledCircle((SUMOReal) 0.9, noPoints);
    // draw the H
    // not if scale to low
    if (scale<4.5) {
        glPopMatrix();
        return;
    }
    glColor3f((SUMOReal)(76./255.), (SUMOReal)(170./255.), (SUMOReal)(50./255.));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    pfSetPosition(0, 0);
    pfSetScale(1.6f);
    SUMOReal w = pfdkGetStringWidth("H");
    glRotated(180, 0, 1, 0);
    glTranslated(-w/2., 0.4, 0);
    pfDrawString("H");
    glPopMatrix();
}


Boundary
GUIBusStop::getBoundary() const
{
    /* !!! */
    Position2D pos = getPosition();
    Boundary ret(pos.x(), pos.y(), pos.x(), pos.y());
    ret.grow(2.0);
    return ret;
}


GUIManipulator *
GUIBusStop::openManipulator(GUIMainWindow &,
                            GUISUMOAbstractView &)
{
    /*
    GUIManip_LaneSpeedTrigger *gui =
        new GUIManip_LaneSpeedTrigger(app, getFullName(), *this, 0, 0);
    gui->create();
    gui->show();
    return gui;
    */
    return 0;
}



/****************************************************************************/

