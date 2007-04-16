/****************************************************************************/
/// @file    GUITrafficLightLogicWrapper.cpp
/// @author  Daniel Krajzewicz
/// @date    Oct/Nov 2003
/// @version $Id$
///
// A wrapper for tl-logics to allow their visualisation and interaction
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
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <cassert>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <gui/GUIApplicationWindow.h>
#include <microsim/MSLane.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <guisim/guilogging/GLObjectValuePassConnector.h>
#include <microsim/logging/FunctionBinding.h>
#include <microsim/logging/FuncBinding_StringParam.h>
#include <utils/gui/tracker/GUITLLogicPhasesTrackerWindow.h>
#include "GUITrafficLightLogicWrapper.h"
#include <utils/gui/div/GUIGlobalSelection.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu)
GUITrafficLightLogicWrapperPopupMenuMap[]=
    {
        FXMAPFUNC(SEL_COMMAND,  MID_SHOWPHASES,             GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdShowPhases),
        FXMAPFUNC(SEL_COMMAND,  MID_TRACKPHASES,            GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdBegin2TrackPhases),
        FXMAPFUNCS(SEL_COMMAND, MID_SWITCH, MID_SWITCH+20, GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdSwitchTLSLogic),
    };

// Object implementation
FXIMPLEMENT(GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu, GUIGLObjectPopupMenu, GUITrafficLightLogicWrapperPopupMenuMap, ARRAYNUMBER(GUITrafficLightLogicWrapperPopupMenuMap))


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu - methods
 * ----------------------------------------------------------------------- */
GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::GUITrafficLightLogicWrapperPopupMenu(
    GUIMainWindow &app, GUISUMOAbstractView &parent,
    GUIGlObject &o)
        : GUIGLObjectPopupMenu(app, parent, o)
{}


GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::~GUITrafficLightLogicWrapperPopupMenu()
{}



long
GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdBegin2TrackPhases(
    FXObject*,FXSelector,void*)
{
    assert(myObject->getType()==GLO_TLLOGIC);
    static_cast<GUITrafficLightLogicWrapper*>(myObject)->begin2TrackPhases();
    return 1;
}


long
GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdShowPhases(
    FXObject*,FXSelector,void*)
{
    assert(myObject->getType()==GLO_TLLOGIC);
    static_cast<GUITrafficLightLogicWrapper*>(myObject)->showPhases();
    return 1;
}


long
GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapperPopupMenu::onCmdSwitchTLSLogic(
    FXObject*,FXSelector sel,void*)
{
    assert(myObject->getType()==GLO_TLLOGIC);
    static_cast<GUITrafficLightLogicWrapper*>(myObject)->switchTLSLogic(FXSELID(sel)-MID_SWITCH);
    return 1;
}



/* -------------------------------------------------------------------------
 * GUITrafficLightLogicWrapper - methods
 * ----------------------------------------------------------------------- */
GUITrafficLightLogicWrapper::GUITrafficLightLogicWrapper(
    GUIGlObjectStorage &idStorage,
    MSTLLogicControl &control, MSTrafficLightLogic &tll)
        : GUIGlObject(idStorage, "tl-logic:"+tll.getID()),
        myTLLogicControl(control), myTLLogic(tll)
{}


GUITrafficLightLogicWrapper::~GUITrafficLightLogicWrapper()
{}


GUIGLObjectPopupMenu *
GUITrafficLightLogicWrapper::getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent)
{
    myApp = &app;
    GUIGLObjectPopupMenu *ret = new GUITrafficLightLogicWrapperPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    //
    const MSTLLogicControl::TLSLogicVariants &vars = myTLLogicControl.get(myTLLogic.getID());
    if (vars.ltVariants.size()>1) {
        std::map<std::string, MSTrafficLightLogic*>::const_iterator i;
        size_t index = 0;
        for (i=vars.ltVariants.begin(); i!=vars.ltVariants.end(); ++i, ++index) {
            if ((*i).second!=vars.defaultTL) {
                new FXMenuCommand(ret, ("Switch to '" + (*i).second->getSubID() + "'").c_str(),
                                  GUIIconSubSys::getIcon(ICON_FLAG_MINUS), ret, MID_SWITCH+index);
            }
        }
        new FXMenuSeparator(ret);
    }
    new FXMenuCommand(ret, "Track Phases", 0, ret, MID_TRACKPHASES);
    new FXMenuCommand(ret, "Show Phases", 0, ret, MID_SHOWPHASES);
    new FXMenuSeparator(ret);
    //
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret, false);
    return ret;
}


void
GUITrafficLightLogicWrapper::begin2TrackPhases()
{
    GUITLLogicPhasesTrackerWindow *window =
        new GUITLLogicPhasesTrackerWindow(*myApp, myTLLogic, *this,
                                          new FuncBinding_StringParam<MSTLLogicControl, CompletePhaseDef>
                                          (&MSNet::getInstance()->getTLSControl(), &MSTLLogicControl::getPhaseDef, myTLLogic.getID()));
    window->create();
    window->show();
}


void
GUITrafficLightLogicWrapper::showPhases()
{
    GUITLLogicPhasesTrackerWindow *window =
        new GUITLLogicPhasesTrackerWindow(*myApp, myTLLogic, *this,
                                          static_cast<MSSimpleTrafficLightLogic&>(myTLLogic).getPhases());
    window->setBeginTime(0);
    window->create();
    window->show();
}


GUIParameterTableWindow *
GUITrafficLightLogicWrapper::getParameterWindow(GUIMainWindow &,
        GUISUMOAbstractView &)
{
    return 0;
}


GUIGlObjectType
GUITrafficLightLogicWrapper::getType() const
{
    return GLO_TLLOGIC;
}


bool
GUITrafficLightLogicWrapper::active() const
{
    return true;
}


const std::string &
GUITrafficLightLogicWrapper::microsimID() const
{
    return myTLLogic.getID();
}


Boundary
GUITrafficLightLogicWrapper::getCenteringBoundary() const
{
    Boundary ret;
    const MSTrafficLightLogic::LaneVectorVector &lanes = myTLLogic.getLanes();
    for(MSTrafficLightLogic::LaneVectorVector::const_iterator i=lanes.begin(); i!=lanes.end(); ++i) {
        const MSTrafficLightLogic::LaneVector &lanes2 = (*i);
        for(MSTrafficLightLogic::LaneVector::const_iterator j=lanes2.begin(); j!=lanes2.end(); ++j) {
            ret.add((*j)->getShape()[-1]);
        }
    }
    ret.grow(20);
    return ret;
}


void
GUITrafficLightLogicWrapper::switchTLSLogic(int to)
{
    const MSTLLogicControl::TLSLogicVariants &vars = myTLLogicControl.get(myTLLogic.getID());
    std::map<std::string, MSTrafficLightLogic*>::const_iterator i;
    int index = 0;
    for (i=vars.ltVariants.begin(); i!=vars.ltVariants.end(); ++i, ++index) {
        if (index==to) {
            myTLLogicControl.switchTo((*i).second->getID(), (*i).second->getSubID());
            return;
        }
    }
}


int
GUITrafficLightLogicWrapper::getLinkIndex(MSLink *link) const
{
    return myTLLogic.getLinkIndex(link);
}



/****************************************************************************/

