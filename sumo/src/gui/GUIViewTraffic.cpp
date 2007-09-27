/****************************************************************************/
/// @file    GUIViewTraffic.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A view on the simulation; this view is a microscopic one
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

#include <iostream>
#include <utility>
#include <cmath>
#include <guisim/GUINet.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUILane.h>
#include <guisim/GUIVehicle.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSCORN.h>
#include <utils/gfx/RGBColor.h>
#include <utils/geom/Position2DVector.h>
#include <utils/shapes/Polygon2D.h>
#include "GUISUMOViewParent.h"
#include "GUIViewTraffic.h"
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <gui/GUIApplicationWindow.h>
#include <foreign/polyfonts/polyfonts.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/gui/drawer/GUICompleteSchemeStorage.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>

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
// static member definitions
// ===========================================================================
GUIColoringSchemesMap<GUIVehicle> GUIViewTraffic::myVehicleColoringSchemes;
GUIColoringSchemesMap<GUILaneWrapper> GUIViewTraffic::myLaneColoringSchemes;


// ===========================================================================
// member method definitions
// ===========================================================================
GUIViewTraffic::GUIViewTraffic(FXComposite *p,
                               GUIMainWindow &app,
                               GUISUMOViewParent *parent,
                               GUINet &net, FXGLVisual *glVis)
        : GUISUMOAbstractView(p, app, parent, net.myGrid, glVis),
        myVehicleDrawer(net.myEdgeWrapper), myLaneDrawer(net.myEdgeWrapper),
        myJunctionDrawer(net.myJunctionWrapper),
        myDetectorDrawer(GUIGlObject_AbstractAdd::getObjectList()),
        myROWDrawer(net.myEdgeWrapper),
        myTrackedID(-1),
        myEdges2Show(0), myJunctions2Show(0), myAdditional2Show(0), myPointToMove(0),myIdToMove(0),
        myLeftButtonPressed(false), myNet(&net)
{
    init(net);
}


GUIViewTraffic::GUIViewTraffic(FXComposite *p,
                               GUIMainWindow &app,
                               GUISUMOViewParent *parent,
                               GUINet &net, FXGLVisual *glVis,
                               FXGLCanvas *share)
        : GUISUMOAbstractView(p, app, parent, net.myGrid, glVis, share),
        myVehicleDrawer(net.myEdgeWrapper), myLaneDrawer(net.myEdgeWrapper),
        myJunctionDrawer(net.myJunctionWrapper),
        myDetectorDrawer(GUIGlObject_AbstractAdd::getObjectList()),
        myROWDrawer(net.myEdgeWrapper),
        myTrackedID(-1),
        myEdges2Show(0), myJunctions2Show(0), myAdditional2Show(0), myPointToMove(0),
        myNet(&net)
{
    init(net);
}


void
GUIViewTraffic::init(GUINet &)
{
    // build the artifact-instances-to-draw - tables
    myEdges2ShowSize = (MSEdge::dictSize()>>5) + 1;
    myEdges2Show = new size_t[myEdges2ShowSize];
    clearUsetable(myEdges2Show, myEdges2ShowSize);
    myJunctions2ShowSize = (MSJunction::dictSize()>>5) + 1;
    myJunctions2Show = new size_t[myJunctions2ShowSize];
    clearUsetable(myJunctions2Show, myJunctions2ShowSize);
    myAdditional2ShowSize = (GUIGlObject_AbstractAdd::getObjectList().size()>>5) + 1;
    myAdditional2Show = new size_t[myAdditional2ShowSize];
    clearUsetable(myAdditional2Show, myAdditional2ShowSize);
    // initialise default scheme
    myVisualizationSettings = &gSchemeStorage.get(gSchemeStorage.getNames()[0]);
}


GUIViewTraffic::~GUIViewTraffic()
{
    delete[] myEdges2Show;
    delete[] myJunctions2Show;
    delete[] myAdditional2Show;
}


