/****************************************************************************/
/// @file    guisim_main.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Main for GUISIM
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

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <ctime>
#include <signal.h>
#include <iostream>
#include <fx.h>
#include <fx3d.h>
#include <microsim/MSNet.h>
#include <microsim/MSEmitControl.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/xml/XMLSubSys.h>
#include <gui/GUIApplicationWindow.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <gui/GUIThreadFactory.h>
#include <guisim/GUIEdge.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/drawer/GUIColoringSchemesMap.h>
#include <gui/drawerimpl/GUIVehicleDrawer.h>
#include <utils/gui/div/GUIFrame.h>
#include <utils/gui/drawer/GUIGradients.h>
#include <utils/gui/drawer/GUIColorer_SingleColor.h>
#include <utils/gui/windows/GUIAppGlobals.h>
#include <utils/gui/images/GUIImageGlobals.h>
#include <utils/gui/drawer/GUICompleteSchemeStorage.h>
#include <gui/GUIViewTraffic.h>

#ifdef HAVE_MESOSIM
#include <mesogui/GUIViewMesoEdges.h>
#endif

#include <gui/GUIColorer_LaneByPurpose.h>
#include <utils/gui/drawer/GUIColorer_LaneBySelection.h>
#include <gui/GUIColorer_LaneByVehKnowledge.h>
#include <gui/GUIColorer_LaneNeighEdges.h>
#include <utils/gui/drawer/GUIColorer_SingleColor.h>
#include <utils/gui/drawer/GUIColorer_ShadeByFunctionValue.h>
#include <utils/gui/drawer/GUIColorer_ShadeByCastedFunctionValue.h>
#include <utils/gui/drawer/GUIColorer_ColorSettingFunction.h>
#include <utils/gui/drawer/GUIColorer_ByDeviceNumber.h>
#include <utils/gui/drawer/GUIColorer_ByOptCORNValue.h>
#include <guisim/GUIVehicle.h>


#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// methods
// ===========================================================================
/* -------------------------------------------------------------------------
 * coloring schemes initialisation
 * ----------------------------------------------------------------------- */
