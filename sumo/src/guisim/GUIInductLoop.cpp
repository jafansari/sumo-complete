/****************************************************************************/
/// @file    GUIInductLoop.cpp
/// @author  Daniel Krajzewicz
/// @date    Aug 2003
/// @version $Id$
///
// The gui-version of the MSInductLoop, together with the according
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

#include <microsim/output/MSInductLoop.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/geom/Position2DVector.h>
#include "GUILaneWrapper.h"
#include "GUIInductLoop.h"
#include <utils/gui/div/GLHelper.h>
#include <utils/geom/Line2D.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <microsim/logging/FuncBinding_IntParam.h>
#include <microsim/logging/FunctionBinding.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

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
/* -------------------------------------------------------------------------
 * GUIInductLoop-methods
 * ----------------------------------------------------------------------- */
GUIInductLoop::GUIInductLoop(const std::string &id, MSLane* lane,
                             SUMOReal position,
                             SUMOTime deleteDataAfterSeconds)
        : MSInductLoop(id, lane, position, deleteDataAfterSeconds)
{}


GUIInductLoop::~GUIInductLoop()
{}


GUIDetectorWrapper *
GUIInductLoop::buildDetectorWrapper(GUIGlObjectStorage &idStorage,
                                    GUILaneWrapper &wrapper)
{
    return new MyWrapper(*this, idStorage, wrapper, posM);
}


/* -------------------------------------------------------------------------
 * GUIInductLoop::MyWrapper-methods
 * ----------------------------------------------------------------------- */
GUIInductLoop::MyWrapper::MyWrapper(GUIInductLoop &detector,
                                    GUIGlObjectStorage &idStorage,
                                    GUILaneWrapper &wrapper, SUMOReal pos)
        : GUIDetectorWrapper(idStorage, "induct loop:"+detector.getID()),
        myDetector(detector), myPosition(pos)
{
    const Position2DVector &v = wrapper.getShape();
    myFGPosition = v.positionAtLengthPosition(pos);
    Line2D l(v.getBegin(), v.getEnd());
    SUMOReal sgPos = pos / v.length() * l.length();
    myBoundary.add(myFGPosition.x()+(SUMOReal) 5.5, myFGPosition.y()+(SUMOReal) 5.5);
    myBoundary.add(myFGPosition.x()-(SUMOReal) 5.5, myFGPosition.y()-(SUMOReal) 5.5);
    myFGRotation = -v.rotationDegreeAtLengthPosition(pos);
}


GUIInductLoop::MyWrapper::~MyWrapper()
{}


Boundary
GUIInductLoop::MyWrapper::getBoundary() const
{
    return myBoundary;
}



GUIParameterTableWindow *
GUIInductLoop::MyWrapper::getParameterWindow(GUIMainWindow &app,
        GUISUMOAbstractView &/*parent !!! recheck this - never needed?*/)
{
    GUIParameterTableWindow *ret =
        new GUIParameterTableWindow(app, *this, 7);
    // add items
    ret->mkItem("flow [veh/h]", true,
                new FuncBinding_IntParam<GUIInductLoop, SUMOReal>(&(getLoop()), &GUIInductLoop::getFlow, 1));
    ret->mkItem("mean speed [m/s]", true,
                new FuncBinding_IntParam<GUIInductLoop, SUMOReal>(&(getLoop()), &GUIInductLoop::getMeanSpeed, 1));
    ret->mkItem("occupancy [%]", true,
                new FuncBinding_IntParam<GUIInductLoop, SUMOReal>(&(getLoop()), &GUIInductLoop::getOccupancy, 1));
    ret->mkItem("mean vehicle length [m]", true,
                new FuncBinding_IntParam<GUIInductLoop, SUMOReal>(&(getLoop()), &GUIInductLoop::getMeanVehicleLength, 1));
    ret->mkItem("empty time [s]", true,
                new FunctionBinding<GUIInductLoop, SUMOReal>(&(getLoop()), &GUIInductLoop::getTimestepsSinceLastDetection));
    //
    ret->mkItem("position [m]", false, myPosition);
    ret->mkItem("lane", false, myDetector.getLane()->getID());
    // close building
    ret->closeBuilding();
    return ret;
}


const std::string &
GUIInductLoop::MyWrapper::microsimID() const
{
    return myDetector.getID();
}



bool
GUIInductLoop::MyWrapper::active() const
{
    return true;
}


void
GUIInductLoop::MyWrapper::drawGL(SUMOReal scale, SUMOReal upscale)
{
    SUMOReal width = (SUMOReal) 2.0 * scale;
    glLineWidth(1.0);
    // shape
    glColor3f(1, 1, 0);
    glPushMatrix();
    glTranslated(myFGPosition.x(), myFGPosition.y(), 0);
    glRotated(myFGRotation, 0, 0, 1);
    glScaled(upscale, upscale, upscale);
    glBegin(GL_QUADS);
    glVertex2d(0-1.0, 2);
    glVertex2d(-1.0, -2);
    glVertex2d(1.0, -2);
    glVertex2d(1.0, 2);
    glEnd();
    glBegin(GL_LINES);
    // without the substracted offsets, lines are partially longer
    //  than the boxes
    glVertex2d(0, 2-.1);
    glVertex2d(0, -2+.1);
    glEnd();

    // outline
    if (width*upscale>1) {
        glColor3f(1, 1, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_QUADS);
        glVertex2f(0-1.0, 2);
        glVertex2f(-1.0, -2);
        glVertex2f(1.0, -2);
        glVertex2f(1.0, 2);
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // position indicator
    if (width*upscale>1) {
        glRotated(90, 0, 0, -1);
        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2d(0, 1.7);
        glVertex2d(0, -1.7);
        glEnd();
    }
    glPopMatrix();
}


Position2D
GUIInductLoop::MyWrapper::getPosition() const
{
    return myFGPosition;
}


GUIInductLoop &
GUIInductLoop::MyWrapper::getLoop()
{
    return myDetector;
}



/****************************************************************************/