void
GUIViewTraffic::create()
{
    FXGLCanvas::create();
}


void
GUIViewTraffic::buildViewToolBars(GUIGlChildWindow &v)
{
    // build coloring tools
    {
        const std::vector<std::string> &names = gSchemeStorage.getNames();
        for (std::vector<std::string>::const_iterator i=names.begin(); i!=names.end(); ++i) {
            v.getColoringSchemesCombo().appendItem((*i).c_str());
        }
        v.getColoringSchemesCombo().setNumVisible(5);
    }
    // for junctions
    new FXButton(v.getLocatorPopup(),
                 "\tLocate Junction\tLocate a junction within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATEJUNCTION), &v, MID_LOCATEJUNCTION,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
    // for edges
    new FXButton(v.getLocatorPopup(),
                 "\tLocate Street\tLocate a street within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATEEDGE), &v, MID_LOCATEEDGE,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
    // for vehicles
    new FXButton(v.getLocatorPopup(),
                 "\tLocate Vehicle\tLocate a vehicle within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATEVEHICLE), &v, MID_LOCATEVEHICLE,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
    // for tls
    new FXButton(v.getLocatorPopup(),
                 "\tLocate TLS\tLocate a tls within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATETLS), &v, MID_LOCATETLS,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
    // for additional stuff
    new FXButton(v.getLocatorPopup(),
                 "\tLocate Additional\tLocate an additional structure within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATEADD), &v, MID_LOCATEADD,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
    // for shapes
    new FXButton(v.getLocatorPopup(),
                 "\tLocate Shape\tLocate a shape within the network.",
                 GUIIconSubSys::getIcon(ICON_LOCATESHAPE), &v, MID_LOCATESHAPE,
                 ICON_ABOVE_TEXT|FRAME_THICK|FRAME_RAISED);
}


void
GUIViewTraffic::setColorScheme(char* data)
{
    string name = (char*) data;
    if (myVisualizationChanger!=0) {
        if (myVisualizationChanger->getCurrentScheme()!=name) {
            myVisualizationChanger->setCurrentScheme(name);
        }
    }
    myVisualizationSettings = &gSchemeStorage.get(data);
    // lanes
    switch (myLaneColoringSchemes.getColorSetType(myVisualizationSettings->laneEdgeMode)) {
    case CST_SINGLE:
        myLaneColoringSchemes.getColorerInterface(myVisualizationSettings->laneEdgeMode)->resetColor(
            myVisualizationSettings->laneColorings[myVisualizationSettings->laneEdgeMode][0]);
        break;
    case CST_MINMAX:
        myLaneColoringSchemes.getColorerInterface(myVisualizationSettings->laneEdgeMode)->resetColor(
            myVisualizationSettings->laneColorings[myVisualizationSettings->laneEdgeMode][0],
            myVisualizationSettings->laneColorings[myVisualizationSettings->laneEdgeMode][1]);
        break;
    default:
        break;
    }
    // vehicles
    switch (myVehicleColoringSchemes.getColorSetType(myVisualizationSettings->vehicleMode)) {
    case CST_SINGLE:
        myVehicleColoringSchemes.getColorerInterface(myVisualizationSettings->vehicleMode)->resetColor(
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][0]);
        break;
    case CST_MINMAX:
        myVehicleColoringSchemes.getColorerInterface(myVisualizationSettings->vehicleMode)->resetColor(
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][0],
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][1]);
        break;
    case CST_MINMAX_OPT:
        myVehicleColoringSchemes.getColorerInterface(myVisualizationSettings->vehicleMode)->resetColor(
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][0],
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][1],
            myVisualizationSettings->vehicleColorings[myVisualizationSettings->vehicleMode][2]);
        break;
    default:
        break;
    }
    update();
}