map<int, vector<RGBColor> >
initVehicleColoringSchemes()
{
    map<int, vector<RGBColor> > vehColMap;
    // insert possible vehicle coloring schemes
    GUIColoringSchemesMap<GUIVehicle> &sm = GUIViewTraffic::getVehiclesSchemesMap();
    // from read/assigned colors
    sm.add("given/assigned vehicle color",
           new GUIColorer_ColorSettingFunction<GUIVehicle>(
               (void(GUIVehicle::*)() const) &GUIVehicle::setOwnDefinedColor));
    sm.add("given/assigned type color",
           new GUIColorer_ColorSettingFunction<GUIVehicle>(
               (void(GUIVehicle::*)() const) &GUIVehicle::setOwnTypeColor));
    sm.add("given/assigned route color",
           new GUIColorer_ColorSettingFunction<GUIVehicle>(
               (void(GUIVehicle::*)() const) &GUIVehicle::setOwnRouteColor));
    // from a vehicle's standard values
    sm.add("by speed",
           new GUIColorer_ShadeByFunctionValue<GUIVehicle>(
               0, (SUMOReal)(150.0/3.6), RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUIVehicle::*)() const) &GUIVehicle::getSpeed));
    sm.add("by waiting time",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, size_t>(
               0, (SUMOReal)(5*60), RGBColor(0, 0, 1), RGBColor(1, 0, 0),
               (size_t(GUIVehicle::*)() const) &GUIVehicle::getWaitingTime));
    sm.add("by time since last lanechange",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, size_t>(
               0, (SUMOReal)(5*60), RGBColor(1, 1, 1), RGBColor((SUMOReal) .5, (SUMOReal) .5, (SUMOReal) .5),
               (size_t(GUIVehicle::*)() const) &GUIVehicle::getLastLaneChangeOffset));
    sm.add("by max speed",
           new GUIColorer_ShadeByFunctionValue<GUIVehicle>(
               0, (SUMOReal)(150.0/3.6), RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUIVehicle::*)() const) &GUIVehicle::getMaxSpeed));
    // ... and some not always used values
    sm.add("by reroute number",
           new GUIColorer_ByOptCORNValue<GUIVehicle, MSCORN::Function>(
               (bool(GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::hasCORNIntValue,
               (int(GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::getCORNIntValue,
               true, 1, 10,
               RGBColor(1,0,0), RGBColor(1,1,0), RGBColor(1,1,1),
               MSCORN::CORN_VEH_NUMBERROUTE));
    /*
    sm.add("by time since last reroute",
        new GUIColorer_ByDeviceNumber<GUIVehicle, MSCORN::Function>(
            (bool (GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::hasCORNIntValue,
            (SUMOReal (GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::getCORNIntValue,
            true, 1, 10,
            RGBColor(1,0,0), RGBColor(1,1,0), RGBColor(1,1,1),
            MSCORN::CORN_VEH_NUMBERROUTE));
            */
    // using TOL-extensions
    /*
    sm.add("TOL: by device number",
           new GUIColorer_ByOptCORNValue<GUIVehicle, MSCORN::Function>(
               (bool(GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::hasCORNIntValue,
               (int(GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::getCORNIntValue,
               true, 1, 10,
               RGBColor(1,0,0), RGBColor(1,1,0), RGBColor(1,1,1),
               MSCORN::CORN_VEH_DEV_NO_CPHONE));
    /
    sm.add("by device state",
        new GUIColorer_ByOptCORNValue<GUIVehicle, MSCORN::Function>(
            (bool (GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::hasCORNDoubleValue,
            (SUMOReal (GUIVehicle::*)(MSCORN::Function) const) &GUIVehicle::getCORNDoubleValue,
            true, 1, 10,
            RGBColor(1,0,0), RGBColor(1,1,0), RGBColor(1,1,1),
            MSCORN::CORN_VEH_DEV_NO_CPHONE));
            */
    // using C2C extensions
    sm.add("C2C: by having a device",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, bool>(
               0, 1, RGBColor((SUMOReal) .8, (SUMOReal) .8, (SUMOReal) .8), RGBColor(1, 1, 0),
               (bool(GUIVehicle::*)() const) &GUIVehicle::isEquipped));
    sm.add("C2C: by number of all information",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, int>(
               0, (SUMOReal)(10), RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (int(GUIVehicle::*)() const) &GUIVehicle::getTotalInformationNumber));  // !!!
    sm.add("C2C: by information number",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, size_t>(
               0, (SUMOReal)(200), RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (size_t(GUIVehicle::*)() const) &GUIVehicle::getInformationNumber));
    sm.add("C2C: by connections number",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, size_t>(
               0, (SUMOReal)(3), RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (size_t(GUIVehicle::*)() const) &GUIVehicle::getConnectionsNumber));
    sm.add("C2C: by having a route information",
           new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, bool>(
               0, 1, RGBColor((SUMOReal) .5, (SUMOReal) .5, (SUMOReal) .5), RGBColor(1, 1, 1),
               (bool(GUIVehicle::*)() const) &GUIVehicle::hasRouteInformation));
    /*
    sm.add("C2C: by last information time",
        new GUIColorer_ShadeByCastedFunctionValue<GUIVehicle, bool>(
            0, 1, RGBColor((SUMOReal) .5, (SUMOReal) .5, (SUMOReal) .5), RGBColor(1, 1, 1),
            (bool (GUIVehicle::*)() const) &GUIVehicle::getLastInfoTime));
            */
    // build the colors map
    {
        for (size_t i=0; i<sm.size(); i++) {
            vehColMap[i] = vector<RGBColor>();
            switch (sm.getColorSetType(i)) {
            case CST_SINGLE:
                vehColMap[i].push_back(sm.getColorerInterface(i)->getSingleColor());
                break;
            case CST_MINMAX:
                vehColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                vehColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                break;
            case CST_MINMAX_OPT:
                vehColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                vehColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                vehColMap[i].push_back(sm.getColorerInterface(i)->getFallbackColor());
                break;
            default:
                break;
            }
        }
    }
    return vehColMap;
}


map<int, vector<RGBColor> >
initLaneColoringSchemes()
{
    map<int, vector<RGBColor> > laneColMap;
    // insert possible lane coloring schemes
    GUIColoringSchemesMap<GUILaneWrapper> &sm = GUIViewTraffic::getLaneSchemesMap();
    // insert possible lane coloring schemes
    //
    sm.add("uniform",
           new GUIColorer_SingleColor<GUILaneWrapper>(RGBColor(0, 0, 0)));
    sm.add("by selection (lane-/streetwise)",
           new GUIColorer_LaneBySelection<GUILaneWrapper>());
    sm.add("by purpose (lanewise)",
           new GUIColorer_LaneByPurpose<GUILaneWrapper>());
    // from a lane's standard values
    sm.add("by allowed speed (lanewise)",
           new GUIColorer_ShadeByFunctionValue<GUILaneWrapper>(
               0, (SUMOReal)(150.0/3.6),
               RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUILaneWrapper::*)() const) &GUILaneWrapper::maxSpeed));
    sm.add("by current density (lanewise)",
           new GUIColorer_ShadeByFunctionValue<GUILaneWrapper>(
               0, (SUMOReal) .95,
               RGBColor(0, 1, 0), RGBColor(1, 0, 0),
               (SUMOReal(GUILaneWrapper::*)() const) &GUILaneWrapper::getDensity));
    sm.add("by first vehicle waiting time (lanewise)",
           new GUIColorer_ShadeByFunctionValue<GUILaneWrapper>(
               0, 200,
               RGBColor(0, 1, 0), RGBColor(1, 0, 0),
               (SUMOReal(GUILaneWrapper::*)() const) &GUILaneWrapper::firstWaitingTime));
    sm.add("by lane number (streetwise)",
           new GUIColorer_ShadeByFunctionValue<GUILaneWrapper>(
               0, (SUMOReal) 5,
               RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUILaneWrapper::*)() const) &GUILaneWrapper::getEdgeLaneNumber));
    /*
    sm.add("by loaded weights (streetwise)",
           new GUIColorer_ShadeByParametrisedFunctionValue<GUILaneWrapper>(
               0, (SUMOReal) 5,
               RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUILaneWrapper::*)(SUMOTime) const) &GUILaneWrapper::getEdgeEffort));
               */
    // using C2C extensions
    /*
    sm.add("C2C: by vehicle knowledge",
    new GUIColorer_LaneByVehKnowledge<GUILaneWrapper>(this));
    sm.add("C2C: by edge neighborhood",
    new GUIColorer_LaneNeighEdges<GUILaneWrapper>(this));
        */
    // build the colors map
    {
        for (size_t i=0; i<sm.size(); i++) {
            laneColMap[i] = vector<RGBColor>();
            switch (sm.getColorSetType(i)) {
            case CST_SINGLE:
                laneColMap[i].push_back(sm.getColorerInterface(i)->getSingleColor());
                break;
            case CST_MINMAX:
                laneColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                laneColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                break;
            case CST_MINMAX_OPT:
                laneColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                laneColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                laneColMap[i].push_back(sm.getColorerInterface(i)->getFallbackColor());
                break;
            default:
                break;
            }
        }
    }
    return laneColMap;
}

