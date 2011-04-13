/****************************************************************************/
/// @file    GUISUMOAbstractView.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The base class for a view
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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

#include <iostream>
#include <utility>
#include <cmath>
#include <cassert>
#include <foreign/gl2ps/gl2ps.h>
#include <utils/common/RGBColor.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/gui/globjects/GUIGLObjectToolTip.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include "GUIDanielPerspectiveChanger.h"
#include "GUISUMOAbstractView.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include "GUIMainWindow.h"
#include "GUIGlChildWindow.h"
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include "GUIDialog_EditViewport.h"
#include <foreign/polyfonts/polyfonts.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <utils/gui/globjects/GUIPolygon2D.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <fxkeys.h>
#include <utils/foxtools/MFXImageHelper.h>

#include <microsim/MSNet.h>


#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <utils/foxtools/FXSingleEventThread.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - FOX callback mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUISUMOAbstractView) GUISUMOAbstractViewMap[]= {
    FXMAPFUNC(SEL_CONFIGURE,           0,                 GUISUMOAbstractView::onConfigure),
    FXMAPFUNC(SEL_PAINT,               0,                 GUISUMOAbstractView::onPaint),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS,     0,                 GUISUMOAbstractView::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,   0,                 GUISUMOAbstractView::onLeftBtnRelease),
    FXMAPFUNC(SEL_RIGHTBUTTONPRESS,    0,                 GUISUMOAbstractView::onRightBtnPress),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,  0,                 GUISUMOAbstractView::onRightBtnRelease),
    FXMAPFUNC(SEL_MOUSEWHEEL,          0,                 GUISUMOAbstractView::onMouseWheel),
    FXMAPFUNC(SEL_MOTION,              0,                 GUISUMOAbstractView::onMouseMove),
    FXMAPFUNC(SEL_LEAVE,               0,                 GUISUMOAbstractView::onMouseLeft),
    FXMAPFUNC(SEL_KEYPRESS,            0,                 GUISUMOAbstractView::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,          0,                 GUISUMOAbstractView::onKeyRelease),

};


FXIMPLEMENT_ABSTRACT(GUISUMOAbstractView,FXGLCanvas,GUISUMOAbstractViewMap,ARRAYNUMBER(GUISUMOAbstractViewMap))


/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - methods
 * ----------------------------------------------------------------------- */
GUISUMOAbstractView::GUISUMOAbstractView(FXComposite *p,
        GUIMainWindow &app,
        GUIGlChildWindow *parent,
        const SUMORTree &grid,
        FXGLVisual *glVis, FXGLCanvas *share)
        : FXGLCanvas(p, glVis, share, p, MID_GLCANVAS,
                     LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0),
        myApp(&app),
        myParent(parent),
        myGrid(&((SUMORTree&)grid)),
        myChanger(0),
        myMouseHotspotX(app.getDefaultCursor()->getHotX()),
        myMouseHotspotY(app.getDefaultCursor()->getHotY()),
        myPopup(0),
        myUseToolTips(false),
        myAmInitialised(false),
        myViewportChooser(0), 
        myVisualizationChanger(0) {
    setTarget(this);
    enable();
    flags|=FLAG_ENABLED;
    myInEditMode=false;
    // compute the net scale
    SUMOReal nw = myGrid->getWidth();
    SUMOReal nh = myGrid->getHeight();
    myNetScale = (nw < nh ? nh : nw);
    // show the middle at the beginning
    myChanger = new GUIDanielPerspectiveChanger(*this);
    gSchemeStorage.setViewport(this);
    myToolTip = new GUIGLObjectToolTip(myApp);
    myVisualizationSettings = &gSchemeStorage.getDefault();
}


GUISUMOAbstractView::~GUISUMOAbstractView() {
    gSchemeStorage.setDefault(myVisualizationSettings->name);
    gSchemeStorage.saveViewport(myChanger->getXPos(), myChanger->getYPos(), myChanger->getZoom());
    delete myChanger;
    delete myToolTip;
    delete myViewportChooser;
    delete myVisualizationChanger;
}


bool
GUISUMOAbstractView::isInEditMode() {
    return myInEditMode;
}