void
GUIViewTraffic::doPaintGL(int mode, SUMOReal scale)
{
    // init view settings
    glRenderMode(mode);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    // get the viewport settings
    const Boundary &nb = myNet->getBoundary();
    {
        SUMOReal width = nb.getWidth();
        SUMOReal height = nb.getHeight();
        SUMOReal mzoom = myChanger->getZoom();
        SUMOReal cy = myChanger->getYPos();//cursorY;
        SUMOReal cx = myChanger->getXPos();//cursorY;

        // compute the visible area in horizontal direction
        SUMOReal mratioX = 1;
        SUMOReal mratioY = 1;
        SUMOReal xs = ((SUMOReal) myWidthInPixels / (SUMOReal) myApp->getMaxGLWidth())
                      / (myGrid->getBoundary().getWidth() / myNetScale) * myRatio;
        SUMOReal ys = ((SUMOReal) myHeightInPixels / (SUMOReal) myApp->getMaxGLHeight())
                      / (myGrid->getBoundary().getHeight() / myNetScale);
        if (xs<ys) {
            mratioX = 1;
            mratioY = ys/xs;
        } else {
            mratioY = 1;
            mratioX = xs/ys;
        }
        SUMOReal sxmin = nb.getCenter().x()
                         - mratioX * width * (SUMOReal) 100 / (mzoom) / (SUMOReal) 2. / (SUMOReal) .97;
        sxmin -= cx;
        SUMOReal sxmax = nb.getCenter().x()
                         + mratioX * width * (SUMOReal) 100 / (mzoom) / (SUMOReal) 2. / (SUMOReal) .97;
        sxmax -= cx;

        // compute the visible area in vertical direction
        SUMOReal symin = nb.getCenter().y()
                         - mratioY * height / mzoom * (SUMOReal) 100 / (SUMOReal) 2. / (SUMOReal) .97;
        symin -= cy;
        SUMOReal symax = nb.getCenter().y()
                         + mratioY * height / mzoom * (SUMOReal) 100 / (SUMOReal) 2. / (SUMOReal) .97;
        symax -= cy;

        // reset the tables of things to show if the viewport has changed
        if (myViewportSettings.differ(sxmin, symin, sxmax, symax)) {
            clearUsetable(myEdges2Show,myEdges2ShowSize);
            clearUsetable(myJunctions2Show, myJunctions2ShowSize);
            myNet->myGrid.get(GLO_LANE|GLO_JUNCTION|GLO_DETECTOR,
                            sxmin, symin, sxmax, symax,
                            myEdges2Show, myJunctions2Show,myAdditional2Show);
            myViewportSettings.set(sxmin, symin, sxmax, symax);
        }
    }

    // compute lane width
    SUMOReal width = m2p(3.0) * scale;
    // compute which drawer shall be used
    if (myUseToolTips) {
        myVehicleDrawer.setGLID(true);
        myLaneDrawer.setGLID(true);
        myDetectorDrawer.setGLID(true);
        myROWDrawer.setGLID(true);
        myJunctionDrawer.setGLID(true);
    } else {
        myVehicleDrawer.setGLID(false);
        myLaneDrawer.setGLID(false);
        myDetectorDrawer.setGLID(true);
        myROWDrawer.setGLID(true);
        myJunctionDrawer.setGLID(true);
    }
    // draw
    {
        myDecalsLock.lock();
        for (std::vector<GUISUMOAbstractView::Decal>::iterator l=myDecals.begin(); l!=myDecals.end(); ++l) {
            GUISUMOAbstractView::Decal &d = *l;
            if (!d.initialised) {
                FXImage *i = MFXImageHelper::loadimage(getApp(), d.filename);
                if (i!=0) {
                    d.glID = GUITexturesHelper::add(i);
                    d.initialised = true;
                }
            }
            glPushMatrix();
            glTranslated(d.left, d.top, 0);
            glRotated(d.rot, 0, 0, 1);
            glColor3d(1,1,1);
            GUITexturesHelper::drawTexturedBox(d.glID, 0, 0, d.right-d.left, d.bottom-d.top);
            glPopMatrix();
        }
        myDecalsLock.unlock();
    }
    drawShapes(myNet->getShapeContainer(), 0, width);

    myJunctionDrawer.drawGLJunctions(myJunctions2Show, myJunctions2ShowSize,
                                     myJunctionColScheme, *myVisualizationSettings);
    myLaneDrawer.drawGLLanes(myEdges2Show, myEdges2ShowSize, width,
                             *myLaneColoringSchemes.getColorer(myVisualizationSettings->laneEdgeMode),
                             *myVisualizationSettings);
    myDetectorDrawer.drawGLDetectors(myAdditional2Show, myAdditional2ShowSize,
                                     width, *myVisualizationSettings);
    myROWDrawer.drawGLROWs(*myNet, myEdges2Show, myEdges2ShowSize, width,
                           *myVisualizationSettings);
    if (myVisualizationSettings->drawEdgeName) {
        myLaneDrawer.drawGLLaneNames(myEdges2Show, myEdges2ShowSize, width,
                                     *myVisualizationSettings);
    }
    if (myVisualizationSettings->drawJunctionName) {
        myJunctionDrawer.drawGLJunctionNames(myJunctions2Show, myJunctions2ShowSize,
                                             width, myJunctionColScheme, *myVisualizationSettings);
    }
    //
    for (std::vector<VehicleOps>::iterator i=myVehicleOps.begin(); i!=myVehicleOps.end(); ++i) {
        const VehicleOps &vo = *i;
        switch (vo.type) {
        case VO_SHOW_ROUTE: {
            if (vo.routeNo>=0) {
                drawRoute(vo, vo.routeNo, 0.25);
            } else {
                if (vo.vehicle->hasCORNIntValue(MSCORN::CORN_VEH_NUMBERROUTE)) {
                    int noReroutePlus1 = vo.vehicle->getCORNIntValue(MSCORN::CORN_VEH_NUMBERROUTE) + 1;
                    for (int i=noReroutePlus1-1; i>=0; i--) {
                        SUMOReal darken =
                            (SUMOReal) 0.4
                            / (SUMOReal) noReroutePlus1
                            * (SUMOReal) i;
                        drawRoute(vo, i, darken);
                    }
                } else {
                    drawRoute(vo, 0, 0.25);
                }
            }
        }
        break;
        default:
            break;
        }
    }
    // draw the Polygons
    drawShapes(myNet->getShapeContainer(), 10, width);
    // draw vehicles only when they're visible
    if (scale*m2p(3)>myVisualizationSettings->minVehicleSize) {
        myVehicleDrawer.drawGLVehicles(myEdges2Show, myEdges2ShowSize, width,
                                       myVehicleColoringSchemes, *myVisualizationSettings);
    }
    glPopMatrix();
}


