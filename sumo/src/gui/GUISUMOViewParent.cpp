/****************************************************************************/
/// @file    GUISUMOViewParent.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A single child window which contains a view of the simulation area
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

#include <utils/geom/Position2D.h>
#include <utils/geom/Boundary.h>
#include <guisim/GUIVehicle.h>
#include <microsim/MSJunction.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUINet.h>

#include <string>
#include <vector>
#include "GUIGlobals.h"
#include "dialogs/GUIDialog_GLObjChooser.h"
#include "GUIViewTraffic.h"
#include "GUIApplicationWindow.h"
#include "GUISUMOViewParent.h"
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIcons.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/foxtools/MFXUtils.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/foxtools/MFXImageHelper.h>

#ifdef HAVE_MESOSIM
#include <mesogui/GUIViewMesoEdges.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


const FXchar patterns[]=
    "\nGIF Image (*.gif)"
    "\nBMP Image (*.bmp)"
    "\nXPM Image (*.xpm)"
    "\nPCX Image (*.pcx)"
    "\nICO Image (*.ico)"
    "\nRGB Image  (*.rgb)"
    "\nXBM Image  (*.xbm)"
    "\nTARGA Image  (*.tga)"
#ifdef HAVE_PNG_H
    "\nPNG Image  (*.png)"
#endif
#ifdef HAVE_JPEG_H
    "\nJPEG Image (*.jpg)"
#endif
#ifdef HAVE_TIFF_H
    "\nTIFF Image (*.tif)"
#endif
    "All Image Files (*.gif, *.bmp, *.xpm, *.pcx, *.ico, *.rgb, *.xbm, *.tga, *.png, *.jpg, *.tif)"
    "All Files (*)"
    ;

// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUISUMOViewParent) GUISUMOViewParentMap[]= {
    FXMAPFUNC(SEL_COMMAND,  MID_MAKESNAPSHOT,   GUISUMOViewParent::onCmdMakeSnapshot),
    //        FXMAPFUNC(SEL_COMMAND,  MID_ALLOWROTATION,  GUISUMOViewParent::onCmdAllowRotation),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEJUNCTION, GUISUMOViewParent::onCmdLocateJunction),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEEDGE,     GUISUMOViewParent::onCmdLocateEdge),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEVEHICLE,  GUISUMOViewParent::onCmdLocateVehicle),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATETLS,      GUISUMOViewParent::onCmdLocateTLS),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATEADD,      GUISUMOViewParent::onCmdLocateAdd),
    FXMAPFUNC(SEL_COMMAND,  MID_LOCATESHAPE,    GUISUMOViewParent::onCmdLocateShape),
    FXMAPFUNC(SEL_COMMAND,  MID_SIMSTEP,        GUISUMOViewParent::onSimStep),

};

// Object implementation
FXIMPLEMENT(GUISUMOViewParent, GUIGlChildWindow, GUISUMOViewParentMap, ARRAYNUMBER(GUISUMOViewParentMap))


// ===========================================================================
// member method definitions
// ===========================================================================
GUISUMOViewParent::GUISUMOViewParent(FXMDIClient* p,
                                     FXGLCanvas * /*share!!!*/,FXMDIMenu *mdimenu,
                                     const FXString& name, GUINet &/*net!!!*/,
                                     GUIMainWindow *parentWindow,
                                     ViewType /*view!!!*/, FXIcon* ic, FXPopup* /*pup!!!*/,
                                     FXuint opts,
                                     FXint /*x!!!*/, FXint /*y!!!*/, FXint /*w!!!*/, FXint /*h!!!*/)
        : GUIGlChildWindow(p, mdimenu, name, ic, 0, opts, 10, 10, 300, 200),
        myParent(parentWindow), myChooser(0)

{
    myParent->addChild(this, false);
}


void
GUISUMOViewParent::init(ViewType view, FXGLCanvas *share, GUINet &net)
{
    /*
    // Make MDI Window Menu
    setTracking();
    FXVerticalFrame *glcanvasFrame =
        new FXVerticalFrame(this,
                            FRAME_SUNKEN|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,
                            0,0,0,0,0,0,0,0);
    // build the tool bar
    buildToolBar(glcanvasFrame);
    */
    switch (view) {
    default:
    case MICROSCOPIC_VIEW:
        if (share!=0) {
            myView =
                new GUIViewTraffic(myContentFrame, *myParent, this, net,
                                   myParent->getGLVisual(), share);
        } else {
            myView =
                new GUIViewTraffic(myContentFrame, *myParent, this, net,
                                   myParent->getGLVisual());
        }
        break;
#ifdef HAVE_MESOSIM
    case EDGE_MESO_VIEW:
        if (share!=0) {
            myView =
                new GUIViewMesoEdges(myContentFrame, *myParent, this,
                                     net, myParent->getGLVisual(), share);
        } else {
            myView =
                new GUIViewMesoEdges(myContentFrame, *myParent, this,
                                     net, myParent->getGLVisual());
        }
        break;
#endif
    }
    myView->buildViewToolBars(*this);
}