#ifdef HAVE_MESOSIM
map<int, vector<RGBColor> >
initEdgeColoringSchemes()
{
    map<int, vector<RGBColor> > edgeColMap;
    // insert possible lane coloring schemes
    GUIColoringSchemesMap<GUIEdge> &sm = GUIViewMesoEdges::getLaneSchemesMap();
    // insert possible lane coloring schemes
    //
    sm.add("uniform",
           new GUIColorer_SingleColor<GUIEdge>(RGBColor(0, 0, 0)));
    sm.add("by selection (lanewise)",
           new GUIColorer_LaneBySelection<GUIEdge>());
    sm.add("by purpose (lanewise)",
           new GUIColorer_LaneByPurpose<GUIEdge>());
    // from a lane's standard values
    sm.add("by allowed speed (lanewise)",
           new GUIColorer_ShadeByFunctionValue<GUIEdge>(
               0, (SUMOReal)(150.0/3.6),
               RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUIEdge::*)() const) &GUIEdge::getAllowedSpeed));
    sm.add("by current density (lanewise)",
           new GUIColorer_ShadeByFunctionValue<GUIEdge>(
               0, (SUMOReal) .95,
               RGBColor(0, 1, 0), RGBColor(1, 0, 0),
               (SUMOReal(GUIEdge::*)() const) &GUIEdge::getDensity));
    /*
    sm.add("by loaded weights (streetwise)",
           new GUIColorer_ShadeByParametrisedFunctionValue<GUILaneWrapper>(
               0, (SUMOReal) 5,
               RGBColor(1, 0, 0), RGBColor(0, 0, 1),
               (SUMOReal(GUILaneWrapper::*)(SUMOTime) const) &GUILaneWrapper::getEdgeEffort));
               */
    // using C2C extensions
    /*
    sm.add("C2C: by vehicle knowledge",
    new GUIColorer_LaneByVehKnowledge<GUILaneWrapper>(this));
    sm.add("C2C: by edge neighborhood",
    new GUIColorer_LaneNeighEdges<GUILaneWrapper>(this));
        */
    // build the colors map
    {
        for (size_t i=0; i<sm.size(); i++) {
            edgeColMap[i] = vector<RGBColor>();
            switch (sm.getColorSetType(i)) {
            case CST_SINGLE:
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getSingleColor());
                break;
            case CST_MINMAX:
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                break;
            case CST_MINMAX_OPT:
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
                edgeColMap[i].push_back(sm.getColorerInterface(i)->getFallbackColor());
                break;
            default:
                break;
            }
        }
    }
    return edgeColMap;
}
#endif