RGBColor
GUIViewTraffic::getEdgeColor(GUIEdge *edge) const
{
    switch (edge->getPurpose()) {
    case GUIEdge::EDGEFUNCTION_NORMAL:
        return RGBColor((SUMOReal) 0, (SUMOReal) 0, (SUMOReal) 0);
    case GUIEdge::EDGEFUNCTION_SOURCE:
        return RGBColor((SUMOReal) 0, (SUMOReal) 0.2, (SUMOReal) 0);
    case GUIEdge::EDGEFUNCTION_SINK:
        return RGBColor((SUMOReal) 0.2, (SUMOReal) 0, (SUMOReal) 0);
    case GUIEdge::EDGEFUNCTION_INTERNAL:
        return RGBColor((SUMOReal) 0, (SUMOReal) 0, (SUMOReal) 0.2);
    default:
        break;
    }
    // hmmmm - should not happen
    return RGBColor((SUMOReal) 1, (SUMOReal) 1, (SUMOReal) 0);
}


void
GUIViewTraffic::startTrack(int id)
{
    myTrackedID = id;
}


void
GUIViewTraffic::stopTrack()
{
    myTrackedID = -1;
}


int
GUIViewTraffic::getTrackedID() const
{
    return myTrackedID;
}


void
GUIViewTraffic::doInit()
{}