void
GUISUMOAbstractView::updateToolTip() {
    if (!myUseToolTips) {
        return;
    }
    update();
}


Position2D
GUISUMOAbstractView::getPositionInformation() const {
    return getPositionInformation(
            myWindowCursorPositionX + myMouseHotspotX, 
            myWindowCursorPositionY + myMouseHotspotY);
}


Position2D
GUISUMOAbstractView::getPositionInformation(int mx, int my) const {
    // compute the offset
    SUMOReal cy = myChanger->getYPos();
    SUMOReal cx = myChanger->getXPos();
    // compute the visible area
    GLdouble sxmin = myGrid->getCenter().x() - cx - myShownNetworkHalfWidth;
    GLdouble sxmax = myGrid->getCenter().x() - cx + myShownNetworkHalfWidth;
    GLdouble symin = myGrid->getCenter().y() - cy - myShownNetworkHalfHeight;
    GLdouble symax = myGrid->getCenter().y() - cy + myShownNetworkHalfHeight;
    SUMOReal sx = sxmin + (sxmax-sxmin) * (SUMOReal) mx / (SUMOReal) myWidthInPixels;
    SUMOReal sy = symin + (symax-symin) * ((SUMOReal) myHeightInPixels - (SUMOReal) my) / (SUMOReal) myHeightInPixels;
    return Position2D(sx, sy);
}


void
GUISUMOAbstractView::updatePositionInformation() const {
    Position2D pos = getPositionInformation();
    std::string text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    myApp->getCartesianLabel().setText(text.c_str());
    GeoConvHelper::cartesian2geo(pos);
    if (GeoConvHelper::usingGeoProjection()) {
        text = "lat:" + toString(pos.y(), GEO_OUTPUT_ACCURACY) + ", lon:" + toString(pos.x(), GEO_OUTPUT_ACCURACY);
    } else {
        text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    }
    myApp->getGeoLabel().setText(text.c_str());
}


Boundary
GUISUMOAbstractView::getVisibleBoundary() const {
    const SUMOReal xmin = myShownNetworkCenterX - myShownNetworkHalfWidth;
    const SUMOReal ymin = myShownNetworkCenterY - myShownNetworkHalfHeight;
    const SUMOReal xmax = myShownNetworkCenterX + myShownNetworkHalfWidth;
    const SUMOReal ymax = myShownNetworkCenterY + myShownNetworkHalfHeight;
    return Boundary(xmin, ymin, xmax, ymax);
}

void
GUISUMOAbstractView::paintGL() {
    myWidthInPixels = getWidth();
    myHeightInPixels = getHeight();
    if (myWidthInPixels==0||myHeightInPixels==0) {
        return;
    }

    if (getTrackedID()>0) {
        GUIGlObject *o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(getTrackedID());
        if (o!=0 && dynamic_cast<GUIGlObject*>(o)!=0) {
            Boundary b = o->getCenteringBoundary();
            myChanger->centerTo(*myGrid, b, false);
        }
        GUIGlObjectStorage::gIDStorage.unblockObject(getTrackedID());
    }

    unsigned int id = 0;
    if (myUseToolTips) {
        id = getObjectUnderCursor();
    }

    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red(),
        myVisualizationSettings->backgroundColor.green(),
        myVisualizationSettings->backgroundColor.blue(),
        1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    if (myVisualizationSettings->antialiase) {
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
    } else {
        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }

    applyChanges(1.0, 0, 0);
    doPaintGL(GL_RENDER, 1.0);
    if (myVisualizationSettings->showSizeLegend) {
        displayLegend();
    }
    // check whether the select mode /tooltips)
    //  shall be computed, too
    swapBuffers();
    if (myUseToolTips&&id!=0) {
        showToolTipFor(id);
    }
}