void
initColoringSchemes(FXApp *a)
{
    map<int, vector<RGBColor> > vehColMap = initVehicleColoringSchemes();
    map<int, vector<RGBColor> > laneColMap = initLaneColoringSchemes();
#ifdef HAVE_MESOSIM
    map<int, vector<RGBColor> > edgeColMap = initEdgeColoringSchemes();
#endif
    // initialise gradients
    myDensityGradient = gGradients->getRGBColors(GUIGradientStorage::GRADIENT_GREEN_YELLOW_RED, 101);
    // initialise available coloring schemes
#ifdef HAVE_MESOSIM
    gSchemeStorage.init(a, vehColMap, laneColMap, edgeColMap);
#else
    gSchemeStorage.init(a, vehColMap, laneColMap, map<int, vector<RGBColor> >());
#endif
}


void
deleteColoringSchemes()
{
    delete &GUIViewTraffic::getVehiclesSchemesMap();
    delete &GUIViewTraffic::getLaneSchemesMap();
#ifdef HAVE_MESOSIM
    delete &GUIViewMesoEdges::getLaneSchemesMap();
#endif
}


/* -------------------------------------------------------------------------
 * options initialisation
 * ----------------------------------------------------------------------- */
void
fillOptions()
{
    OptionsCont &oc = OptionsCont::getOptions();
    oc.addCallExample("");
    oc.addCallExample("-c <CONFIGURATION>");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Process");
    oc.addOptionSubTopic("Visualisation");
    oc.addOptionSubTopic("Open GL");
    oc.addOptionSubTopic("Report");


    // insert options
    GUIFrame::fillInitOptions();
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    // make the output aware of threading
    MFXMutex lock;
    MsgHandler::assignLock(&lock);
    // get the options
    OptionsCont &oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("GUI version of the simulation SUMO.");
#ifdef WIN32
    oc.setApplicationName("guisim.exe", "SUMO guisim Version " + (string)VERSION_STRING);
#else
    oc.setApplicationName("sumo-guisim", "SUMO guisim Version " + (string)VERSION_STRING);
#endif
    int ret = 0;
#ifndef _DEBUG
    try {
#else
    {
#endif
        // initialise subsystems
        XMLSubSys::init();
        fillOptions();
        OptionsIO::getOptions(false, argc, argv);
        if(oc.processMetaOptions(false)) {
            SystemFrame::close();
            return 0;
        }
        GUIFrame::checkInitOptions();
        // within gui-based applications, nothing is reported to the console
        MsgHandler::getErrorInstance()->report2cout(false);
        MsgHandler::getErrorInstance()->report2cerr(false);
        MsgHandler::getWarningInstance()->report2cout(false);
        MsgHandler::getWarningInstance()->report2cerr(false);
        MsgHandler::getMessageInstance()->report2cout(false);
        MsgHandler::getMessageInstance()->report2cerr(false);
        // Make application
        FXApp application("SUMO GUISimulation","DLR+ZAIK");
        gFXApp = &application;
        // Open display
        application.init(argc,argv);
        int minor, major;
        if (!FXGLVisual::supported(&application, major, minor)) {
            throw ProcessError("This system has no OpenGL support. Exiting.");
        }
        // initialise global settings
        gQuitOnEnd = oc.getBool("quit-on-end");
        gAllowTextures = !oc.getBool("disable-textures");
        gSuppressEndInfo = oc.getBool("suppress-end-info");
        bool useConfig = oc.isSet("configuration-file");
        string configFile = useConfig ? oc.getString("configuration-file") : "";
        bool runAfterLoad = !oc.isSet("no-start");

        // build the main window
        GUIThreadFactory tf;
        GUIApplicationWindow * window =
            new GUIApplicationWindow(&application,
                                     oc.getInt("w"), oc.getInt("h"), "*.sumo.cfg");
        window->dependentBuild(tf);
        gGradients = new GUIGradientStorage(window);
        initColoringSchemes(&application);
        // delete startup-options
        OptionsCont::getOptions().clear();
        // Create app
        application.addSignal(SIGINT,window, MID_QUIT);
        application.create();
        // Load configuration given oncommand line
        if (useConfig) {
            window->loadOnStartup(configFile, runAfterLoad);
        }
        // Run
        ret = application.run();
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    }
#else
    }
#endif
    SystemFrame::close();
    return ret;
}



/****************************************************************************/