void
GUIViewTraffic::drawRoute(const VehicleOps &vo, int routeNo, SUMOReal darken)
{
    if (myUseToolTips) {
        glPushName(vo.vehicle->getGlID());
    }
    myVehicleColoringSchemes.getColorer(myVisualizationSettings->vehicleMode)->setGlColor(*(vo.vehicle));
    GLdouble colors[4];
    glGetDoublev(GL_CURRENT_COLOR, colors);
    colors[0] -= darken;
    if (colors[0]<0) colors[0] = 0;
    colors[1] -= darken;
    if (colors[1]<0) colors[1] = 0;
    colors[2] -= darken;
    if (colors[2]<0) colors[2] = 0;
    colors[3] -= darken;
    if (colors[3]<0) colors[3] = 0;
    glColor3dv(colors);
    draw(vo.vehicle->getRoute(routeNo));
    if (myUseToolTips) {
        glPopName();
    }
}


void
GUIViewTraffic::draw(const MSRoute &r)
{
    MSRouteIterator i = r.begin();
    for (; i!=r.end(); ++i) {
        const MSEdge *e = *i;
        const GUIEdge *ge = static_cast<const GUIEdge*>(e);
        const GUILaneWrapper &lane = ge->getLaneGeometry((size_t) 0);
        GLHelper::drawBoxLines(lane.getShape(), lane.getShapeRotations(), lane.getShapeLengths(), 1.0);
    }
}


void
GUIViewTraffic::showRoute(GUIVehicle *v, int index)
{
    VehicleOps vo;
    vo.vehicle = v;
    vo.type = VO_SHOW_ROUTE;
    vo.routeNo = index;
    myVehicleOps.push_back(vo);
    update();
}


void
GUIViewTraffic::hideRoute(GUIVehicle *v, int index)
{
    std::vector<VehicleOps>::iterator i =
        find_if(myVehicleOps.begin(), myVehicleOps.end(), vehicle_in_ops_finder(v));
    while (i!=myVehicleOps.end()) {
        if ((*i).type==VO_SHOW_ROUTE&&(*i).routeNo==index) {
            i = myVehicleOps.erase(i);
            update();
            return;
        }
        i = find_if(i+1, myVehicleOps.end(), vehicle_in_ops_finder(v));
    }
    update();
}


bool
GUIViewTraffic::amShowingRouteFor(GUIVehicle *v, int index)
{
    std::vector<VehicleOps>::iterator i =
        find_if(myVehicleOps.begin(), myVehicleOps.end(), vehicle_in_ops_finder(v));
    while (i!=myVehicleOps.end()) {
        if ((*i).type==VO_SHOW_ROUTE&&(*i).routeNo==index) {
            return true;
        }
        i = find_if(i+1, myVehicleOps.end(), vehicle_in_ops_finder(v));
    }
    return false;
}


void
GUIViewTraffic::showViewschemeEditor()
{
    if (myVisualizationChanger==0) {
        myVisualizationChanger =
            new GUIDialog_ViewSettings(
                this, myVisualizationSettings,
                &myLaneColoringSchemes, &myVehicleColoringSchemes,
                &myDecals, &myDecalsLock);
        myVisualizationChanger->create();
    }
    myVisualizationChanger->show();
}