unsigned int
GUISUMOAbstractView::getObjectUnderCursor() {
    int xpos = myWindowCursorPositionX+myMouseHotspotX;
    int ypos = myWindowCursorPositionY+myMouseHotspotY;
    if (xpos<0||xpos>=myWidthInPixels) {
        return 0;
    }
    if (ypos<0||ypos>=myHeightInPixels) {
        return 0;
    }


    const int SENSITIVITY = 4;
    const int NB_HITS_MAX = 1000;
    // Prepare the selection mode
    static GLuint hits[NB_HITS_MAX];
    static GLint nb_hits = 0;
    glSelectBuffer(NB_HITS_MAX, hits);
    glInitNames();
    // compute new scale
    SUMOReal scale = SUMOReal(getWidth())/SUMOReal(SENSITIVITY);
    applyChanges(scale, myWindowCursorPositionX+myMouseHotspotX, myWindowCursorPositionY+myMouseHotspotY);
    // paint in select mode
    bool tmp = myUseToolTips;
    myUseToolTips = true;
    int hits2 = doPaintGL(GL_SELECT, scale);
    myUseToolTips = tmp;
    // Get the results
    nb_hits = glRenderMode(GL_RENDER);
    if (nb_hits==0||hits2==0) {
        applyChanges(1, 0, 0);
        return 0;
    }
    // Interpret results
    unsigned int idMax = 0;
    int prevLayer = -1000;
    for (int i=0; i<nb_hits; ++i) {
        assert(i*4+3<NB_HITS_MAX);
        unsigned int id = hits[i*4+3];
        GUIGlObject *o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        if (o==0) {
            continue;
        }
        if (o->getGlID()==0) {
            continue;
        }
        GUIGlObjectType type = o->getType();
        if (type!=0) {
            int clayer = (int) type;
            // determine an "abstract" layer for shapes
            //  this "layer" resembles the layer of the shape
            //  taking into account the stac of other objects
            if (type==GLO_SHAPE) {
                if (dynamic_cast<GUIPolygon2D*>(o)!=0) {
                    if (dynamic_cast<GUIPolygon2D*>(o)->getLayer()>0) {
                        clayer = GLO_MAX + dynamic_cast<GUIPolygon2D*>(o)->getLayer();
                    }
                    if (dynamic_cast<GUIPolygon2D*>(o)->getLayer()<0) {
                        clayer = dynamic_cast<GUIPolygon2D*>(o)->getLayer();
                    }
                }
                if (dynamic_cast<GUIPointOfInterest*>(o)!=0) {
                    if (dynamic_cast<GUIPointOfInterest*>(o)->getLayer()>0) {
                        clayer = GLO_MAX + dynamic_cast<GUIPointOfInterest*>(o)->getLayer();
                    }
                    if (dynamic_cast<GUIPointOfInterest*>(o)->getLayer()<0) {
                        clayer = dynamic_cast<GUIPointOfInterest*>(o)->getLayer();
                    }
                }
            }
            // check whether the current object is above a previous one
            if (prevLayer==-1000||prevLayer<clayer) {
                idMax = id;
                prevLayer = clayer;
            }
        }
        GUIGlObjectStorage::gIDStorage.unblockObject(id);
        assert(i*4+3<NB_HITS_MAX);
    }
    applyChanges(1, 0, 0);
    return idMax;
}


void
GUISUMOAbstractView::showToolTipFor(unsigned int id) {
    if (id!=0) {
        GUIGlObject *object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        int x, y;
        FXuint b;
        myApp->getCursorPosition(x, y, b);
        myToolTip->setObjectTip(object, x + myApp->getX(), y + myApp->getY());
        if (object!=0) {
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
        }
    } else {
        myToolTip->hide();
    }
}


void
GUISUMOAbstractView::paintGLGrid() {
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);

    SUMOReal xmin = myGrid->xmin();
    SUMOReal ymin = myGrid->ymin();
    SUMOReal ypos = ymin;
    SUMOReal xpos = xmin;
    SUMOReal xend = myGrid->xmax();
    SUMOReal yend = myGrid->ymax();

    glTranslated(0, 0, .55);
    glColor3d(0.5, 0.5, 0.5);
    // draw horizontal lines
    glBegin(GL_LINES);
    for (; ypos<yend;) {
        glVertex2d(xmin, ypos);
        glVertex2d(xend, ypos);
        ypos += myVisualizationSettings->gridYSize;
    }
    // draw vertical lines
    for (; xpos<xend;) {
        glVertex2d(xpos, ymin);
        glVertex2d(xpos, yend);
        xpos += myVisualizationSettings->gridXSize;
    }
    glEnd();
    glTranslated(0, 0, -.55);
}