GUISUMOViewParent::~GUISUMOViewParent()
{
    myParent->removeChild(this);
//    delete myToolBarDrag;
}


void
GUISUMOViewParent::create()
{
    GUIGlChildWindow::create();
}


long
GUISUMOViewParent::onCmdMakeSnapshot(FXObject*,FXSelector,void*)
{
    // get the new file name
    FXFileDialog opendialog(this, "Save Snapshot");
    opendialog.setIcon(GUIIconSubSys::getIcon(ICON_EMPTY));
    opendialog.setSelectMode(SELECTFILE_ANY);
    opendialog.setPatternList(patterns);
    if (gCurrentFolder.length()!=0) {
        opendialog.setDirectory(gCurrentFolder);
    }
    if (!opendialog.execute()||!MFXUtils::userPermitsOverwritingWhenFileExists(this, opendialog.getFilename())) {
        return 1;
    }
    gCurrentFolder = opendialog.getDirectory();
    string file = opendialog.getFilename().text();
    FXColor *buf = myView->getSnapshot();
    // save
    try {
        MFXImageHelper::saveimage(getApp(), file,
                                  myView->getWidth(), myView->getHeight(), buf);
    } catch (...) {
        string msg = "Could not save '" + file + "'.\nMaybe the extension is unknown.";
        FXMessageBox::error(this, MBOX_OK, "Saving failed.",
                            msg.c_str());
    }
    FXFREE(&buf);
    return 1;
}

/*
long
GUISUMOViewParent::onCmdAllowRotation(FXObject*sender,FXSelector,void*)
{
    MFXCheckableButton *button = static_cast<MFXCheckableButton*>(sender);
    button->setChecked(!button->amChecked());
    myAllowRotation = button->amChecked();
    return 1;
}
*/


void
GUISUMOViewParent::showLocator(GUIGlObjectType type)
{
    myLocatorPopup->popdown();
    myLocatorButton->killFocus();
    myLocatorPopup->update();
    GUIDialog_GLObjChooser *chooser =
        new GUIDialog_GLObjChooser(this, type, gIDStorage);
    chooser->create();
    chooser->show();
}


long
GUISUMOViewParent::onCmdLocateJunction(FXObject *,FXSelector,void*)
{
    showLocator(GLO_JUNCTION);
    return 1;
}


long
GUISUMOViewParent::onCmdLocateEdge(FXObject *,FXSelector,void*)
{
    showLocator(GLO_EDGE);
    return 1;
}


long
GUISUMOViewParent::onCmdLocateVehicle(FXObject *,FXSelector,void*)
{
    showLocator(GLO_VEHICLE);
    return 1;
}


long
GUISUMOViewParent::onCmdLocateTLS(FXObject *,FXSelector,void*)
{
    showLocator(GLO_TLLOGIC);
    return 1;
}


long
GUISUMOViewParent::onCmdLocateAdd(FXObject *,FXSelector,void*)
{
    showLocator(GLO_ADDITIONAL);
    return 1;
}


long
GUISUMOViewParent::onCmdLocateShape(FXObject *sender,FXSelector,void*)
{
    myLocatorPopup->popdown();
    myLocatorButton->killFocus();
    myLocatorPopup->update();
    static_cast<FXButton*>(sender)->getParent()->hide();
    GUIDialog_GLObjChooser *chooser =
        new GUIDialog_GLObjChooser(this, GLO_SHAPE, gIDStorage);
    chooser->create();
    chooser->show();
    return 1;
}





void
GUISUMOViewParent::setView(GUIGlObject *o)
{
    myView->centerTo(o);
}


int
GUISUMOViewParent::getMaxGLWidth() const
{
    return myParent->getMaxGLWidth();
}


int
GUISUMOViewParent::getMaxGLHeight() const
{
    return myParent->getMaxGLHeight();
}


long
GUISUMOViewParent::onSimStep(FXObject*,FXSelector,void*)
{
    myView->update();
    return 1;
}



/****************************************************************************/