long
GUIViewTraffic::onLeftBtnPress(FXObject *o,FXSelector sel,void *data)
{
    FXEvent *e = (FXEvent*) data;
    myLeftButtonPressed=true;
    if (e->state&CAPSLOCKMASK) {
        if (makeCurrent()) {
            unsigned int id = getObjectUnderCursor();
            if (id!=0) {
                GUIGlObject *o = gIDStorage.getObjectBlocking(id);
                std::string n= o->getFullName();
                std::string name = n.substr(n.find(":")+1,n.length());
                GUIPointOfInterest *p= static_cast<GUIPointOfInterest *>(myNet->getShapeContainer().getPOICont(1).get(name));
                setFirstPoint(p);
            } else {
                FXMessageBox::error(myApp,MBOX_OK,"Error Get Point","No Point at the Position","");
            }
            makeNonCurrent();
        }
    }

    if ((e->state&SHIFTMASK)!=0) {
        // try to get the object-id if so
        if (makeCurrent()) {
            unsigned int id = getObjectUnderCursor();
            setIdToMove(id);
            makeNonCurrent();
            if (id!=0) {
                GUIGlObject *o = gIDStorage.getObjectBlocking(id);
                std::string n= o->getFullName();
                std::string name = n.substr(n.find(":")+1,n.length());
                GUIPointOfInterest *p= static_cast<GUIPointOfInterest *>
                                       (myNet->getShapeContainer().getPOICont(1).get(name));
                setPointToMove(p);
            } else {
                Position2D point = getPositionInformation();
                std::string Id= toString(point.x()) +  "," + toString(point.y());
                GUIPointOfInterest *p = new GUIPointOfInterest(gIDStorage, 1, Id, "point",
                                        Position2D(point.x(), point.y()),RGBColor(0,0,0));
                myNet->getShapeContainer().add(1, p);
                update();
            }
        }
        return 1;
    }
    return GUISUMOAbstractView::onLeftBtnPress(o, sel, data);
}


void
GUIViewTraffic::setPointToMove(PointOfInterest *p)
{
    myPointToMove = p;
}


void
GUIViewTraffic::setIdToMove(unsigned int id)
{
    myIdToMove = id;
}



long
GUIViewTraffic::onLeftBtnRelease(FXObject *o,FXSelector sel,void *data)
{
    long ret = GUISUMOAbstractView::onLeftBtnRelease(o, sel, data);
    FXEvent *e = (FXEvent*) data;
    if (myLeftButtonPressed && myFirstPoint!=0 && e->state&CAPSLOCKMASK) {
        if (makeCurrent()) {
            unsigned int id = getObjectUnderCursor();
            if (id==-1) {
                GUIGlObject *o = gIDStorage.getObjectBlocking(id);
                std::string n= o->getFullName();
                std::string name = n.substr(n.find(":")+1,n.length());
                GUIPointOfInterest *p= static_cast<GUIPointOfInterest *>(myNet->getShapeContainer().getPOICont(1).get(name));
                setSecondPoint(p);
//!!!				_net->getShapeContainer().addPair(myFirstPoint,_secondPoint);
                update();
            } else {
                Position2D point = getPositionInformation();
                std::string Id= toString(point.x())+toString(point.y());
                GUIPointOfInterest *p =
                    new GUIPointOfInterest(gIDStorage, 1, Id, "point", point, RGBColor(0,0,0));
                myNet->getShapeContainer().add(1, p);
                setSecondPoint(p);
//!!!				_net->getShapeContainer().addPair(myFirstPoint,_secondPoint);
                SUMOReal dist = (SUMOReal)(sqrt(pow((myFirstPoint->x() - mySecondPoint->x()),2) + pow((myFirstPoint->y() - mySecondPoint->y()),2)));
                update();
            }
            myFirstPoint=0;
            makeNonCurrent();
        }

    } else {
        delete myPopup;
        myPopup = 0;
        ungrab();
    }
    myLeftButtonPressed=false;
    return ret;
}


long
GUIViewTraffic::onMouseMove(FXObject *o,FXSelector sel,void *data)
{
    /*
    SUMOReal xpos = myChanger->getXPos();
    SUMOReal ypos = myChanger->getYPos();
    SUMOReal zoom = myChanger->getZoom();
    */
    FXEvent *e=(FXEvent*)data;

    if (myPointToMove!=0 && e->state&SHIFTMASK) {
        // Keep Color Informations
        Position2D point = getPositionInformation(e->win_x, e->win_y);
        myPointToMove->set(point.x(), point.y());
        if (!myLeftButtonPressed) {
            myPointToMove=0;
        }
        updatePositionInformation();
        update();
    } else {
        if (myFirstPoint!=0 && e->state&CAPSLOCKMASK && myLeftButtonPressed) {
            //do Nothing
            return 1;
        }
    }
    return GUISUMOAbstractView::onMouseMove(o, sel, data);
}