void
GUISUMOAbstractView::applyChanges(SUMOReal scale, size_t xoff, size_t yoff) {
    myWidthInPixels = getWidth();
    myHeightInPixels = getHeight();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    myShownNetworkHalfWidth = myShownNetworkHalfHeight = 1;
    myAddScl = 1.;
    // rotate first;
    //  This is quite unchecked, so it's meaning and position is quite
    //  unclear
    glRotated(myChanger->getRotation(), 0, 0, 1);
    // Fit the view's size to the size of the net
    myShownNetworkHalfWidth /= (2.0/myNetScale);
    myShownNetworkHalfHeight /= (2.0/myNetScale);
    glScaled(2.0/myNetScale, 2.0/myNetScale, 1);
    //myAddScl *= (2.0/myNetScale);
    // apply ratio between window width and height
    glScaled(1./myRatio, 1, 1);
    myShownNetworkHalfWidth /= (1./myRatio);
    myShownNetworkHalfHeight /= 1.;
    SUMOReal width = myGrid->getWidth();
    SUMOReal height = myGrid->getHeight();
    // initially (zoom=100), the net shall be completely visible on the screen
    SUMOReal xs = (SUMOReal)(1. / (width / myNetScale) * myRatio);
    SUMOReal ys = (SUMOReal)(1. / (height / myNetScale));
    if (xs<ys) {
        glScaled(xs, xs, 1);
        myShownNetworkHalfWidth /= xs;
        myShownNetworkHalfHeight /= xs;
        myAddScl *= xs;
    } else {
        glScaled(ys, ys, 1);
        myShownNetworkHalfWidth /= ys;
        myShownNetworkHalfHeight /= ys;
        myAddScl *= ys;
    }
    // initially, leave some room for the net
    glScaled(0.97, 0.97, 1.);
    myShownNetworkHalfWidth /= 0.97;
    myShownNetworkHalfHeight /= 0.97;
    myAddScl *= (SUMOReal) .97;

    // Apply the zoom and the scale
    SUMOReal zoom = (SUMOReal)(myChanger->getZoom() / 100.0 * scale);
    glScaled(zoom, zoom, 1);
    myShownNetworkHalfWidth /= zoom;
    myShownNetworkHalfHeight /= zoom;
    //myAddScl *= zoom;
    // Translate to the middle of the net
    Position2D center = myGrid->getCenter();
    glTranslated(-center.x(), -center.y(), 0);
    myShownNetworkCenterX = center.x();
    myShownNetworkCenterY = center.y();
    // Translate in dependence to the view position applied by the user
    glTranslated(myChanger->getXPos(), myChanger->getYPos(), 0);
    myShownNetworkCenterX -= myChanger->getXPos();
    myShownNetworkCenterY -= myChanger->getYPos();
    // Translate to the mouse pointer, when wished
    if (xoff!=0||yoff!=0) {
        SUMOReal absX = (SUMOReal)((SUMOReal)xoff-(((SUMOReal) myWidthInPixels)/2.0));
        SUMOReal absY = (SUMOReal)((SUMOReal)yoff-(((SUMOReal) myHeightInPixels)/2.0));
        glTranslated(-p2m(absX), p2m(absY), 0);
        myShownNetworkCenterX += p2m(absX);
        myShownNetworkCenterY -= p2m(absY);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}


void
GUISUMOAbstractView::displayLegend() {
    // compute the scale bar length
    size_t length = 1;
    const std::string text("10000000000");
    size_t noDigits = 1;
    size_t pixelSize = 0;
    while (true) {
        pixelSize = (size_t) m2p((SUMOReal) length);
        if (pixelSize>20) {
            break;
        }
        length *= 10;
        noDigits++;
        if (noDigits>text.length()) {
            return;
        }
    }
    SUMOReal lineWidth = 1.0;
    glLineWidth((SUMOReal) lineWidth);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // draw the scale bar
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    SUMOReal len = (SUMOReal) pixelSize / (SUMOReal)(myWidthInPixels-1) * (SUMOReal) 2.0;
    glColor3d(0, 0, 0);
    double o = double(15) / double(myHeightInPixels);
    double o2 = o + o;
    double oo = double(5) / double(myHeightInPixels);
    glBegin(GL_LINES);
    // vertical
    glVertex2d(-.98, -1.+o);
    glVertex2d(-.98+len, -1.+o);
    // tick at begin
    glVertex2d(-.98, -1.+o);
    glVertex2d(-.98, -1.+o2);
    // tick at end
    glVertex2d(-.98+len, -1.+o);
    glVertex2d(-.98+len, -1.+o2);
    glEnd();

    SUMOReal w = SUMOReal(35) / SUMOReal(myWidthInPixels);
    SUMOReal h = SUMOReal(35) / SUMOReal(myHeightInPixels);
    pfSetPosition(SUMOReal(-0.99), SUMOReal(1.-o2-oo));
    pfSetScaleXY(w, h);
    glRotated(180, 1, 0, 0);
    pfDrawString("0m");
    glRotated(-180, 1, 0, 0);

    pfSetPosition(SUMOReal(-.99+len), SUMOReal(1.-o2-oo));
    glRotated(180, 1, 0, 0);
    pfDrawString((text.substr(0, noDigits) + "m").c_str());
    glRotated(-180, 1, 0, 0);

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


SUMOReal
GUISUMOAbstractView::m2p(SUMOReal meter) const {
    return (SUMOReal)(meter / myNetScale
                      *(myWidthInPixels/myRatio)
                      * myAddScl * myChanger->getZoom() / (SUMOReal) 100.0);
}


SUMOReal
GUISUMOAbstractView::p2m(SUMOReal pixel) const {
    return (SUMOReal) pixel * myNetScale /
           ((myWidthInPixels/myRatio) * myAddScl * myChanger->getZoom() / (SUMOReal) 100.0);
}


void
GUISUMOAbstractView::recenterView() {
    myChanger->recenterView();
}


void
GUISUMOAbstractView::centerTo(const GUIGlObject * const o) {
    centerTo(o->getCenteringBoundary());
    update();
}


void
GUISUMOAbstractView::centerTo(Boundary bound) {
    myChanger->centerTo(*myGrid, bound);
}

/*
bool
GUISUMOAbstractView::allowRotation() const
{
    return myParent->allowRotation();
}
*/

void
GUISUMOAbstractView::setWindowCursorPosition(FXint x, FXint y) {
    myWindowCursorPositionX = x;
    myWindowCursorPositionY = y;
}


FXbool
GUISUMOAbstractView::makeCurrent() {
    FXbool ret = FXGLCanvas::makeCurrent();
    return ret;
}


long
GUISUMOAbstractView::onConfigure(FXObject*,FXSelector,void*) {
    if (makeCurrent()) {
        myWidthInPixels = getWidth();
        myHeightInPixels = getHeight();
        myRatio = (SUMOReal) myWidthInPixels / (SUMOReal) myHeightInPixels;
        glViewport(0, 0, myWidthInPixels-1, myHeightInPixels-1);
        glClearColor(
            myVisualizationSettings->backgroundColor.red(),
            myVisualizationSettings->backgroundColor.green(),
            myVisualizationSettings->backgroundColor.blue(),
            1);
        doInit();
        myAmInitialised = true;
        makeNonCurrent();
        checkSnapshots();
    }
    return 1;
}


long
GUISUMOAbstractView::onPaint(FXObject*,FXSelector,void*) {
    if (!isEnabled()||!myAmInitialised) {
        return 1;
    }
    if (makeCurrent()) {
        paintGL();
        makeNonCurrent();
    }
    return 1;
}


long
GUISUMOAbstractView::onLeftBtnPress(FXObject *,FXSelector ,void *data) {
    delete myPopup;
    myPopup = 0;
    FXEvent *e = (FXEvent*) data;
    // check whether the selection-mode is activated
    if (e->state&CONTROLMASK) {
        // try to get the object-id if so
        if (makeCurrent()) {
            unsigned int id = getObjectUnderCursor();
            if (id!=0) {
                gSelected.toggleSelection(id);
            }
            makeNonCurrent();
            if (id!=0) {
                // possibly, the selection-colouring is used,
                //  so we should update the screen again...
                update();
            }
        }
    }
    myChanger->onLeftBtnPress(data);
    grab();
    return 1;
}


long
GUISUMOAbstractView::onLeftBtnRelease(FXObject *,FXSelector ,void *data) {
    delete myPopup;
    myPopup = 0;
    myChanger->onLeftBtnRelease(data);
    if (myApp->isGaming()) {
        onGamingClick(getPositionInformation());
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onRightBtnPress(FXObject *,FXSelector ,void *data) {
    delete myPopup;
    myPopup = 0;
    myChanger->onRightBtnPress(data);
    grab();
    return 1;
}


long
GUISUMOAbstractView::onRightBtnRelease(FXObject *,FXSelector ,void *data) {
    delete myPopup;
    myPopup = 0;
    if (!myChanger->onRightBtnRelease(data) && !myApp->isGaming()) {
        openObjectDialog();
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onMouseWheel(FXObject *,FXSelector ,void *data) {
    myChanger->onMouseWheel(data);
    return 1;
}


long
GUISUMOAbstractView::onMouseMove(FXObject *,FXSelector ,void *data) {
    SUMOReal xpos = myChanger->getXPos();
    SUMOReal ypos = myChanger->getYPos();
    SUMOReal zoom = myChanger->getZoom();
    if (myViewportChooser==0||!myViewportChooser->haveGrabbed()) {
        myChanger->onMouseMove(data);
    }
    if (myViewportChooser!=0 &&
            (xpos!=myChanger->getXPos()||ypos!=myChanger->getYPos()||zoom!=myChanger->getZoom())) {

        myViewportChooser->setValues(
            myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos());

    }
    updatePositionInformation();
    return 1;
}


long
GUISUMOAbstractView::onMouseLeft(FXObject *,FXSelector ,void* /*data*/) {
    if (myViewportChooser==0||!myViewportChooser->haveGrabbed()) {
//        myChanger->onMouseLeft();
        myToolTip->setObjectTip(0, -1, -1);
    }
    return 1;
}


void
GUISUMOAbstractView::openObjectDialog() {
    ungrab();
    if (!isEnabled()||!myAmInitialised) {
        return;
    }
    if (makeCurrent()) {
        // initialise the select mode
        unsigned int id = getObjectUnderCursor();
        GUIGlObject *o = 0;
        if (id!=0) {
            o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        } else {
            o = GUIGlObjectStorage::gIDStorage.getNetObject();
        }
        if (o!=0) {
            myPopup = o->getPopUpMenu(*myApp, *this);
            int x, y;
            FXuint b;
            myApp->getCursorPosition(x, y, b);
            myPopup->setX(x + myApp->getX());
            myPopup->setY(y + myApp->getY());
            myPopup->create();
            myPopup->show();
            myChanger->onRightBtnRelease(0);
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
        }
        makeNonCurrent();
    }
}


long
GUISUMOAbstractView::onKeyPress(FXObject *o,FXSelector sel,void *data) {
    FXEvent *e = (FXEvent*) data;
    if ((e->state&ALTMASK)!=0) {
        setDefaultCursor(getApp()->getDefaultCursor(DEF_CROSSHAIR_CURSOR));
        grabKeyboard();
    }
    /*
    switch(e->code) {
    case KEY_Left:
        myChanger->move((SUMOReal) -p2m((SUMOReal) getWidth()/10), 0);
        break;
    case KEY_Right:
        myChanger->move((SUMOReal) p2m((SUMOReal) getWidth()/10), 0);
        break;
    case KEY_Up:
        myChanger->move(0, (SUMOReal) -p2m((SUMOReal) getHeight()/10));
        break;
    case KEY_Down:
        myChanger->move(0, (SUMOReal) p2m((SUMOReal) getHeight()/10));
        break;
    default:
        break;
    }
    */
    return FXGLCanvas::onKeyPress(o, sel, data);
}


long
GUISUMOAbstractView::onKeyRelease(FXObject *o,FXSelector sel,void *data) {
    FXEvent *e = (FXEvent*) data;
    if ((e->state&ALTMASK)==0) {
        ungrabKeyboard();
        setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    }
    return FXGLCanvas::onKeyRelease(o, sel, data);
}


// ------------ Dealing with snapshots
void
GUISUMOAbstractView::setSnapshots(std::map<SUMOTime, std::string> snaps) {
    mySnapshots.insert(snaps.begin(), snaps.end());
}


std::string
GUISUMOAbstractView::makeSnapshot(const std::string &destFile) {
    std::string errorMessage;
    FXString ext=FXPath::extension(destFile.c_str());
    bool useGL2PS = ext=="ps"||ext=="eps"||ext=="pdf"||ext=="svg"||ext=="tex"||ext=="pgf";

    for (int i=0; i<10&&!makeCurrent(); ++i) {
        FXSingleEventThread::sleep(100);
    }
    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red(),
        myVisualizationSettings->backgroundColor.green(),
        myVisualizationSettings->backgroundColor.blue(),
        1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    if (myVisualizationSettings->antialiase) {
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
    } else {
        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }

    applyChanges(1.0, 0, 0);

    if (useGL2PS) {
        GLint format = GL2PS_PS;
        if (ext=="ps") {
            format = GL2PS_PS;
        } else if (ext=="eps") {
            format = GL2PS_EPS;
        } else if (ext=="pdf") {
            format = GL2PS_PDF;
        } else if (ext=="tex") {
            format = GL2PS_TEX;
        } else if (ext=="svg") {
            format = GL2PS_SVG;
        } else if (ext=="pgf") {
            format = GL2PS_PGF;
        } else {
            return "Could not save '" + destFile + "'.\n Unrecognized format '" + std::string(ext.text()) + "'.";
        }
        FILE *fp = fopen(destFile.c_str(), "wb");
        if (fp==0) {
            return "Could not save '" + destFile + "'.\n Could not open file for writing";
        }
        GLint buffsize = 0, state = GL2PS_OVERFLOW;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        while (state == GL2PS_OVERFLOW) {
            buffsize += 1024*1024;
            gl2psBeginPage(destFile.c_str(), "sumo-gui; http://sumo.sf.net", viewport, format, GL2PS_SIMPLE_SORT,
                           GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
                           GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "out.eps");
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            GLdouble sxmin = myShownNetworkCenterX - myShownNetworkHalfWidth;
            GLdouble sxmax = myShownNetworkCenterX + myShownNetworkHalfWidth;
            GLdouble symin = myShownNetworkCenterY - myShownNetworkHalfHeight;
            GLdouble symax = myShownNetworkCenterY + myShownNetworkHalfHeight;
            // compute lane width
            SUMOReal lw = m2p(3.0) * 1.;
            // draw decals (if not in grabbing mode)
            if (!myUseToolTips) {
                drawDecals();
                if (myVisualizationSettings->showGrid) {
                    paintGLGrid();
                }
            }
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            float minB[2];
            float maxB[2];
            minB[0] = sxmin;
            minB[1] = symin;
            maxB[0] = sxmax;
            maxB[1] = symax;
            myVisualizationSettings->needsGlID = myUseToolTips;
            myVisualizationSettings->scale = lw;
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            myGrid->Search(minB, maxB, *myVisualizationSettings);

            if (myVisualizationSettings->showSizeLegend) {
                displayLegend();
            }
            state = gl2psEndPage();
            glFinish();
        }
        fclose(fp);
    } else {
        doPaintGL(GL_RENDER, 1.0);
        if (myVisualizationSettings->showSizeLegend) {
            displayLegend();
        }
        swapBuffers();
        glFinish();
        FXColor *buf;
        FXMALLOC(&buf, FXColor, getWidth()*getHeight());
        // read from the back buffer
        glReadBuffer(GL_BACK);
        // Read the pixels
        glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buf);
        makeNonCurrent();
        update();
        // mirror
        size_t mwidth = getWidth();
        size_t mheight = getHeight();
        FXColor *paa = buf;
        FXColor *pbb = buf + mwidth * (mheight-1);
        do {
            FXColor *pa=paa;
            paa+=mwidth;
            FXColor *pb=pbb;
            pbb-=mwidth;
            do {
                FXColor t=*pa;
                *pa++=*pb;
                *pb++=t;
            } while (pa<paa);
        } while (paa<pbb);
        try {
            if (!MFXImageHelper::saveImage(destFile, getWidth(), getHeight(), buf)) {
                errorMessage = "Could not save '" + destFile + "'.";
            }
        } catch (InvalidArgument &e) {
            errorMessage = "Could not save '" + destFile + "'.\n" + e.what();
        }
        FXFREE(&buf);
    }
    return errorMessage;
}


void
GUISUMOAbstractView::checkSnapshots() {
    SUMOTime current = 0;
    try {
        current = MSNet::getInstance()->getCurrentTimeStep();
    } catch (ProcessError) {}
    std::map<SUMOTime, std::string>::iterator snapIt = mySnapshots.find(current);
    if (snapIt != mySnapshots.end()) {
        std::string error = makeSnapshot(snapIt->second);
        if (error!="") {
            MsgHandler::getWarningInstance()->inform(error);
        }
    }
}


void
GUISUMOAbstractView::showViewportEditor() {
    if (myViewportChooser==0) {
        myViewportChooser =
            new GUIDialog_EditViewport(this, "Edit Viewport...",
                                       myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos(),
                                       0, 0);
        myViewportChooser->create();
    }
    myViewportChooser->setOldValues(
        myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos());
    myViewportChooser->show();
}


void
GUISUMOAbstractView::setViewport(SUMOReal zoom, SUMOReal xPos, SUMOReal yPos) {
    myChanger->setViewport(zoom, xPos, yPos);
    update();
}


void
GUISUMOAbstractView::showToolTips(bool val) {
    myUseToolTips = val;
}



SUMOReal
GUISUMOAbstractView::getGridWidth() const {
    return myGrid->getWidth();
}


SUMOReal
GUISUMOAbstractView::getGridHeight() const {
    return myGrid->getHeight();
}


FXComboBox &
GUISUMOAbstractView::getColoringSchemesCombo() {
    return myParent->getColoringSchemesCombo();
}


void
GUISUMOAbstractView::drawDecals() {
    glTranslated(0, 0, .99);
    myDecalsLock.lock();
    for (std::vector<GUISUMOAbstractView::Decal>::iterator l=myDecals.begin(); l!=myDecals.end();) {
        GUISUMOAbstractView::Decal &d = *l;
        if (!d.initialised) {
            try {
                FXImage *i = MFXImageHelper::loadImage(getApp(), d.filename);
                if (MFXImageHelper::scalePower2(i)) {
                    MsgHandler::getWarningInstance()->inform("Scaling '" + d.filename + "'.");
                }
                d.glID = GUITexturesHelper::add(i);
                d.initialised = true;
            } catch (InvalidArgument &e) {
                MsgHandler::getErrorInstance()->inform("Could not load '" + d.filename + "'.\n" + e.what());
                l = myDecals.erase(l);
                continue;
            }
        }
        glPushMatrix();
        glTranslated(d.centerX, d.centerY, 0);
        glRotated(d.rot, 0, 0, 1);
        glColor3d(1,1,1);
        SUMOReal halfWidth((d.width / 2.));
        SUMOReal halfHeight((d.height / 2.));
        GUITexturesHelper::drawTexturedBox(d.glID, -halfWidth, -halfHeight, halfWidth, halfHeight);
        glPopMatrix();
        ++l;
    }
    myDecalsLock.unlock();
    glTranslated(0, 0, -.99);
}


// ------------ Additional visualisations
bool
GUISUMOAbstractView::addAdditionalGLVisualisation(GUIGlObject * const which) {
    if (myAdditionallyDrawn.find(which)==myAdditionallyDrawn.end()) {
        myAdditionallyDrawn[which] = 1;
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] + 1;
    }
    update();
    return true;
}


bool
GUISUMOAbstractView::removeAdditionalGLVisualisation(GUIGlObject * const which) {
    if (getTrackedID()==static_cast<int>(which->getGlID())) {
        stopTrack();
    }
    if (myAdditionallyDrawn.find(which)==myAdditionallyDrawn.end()) {
        return false;
    }
    int cnt = myAdditionallyDrawn[which];
    if (cnt==1) {
        myAdditionallyDrawn.erase(which);
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] - 1;
    }
    update();
    return true;
}



/****************************************************************************/

