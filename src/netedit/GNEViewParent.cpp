//---------------------------------------------------------------------------//
//                        GNEViewParent.cpp -
//  A window that controls the display(s) of the simulation
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 15 Dec 2004
//  copyright            : (C) 2004 by Daniel Krajzewicz
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
// Revision 1.7  2006/08/01 07:31:35  dkrajzew
// API adaptations
//
// Revision 1.6  2006/01/09 11:58:37  dkrajzew
// new visualization settings implemented
//
// Revision 1.5  2005/10/07 11:38:33  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/15 12:03:02  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2005/01/31 09:27:35  dkrajzew
// added the possibility to save nodes and edges or the build network to netedit
//
// Revision 1.2  2005/01/27 14:31:28  dksumo
// netedit now works using an own MDIChild-window; Graph is translated into a MSNet
//
// Revision 1.1  2004/12/15 09:20:19  dkrajzew
// made guisim independent of giant/netedit
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

#include <utils/geom/Position2D.h>
#include <utils/geom/Boundary.h>
#include <guisim/GUIVehicle.h>
#include <microsim/MSJunction.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUINet.h>

#include <string>
#include <vector>
#include <gui/GUIGlobals.h>
#include <gui/dialogs/GUIDialog_GLObjChooser.h>
#include <gui/GUIViewTraffic.h>
#include <gui/GUIApplicationWindow.h>
#include "GNEViewParent.h"
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIcons.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include "GNEViewTraffic.h"

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
FXDEFMAP(GNEViewParent) GNEViewParentMap[]=
{
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,   0,       GNEViewParent::onLeftBtnRelease),

};

// Object implementation
FXIMPLEMENT(GNEViewParent, GUIGlChildWindow, GNEViewParentMap, ARRAYNUMBER(GNEViewParentMap))


/* =========================================================================
 * member method definitions
 * ======================================================================= */
GNEViewParent::GNEViewParent(FXMDIClient* p,
                                     FXGLCanvas *share,FXMDIMenu *mdimenu,
                                     const FXString& name, GUINet &net,
                                     GUIMainWindow *parentWindow,
                                     GUISUMOViewParent::ViewType view,
                                     FXIcon* ic, FXPopup* pup,
                                     FXuint opts,
                                     FXint x, FXint y, FXint w, FXint h)
    : GUISUMOViewParent( p, share, mdimenu, name, net, parentWindow,
            view, ic, pup, opts, x, y, w, h)
{
}


void
GNEViewParent::init(GUISUMOViewParent::ViewType view, FXGLCanvas *share, GUINet &net)
{
    // Make MDI Window Menu
    setTracking();
    FXVerticalFrame *contentFrame =
        new FXVerticalFrame(this,
        FRAME_SUNKEN|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0);
    // build the tool bar
    buildToolBar(contentFrame);
    FXHorizontalFrame *glcanvasFrame =
        new FXHorizontalFrame(contentFrame,
        FRAME_SUNKEN|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,
        0,0,0,0,0,0,0,0);
	switch(view) {
    case MICROSCOPIC_VIEW:
    default:
        if(share!=0) {
			buildEditFrame(glcanvasFrame);
            _view =
                new GNEViewTraffic(glcanvasFrame, *myParent, this, net,
                    myParent->getGLVisual(), share);
        } else {
			buildEditFrame(glcanvasFrame);
            _view =
                new GNEViewTraffic(glcanvasFrame, *myParent, this, net,
                    myParent->getGLVisual());
			//if(!(_view->isInEditMode()))
			//	groupBox->hide();
        }
        break;
    }
    _view->buildViewToolBars(*this);
}


GNEViewParent::~GNEViewParent()
{
}

void
GNEViewParent::buildEditFrame(FXComposite *c)
{
	groupBox=new FXGroupBox(c,"Title Left",
		GROUPBOX_TITLE_LEFT|FRAME_RIDGE|LAYOUT_FILL_Y);
	new FXLabel(groupBox,"Junction(s)");
	FXMatrix *myMatrix1=new FXMatrix(groupBox,2,FRAME_THICK|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT|MATRIX_BY_COLUMNS,0,0,0,0,10,10,10,10, 5,8);
	new FXLabel(myMatrix1,"Name:");
	JunctionNameTextField =new FXTextField(myMatrix1,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix1,"X-Position:");
	xPosTextField =new FXTextField(myMatrix1,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix1,"Y-Position");
	yPosTextField =new FXTextField(myMatrix1,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix1,"Mode");
	FXHorizontalFrame *buttonFrame =
		new FXHorizontalFrame(myMatrix1,
		LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0);
	new FXToggleButton(buttonFrame,"\t\tClick to change junction mode (currently in priority mode).","\t\tClick to change junction mode (currently in trafficlights mode).",GUIIconSubSys::getIcon(ICON_EXTRACT)/*icon1*/,GUIIconSubSys::getIcon(ICON_DILATE)/*icon2*/,NULL,0,ICON_BEFORE_TEXT|JUSTIFY_LEFT|FRAME_RAISED|FRAME_THICK);
	new FXButton(buttonFrame,"\t\ttest\tstatus bar?",GUIIconSubSys::getIcon(ICON_EXTRACT),NULL,0,ICON_BEFORE_TEXT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK);
	new FXButton(myMatrix1,"Add Junction\t\tAdd Junction.",NULL,NULL,0,ICON_BEFORE_TEXT|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);
	new FXButton(myMatrix1,"Delete Junction\t\tDelete Junction.",NULL,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);

	new FXLabel(groupBox,"Lane(s)");
	FXMatrix *myMatrix2=new FXMatrix(groupBox,2,FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT|MATRIX_BY_COLUMNS,0,0,0,0,10,10,10,10, 5,8);
	new FXLabel(myMatrix2,"Name:");
	LaneNameTextField =new FXTextField(myMatrix2,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix2,"Lane Start:");
	LaneStartTextField =new FXTextField(myMatrix2,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix2,"Lane End");
	LaneEndTextField =new FXTextField(myMatrix2,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix2,"Max. Speed");
	MaxSpeedTextField =new FXTextField(myMatrix2,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXLabel(myMatrix2,"Length");
	LengthTextField =new FXTextField(myMatrix2,12,this,
		NULL,JUSTIFY_RIGHT|LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
	new FXButton(myMatrix2,"Add Lane\ttest\tAdd Lane.",NULL,NULL,0,ICON_BEFORE_TEXT|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);
	new FXButton(myMatrix2,"Delete Lane\ttest\tDelete Lane.",NULL,NULL,0,ICON_BEFORE_TEXT|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);
}


FXGroupBox*
GNEViewParent::getEditGroupBox()
{
	return groupBox;
}


void
GNEViewParent::buildToolBar(FXComposite *c)
{
    GUISUMOViewParent::buildToolBar(c);
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