void
GUIViewTraffic::rename(GUIGlObject *o)
{
    std::string n= o->getFullName();
    std::string name = n.substr(n.find(":")+1,n.length());

    FXInputDialog* dialog = new FXInputDialog(myApp,"Please Enter the new Name", n.c_str());
    dialog->setText(name.c_str());
    if (dialog->execute()) {
        /*
        FXString input = dialog->getText();
        if(input.length() > 0){
        PointOfInterest* p =_net->getShapeContainer().getPOICont(1).get(name);
        RGBColor col(p->red(),p->green(),p->blue());
        GUIPointOfInterest *p_new = new GUIPointOfInterest(gIDStorage, input.text(), "point",
          Position2D(p->x(), p->y()),col);
        myNet->getShapeContainer().add(1, p_new);

        //!!!           const std::map<PointOfInterest*, PointOfInterest*> &pv = myNet->getShapeContainer().getPOIPairCont();
        std::map<PointOfInterest*, PointOfInterest*>::const_iterator pi = pv.begin();
        for(; pi!=pv.end(); pi++) {
        if((*pi).first->getID()==name){
                   (*pi).first->setID(toString(input.text()));
        }
        if((*pi).second->getID()==name){
                   (*pi).second->setID(toString(input.text()));
        }
        }

        bool b=_net->getShapeContainer().getPOICont().erase(p->getID());
        //gIDStorage.remove(id); ?? muss ich den Point daraus l�schen??

        }
        */
    }
    delete dialog;
}

void
GUIViewTraffic::moveTo(GUIGlObject *)
{
    /*
    std::string n= o->getFullName();
    std::string name = n.substr(n.find(":")+1,n.length());

    FXDialogBox* dialog = new FXDialogBox(myApp,"Move to new Position",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,300,100, 0,0,0,0, 4,4);

    // Bottom buttons
    FXHorizontalFrame* buttons=new FXHorizontalFrame(dialog,LAYOUT_SIDE_BOTTOM|FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,40,40,20,20);

    // Separator
    new FXHorizontalSeparator(dialog,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|SEPARATOR_GROOVE);
    FXHorizontalFrame* contents=new FXHorizontalFrame(dialog,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);

    new FXLabel(contents,"Enter XPos",NULL,LAYOUT_LEFT);
    FXTextField* textX =new FXTextField(contents,6,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,2,2);
    textX->setText((toString((myNet->getShapeContainer().getPOICont().get(name))->x())).c_str());

    new FXLabel(contents,"Enter YPos",NULL,LAYOUT_LEFT);
    FXTextField* textY =new FXTextField(contents,6,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,2,2);
    textY->setText((toString((myNet->getShapeContainer().getPOICont().get(name))->y())).c_str());

    new FXButton(buttons,"&Accept",NULL,dialog,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
    new FXButton(buttons,"&Cancel",NULL,dialog,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);

    if(dialog->execute()){
    FXString inputX = textX->getText();
    FXString inputY = textX->getText();
    if(inputX.length() > 0 && inputY.length() > 0){
    SUMOReal x = TplConvert<char>::_2SUMOReal(textX->getText().text());
    SUMOReal y = TplConvert<char>::_2SUMOReal(textY->getText().text());
     myNet->getShapeContainer().getPOICont().get(name)->set(x,y);
    }
    }
    delete dialog;
    */
}


