/****************************************************************************/
/// @file    GUIE3Collector.cpp
/// @author  Daniel Krajzewicz
/// @date    Jan 2004
/// @version $Id$
///
// The gui-version of a MSE3Collector
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

#include "GUIE3Collector.h"
#include "GUIEdge.h"
#include <utils/geom/Line2D.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/images/GUITexturesHelper.h>
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
 * GUIE3Collector::MyWrapper-methods
 * ----------------------------------------------------------------------- */
GUIE3Collector::MyWrapper::MyWrapper(GUIE3Collector &detector,
                                     GUIGlObjectStorage &idStorage)
        : GUIDetectorWrapper(idStorage, "E3 detector:"+detector.getID()),
        myDetector(detector)
{
    const CrossSectionVector &entries = detector.getEntries();
    const CrossSectionVector &exits = detector.getExits();
    CrossSectionVectorConstIt i;
    for (i=entries.begin(); i!=entries.end(); ++i) {
        SingleCrossingDefinition def = buildDefinition(*i);
        myBoundary.add(def.myFGPosition);
        myEntryDefinitions.push_back(def);
    }
    for (i=exits.begin(); i!=exits.end(); ++i) {
        SingleCrossingDefinition def = buildDefinition(*i);
        myBoundary.add(def.myFGPosition);
        myExitDefinitions.push_back(def);
    }
}


GUIE3Collector::MyWrapper::~MyWrapper()
{}


GUIE3Collector::MyWrapper::SingleCrossingDefinition
GUIE3Collector::MyWrapper::buildDefinition(const MSCrossSection &section)
{
    const MSLane *lane = section.myLane;
    SUMOReal pos = section.myPosition;
    const Position2DVector &v =
        static_cast<const GUIEdge * const>(lane->getEdge())->getLaneGeometry(static_cast<const MSLane*>(lane)).getShape();
    Line2D l(v.getBegin(), v.getEnd());

    SingleCrossingDefinition def;
    def.myFGPosition = v.positionAtLengthPosition(pos);
    def.myFGRotation = -v.rotationDegreeAtLengthPosition(pos);
    return def;
}


Boundary
GUIE3Collector::MyWrapper::getBoundary() const
{
    return myBoundary;
}


GUIParameterTableWindow *
GUIE3Collector::MyWrapper::getParameterWindow(GUIMainWindow &app,
        GUISUMOAbstractView &)
{
    GUIParameterTableWindow *ret =
        new GUIParameterTableWindow(app, *this, 3);
    // add items
        // values
    ret->mkItem("touched vehicles [#]", true,
                new FunctionBinding<MSE3Collector, SUMOReal>(&myDetector, &MSE3Collector::getCurrentTouchedVehicles));
    ret->mkItem("mean speed [m/s]", true,
                new FunctionBinding<MSE3Collector, SUMOReal>(&myDetector, &MSE3Collector::getCurrentMeanSpeed));
    ret->mkItem("haltings [#]", true,
                new FunctionBinding<MSE3Collector, SUMOReal>(&myDetector, &MSE3Collector::getCurrentHaltingNumber));
    // close building
    ret->closeBuilding();
    return ret;
}


const std::string &
GUIE3Collector::MyWrapper::microsimID() const
{
    return myDetector.getID();
}


bool
GUIE3Collector::MyWrapper::active() const
{
    return true;
}


void
GUIE3Collector::MyWrapper::drawGL(SUMOReal /*scale*/, SUMOReal upscale)
{
    typedef std::vector<SingleCrossingDefinition> CrossingDefinitions;
    CrossingDefinitions::const_iterator i;
    glColor3d(0, .8, 0);
    for (i=myEntryDefinitions.begin(); i!=myEntryDefinitions.end(); ++i) {
        drawSingleCrossing((*i).myFGPosition, (*i).myFGRotation, upscale);
    }
    glColor3d(.8, 0, 0);
    for (i=myExitDefinitions.begin(); i!=myExitDefinitions.end(); ++i) {
        drawSingleCrossing((*i).myFGPosition, (*i).myFGRotation, upscale);
    }
}


void
GUIE3Collector::MyWrapper::drawSingleCrossing(const Position2D &pos,
        SUMOReal rot, SUMOReal upscale) const
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPushMatrix();
    glScaled(upscale, upscale, upscale);
    glTranslated(pos.x(), pos.y(), 0);
    glRotated(rot, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(1.7, 0);
    glVertex2d(-1.7, 0);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2d(-1.7, .5);
    glVertex2d(-1.7, -.5);
    glVertex2d(1.7, -.5);
    glVertex2d(1.7, .5);
    glEnd();
    // arrows
    glTranslated(1.5, 0, 0);
    GUITexturesHelper::drawDirectionArrow(TEXTURE_LINKDIR_STRAIGHT, 1.0);
    glTranslated(-3, 0, 0);
    GUITexturesHelper::drawDirectionArrow(TEXTURE_LINKDIR_STRAIGHT, 1.0);
    glPopMatrix();
}


Position2D
GUIE3Collector::MyWrapper::getPosition() const
{
    return myBoundary.getCenter();
}


GUIE3Collector &
GUIE3Collector::MyWrapper::getDetector()
{
    return myDetector;
}


/* -------------------------------------------------------------------------
 * GUIE3Collector-methods
 * ----------------------------------------------------------------------- */
GUIE3Collector::GUIE3Collector(const std::string &id,
                               const CrossSectionVector &entries,  const CrossSectionVector &exits,
                               MetersPerSecond haltingSpeedThreshold,
                               SUMOTime haltingTimeThreshold) throw()
        : MSE3Collector(id, entries,  exits, haltingSpeedThreshold, haltingTimeThreshold)
{}


GUIE3Collector::~GUIE3Collector() throw()
{}


const CrossSectionVector &
GUIE3Collector::getEntries() const
{
    return myEntries;
}


const CrossSectionVector &
GUIE3Collector::getExits() const
{
    return myExits;
}



GUIDetectorWrapper *
GUIE3Collector::buildDetectorWrapper(GUIGlObjectStorage &idStorage)
{
    return new MyWrapper(*this, idStorage);
}



/****************************************************************************/