void
GUIViewTraffic::changeCol(GUIGlObject *)
{
    /*
    std::string n= o->getFullName();
    std::string name = n.substr(n.find(":")+1,n.length());

    FXColorDialog * dialog = new FXColorDialog(myApp,"Color");

    RGBColor col = RGBColor(myNet->getShapeContainer().getPOICont().get(name)->myRed,
    			_net->getShapeContainer().getPOICont().get(name)->myGreen,
    			_net->getShapeContainer().getPOICont().get(name)->myBlue);
    dialog->setRGBA(FXRGB(col.red()*255., col.green()*255., col.blue()*255.));
    if(dialog->execute()){
    FXColor c = dialog->getRGBA();
    myNet->getShapeContainer().getPOICont().get(name)->myRed=(SUMOReal) FXREDVAL(c) / (SUMOReal) 255.;
    myNet->getShapeContainer().getPOICont().get(name)->myGreen=(SUMOReal) FXGREENVAL(c) / (SUMOReal) 255.;
    myNet->getShapeContainer().getPOICont().get(name)->myBlue=(SUMOReal) FXBLUEVAL(c) / (SUMOReal) 255.;
    }
    delete dialog;
    */
}

void
GUIViewTraffic::changeTyp(GUIGlObject *)
{
    /*
    std::string n= o->getFullName();
    std::string name = n.substr(n.find(":")+1,n.length());

    FXDialogBox* dialog = new FXDialogBox(myApp,"Change Type... ",DECOR_TITLE|DECOR_BORDER);

    // Bottom buttons
    FXHorizontalFrame* buttons=new FXHorizontalFrame(dialog,LAYOUT_SIDE_BOTTOM|FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,40,40,20,20);

    // Separator
    new FXHorizontalSeparator(dialog,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|SEPARATOR_GROOVE);

    FXHorizontalFrame* contents=new FXHorizontalFrame(dialog,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);


    FXListBox *lstb= new FXListBox (contents, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X);
    lstb->insertItem (0,"Point");
    lstb->insertItem (1,"Vehicle");
    lstb->insertItem (2,"Lane");
    lstb->insertItem (3,"POI");
    lstb->insertItem (4,"Polygon");
    lstb->insertItem (5,"Building");

    // Accept
    new FXButton(buttons,"&Accept",NULL,dialog,FXDialogBox::ID_ACCEPT,BUTTON_DEFAULT|BUTTON_INITIAL|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
    // Cancel
    new FXButton(buttons,"&Cancel",NULL,dialog,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);

    if(dialog->execute()){
    (myNet->getShapeContainer().getPOICont().get(name))->setType((lstb->getItemText(lstb->getCurrentItem())).text());
    }

    delete buttons;
    delete lstb;
    delete contents;
    delete dialog;
    */
}

void
GUIViewTraffic::deleteObj(GUIGlObject *)
{
    /*
    std::string n= o->getFullName();
    std::string name = n.substr(n.find(":")+1,n.length());

    FXuint answer=FXMessageBox::question(myApp,MBOX_YES_NO_CANCEL,"Delete the Point","Do you want to delete the Point?");
    if(answer==MBOX_CLICKED_YES){
    gIDStorage.remove(getObjectUnderCursor());
    bool b =_net->getShapeContainer().getPOICont().erase(name);
    if(!b){
     FXMessageBox::error(myApp,MBOX_OK,"Error by Delete ","Cannot delete the Point at the Position","");
    }
    }
    */
}

void
GUIViewTraffic::setFirstPoint(PointOfInterest *p)
{
    myFirstPoint=p;
}

void
GUIViewTraffic::setSecondPoint(PointOfInterest *p)
{
    mySecondPoint=p;
}


GUIColoringSchemesMap<GUIVehicle> &
GUIViewTraffic::getVehiclesSchemesMap()
{
    return myVehicleColoringSchemes;
}


GUIColoringSchemesMap<GUILaneWrapper> &
GUIViewTraffic::getLaneSchemesMap()
{
    return myLaneColoringSchemes;
}



/****************************************************************************/

