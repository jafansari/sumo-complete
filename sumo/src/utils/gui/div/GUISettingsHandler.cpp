/****************************************************************************/
/// @file    GUISettingsHandler.cpp
/// @author  Michael Behrisch
/// @date    Fri, 24. Apr 2009
/// @version $Id$
///
// The dialog to change the view (gui) settings.
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
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

#include <vector>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/RGBColor.h>
#include <utils/common/MsgHandler.h>
#include <utils/gui/windows/GUIVisualizationSettings.h>
#include <utils/gui/drawer/GUICompleteSchemeStorage.h>
#include <utils/foxtools/MFXImageHelper.h>
#include "GUISettingsHandler.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUISettingsHandler::GUISettingsHandler(std::string file) throw()
        : myZoom(-1), myXPos(-1), myYPos(-1), mySnapshotFile("") {
    XMLSubSys::runParser(*this, file);
}


GUISettingsHandler::~GUISettingsHandler() throw() {
}


void
GUISettingsHandler::myStartElement(SumoXMLTag element,
        const SUMOSAXAttributes &attrs) throw(ProcessError) {
    switch (element) {
    case SUMO_TAG_VIEWPORT:
        myZoom = attrs.getFloatSecure(SUMO_ATTR_ZOOM, myZoom);
        myXPos = attrs.getFloatSecure(SUMO_ATTR_X, myXPos);
        myYPos = attrs.getFloatSecure(SUMO_ATTR_Y, myYPos);
        break;
    case SUMO_TAG_SNAPSHOT:
        mySnapshotFile = attrs.getStringSecure(SUMO_ATTR_FILE, "");
        break;
    case SUMO_TAG_VIEWSETTINGS_SCHEME:
        mySettings.name = attrs.getStringSecure("name", mySettings.name);
        break;
    case SUMO_TAG_VIEWSETTINGS_OPENGL:
        mySettings.antialiase = TplConvert<char>::_2bool(attrs.getStringSecure("antialiase", toString(mySettings.antialiase)).c_str());
        mySettings.dither = TplConvert<char>::_2bool(attrs.getStringSecure("dither", toString(mySettings.dither)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_BACKGROUND:
        mySettings.backgroundColor = RGBColor::parseColor(attrs.getStringSecure("backgroundColor", toString(mySettings.backgroundColor)));
        mySettings.showGrid = TplConvert<char>::_2bool(attrs.getStringSecure("showGrid", toString(mySettings.showGrid)).c_str());
        mySettings.gridXSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("gridXSize", toString(mySettings.gridXSize)).c_str());
        mySettings.gridYSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("gridYSize", toString(mySettings.gridYSize)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_EDGES:
        mySettings.laneEdgeMode = TplConvert<char>::_2int(attrs.getStringSecure("laneEdgeMode", toString(mySettings.laneEdgeMode)).c_str());
        mySettings.laneShowBorders = TplConvert<char>::_2bool(attrs.getStringSecure("laneShowBorders", toString(mySettings.laneShowBorders)).c_str());
        mySettings.showLinkDecals = TplConvert<char>::_2bool(attrs.getStringSecure("showLinkDecals", toString(mySettings.showLinkDecals)).c_str());
        mySettings.showRails = TplConvert<char>::_2bool(attrs.getStringSecure("showRails", toString(mySettings.showRails)).c_str());
        mySettings.drawEdgeName = TplConvert<char>::_2bool(attrs.getStringSecure("drawEdgeName", toString(mySettings.drawEdgeName)).c_str());
        mySettings.edgeNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("edgeNameSize", toString(mySettings.edgeNameSize)).c_str());
        mySettings.edgeNameColor = RGBColor::parseColor(attrs.getStringSecure("edgeNameColor", toString(mySettings.edgeNameColor)));
        mySettings.drawInternalEdgeName = TplConvert<char>::_2bool(attrs.getStringSecure("drawInternalEdgeName", toString(mySettings.drawInternalEdgeName)).c_str());
        mySettings.internalEdgeNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("internalEdgeNameSize", toString(mySettings.internalEdgeNameSize)).c_str());
        mySettings.internalEdgeNameColor = RGBColor::parseColor(attrs.getStringSecure("internalEdgeNameColor", toString(mySettings.internalEdgeNameColor)));
        mySettings.hideConnectors = TplConvert<char>::_2bool(attrs.getStringSecure("hideConnectors", toString(mySettings.hideConnectors)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_EDGE_COLOR_ITEM: {
        int index = TplConvert<char>::_2int(attrs.getStringSecure("index", "").c_str());
        RGBColor value = RGBColor::parseColor(attrs.getStringSecure("value", "1,1,0"));
        if (mySettings.laneColorings.find(index)==mySettings.laneColorings.end()) {
            mySettings.laneColorings[index] = std::vector<RGBColor>();
        }
        mySettings.laneColorings[index].push_back(value);
    }
    break;
    case SUMO_TAG_VIEWSETTINGS_VEHICLES:
        mySettings.vehicleMode = TplConvert<char>::_2int(attrs.getStringSecure("vehicleMode", toString(mySettings.vehicleMode)).c_str());
        mySettings.vehicleQuality = TplConvert<char>::_2int(attrs.getStringSecure("vehicleQuality", toString(mySettings.vehicleQuality)).c_str());
        mySettings.minVehicleSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("minVehicleSize", toString(mySettings.minVehicleSize)).c_str());
        mySettings.vehicleExaggeration = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("vehicleExaggeration", toString(mySettings.vehicleExaggeration)).c_str());
        mySettings.showBlinker = TplConvert<char>::_2bool(attrs.getStringSecure("showBlinker", toString(mySettings.showBlinker)).c_str());
        mySettings.drawVehicleName = TplConvert<char>::_2bool(attrs.getStringSecure("drawVehicleName", toString(mySettings.drawVehicleName)).c_str());
        mySettings.vehicleNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("vehicleNameSize", toString(mySettings.vehicleNameSize)).c_str());
        mySettings.vehicleNameColor = RGBColor::parseColor(attrs.getStringSecure("vehicleNameColor", toString(mySettings.vehicleNameColor)));
        break;
    case SUMO_TAG_VIEWSETTINGS_VEHICLE_COLOR_ITEM: {
        int index = TplConvert<char>::_2int(attrs.getStringSecure("index", "").c_str());
        RGBColor value = RGBColor::parseColor(attrs.getStringSecure("value", "1,1,0"));
        if (mySettings.vehicleColorings.find(index)==mySettings.vehicleColorings.end()) {
            mySettings.vehicleColorings[index] = std::vector<RGBColor>();
        }
        mySettings.vehicleColorings[index].push_back(value);
    }
    break;
    case SUMO_TAG_VIEWSETTINGS_JUNCTIONS:
        mySettings.junctionMode = TplConvert<char>::_2int(attrs.getStringSecure("junctionMode", toString(mySettings.junctionMode)).c_str());
        mySettings.drawLinkTLIndex = TplConvert<char>::_2bool(attrs.getStringSecure("drawLinkTLIndex", toString(mySettings.drawLinkTLIndex)).c_str());
        mySettings.drawLinkJunctionIndex = TplConvert<char>::_2bool(attrs.getStringSecure("drawLinkJunctionIndex", toString(mySettings.drawLinkJunctionIndex)).c_str());
        mySettings.drawJunctionName = TplConvert<char>::_2bool(attrs.getStringSecure("drawJunctionName", toString(mySettings.drawJunctionName)).c_str());
        mySettings.junctionNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("junctionNameSize", toString(mySettings.junctionNameSize)).c_str());
        mySettings.junctionNameColor = RGBColor::parseColor(attrs.getStringSecure("junctionNameColor", toString(mySettings.junctionNameColor)));
        mySettings.showLane2Lane = TplConvert<char>::_2bool(attrs.getStringSecure("showLane2Lane", toString(mySettings.showLane2Lane)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_ADDITIONALS:
        mySettings.addMode = TplConvert<char>::_2int(attrs.getStringSecure("addMode", toString(mySettings.addMode)).c_str());
        mySettings.minAddSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("minAddSize", toString(mySettings.minAddSize)).c_str());
        mySettings.addExaggeration = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("addExaggeration", toString(mySettings.addExaggeration)).c_str());
        mySettings.drawAddName = TplConvert<char>::_2bool(attrs.getStringSecure("drawAddName", toString(mySettings.drawAddName)).c_str());
        mySettings.addNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("addNameSize", toString(mySettings.addNameSize)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_POIS:
        mySettings.poiExaggeration = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("poiExaggeration", toString(mySettings.poiExaggeration)).c_str());
        mySettings.minPOISize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("minPOISize", toString(mySettings.minPOISize)).c_str());
        mySettings.drawPOIName = TplConvert<char>::_2bool(attrs.getStringSecure("drawPOIName", toString(mySettings.drawPOIName)).c_str());
        mySettings.poiNameSize = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("poiNameSize", toString(mySettings.poiNameSize)).c_str());
        mySettings.poiNameColor = RGBColor::parseColor(attrs.getStringSecure("poiNameColor", toString(mySettings.poiNameColor)));
        break;
    case SUMO_TAG_VIEWSETTINGS_LEGEND:
        mySettings.showSizeLegend = TplConvert<char>::_2bool(attrs.getStringSecure("showSizeLegend", toString(mySettings.showSizeLegend)).c_str());
        break;
    case SUMO_TAG_VIEWSETTINGS_DECAL: {
        GUISUMOAbstractView::Decal d;
        d.filename = attrs.getStringSecure("filename", d.filename);
        d.centerX = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("centerX", toString(d.centerX)).c_str());
        d.centerY = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("centerY", toString(d.centerY)).c_str());
        d.width = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("width", toString(d.width)).c_str());
        d.height = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("height", toString(d.height)).c_str());
        d.rot = TplConvert<char>::_2SUMOReal(attrs.getStringSecure("rotation", toString(d.rot)).c_str());
        d.initialised = false;
        myDecals.push_back(d);
    }
    break;
    }
}


std::string
GUISettingsHandler::addSettings(GUISUMOAbstractView* view) throw() {
    if (mySettings.name != "") {
        gSchemeStorage.add(mySettings);
        size_t index = view->getColoringSchemesCombo().appendItem(mySettings.name.c_str());
        view->getColoringSchemesCombo().setCurrentItem(index);
        view->setColorScheme(mySettings.name.c_str());
    }
    return mySettings.name;
}


void
GUISettingsHandler::setViewport(GUISUMOAbstractView* view) throw() {
    if (myZoom > 0) {
        view->setViewport(myZoom, myXPos, myYPos);
    }
}


void
GUISettingsHandler::makeSnapshot(GUISUMOAbstractView* view) throw() {
    if (mySnapshotFile != "") {
        FXColor *buf = view->getSnapshot();
        try {
            if (!MFXImageHelper::saveImage(mySnapshotFile, view->getWidth(), view->getHeight(), buf)) {
                MsgHandler::getWarningInstance()->inform("Could not save '" + mySnapshotFile + "'.");
            }
        } catch (InvalidArgument e) {
            MsgHandler::getWarningInstance()->inform("Could not save '" + mySnapshotFile + "'.\n" + e.what());
        }
        FXFREE(&buf);
    }
}


bool
GUISettingsHandler::hasDecals() throw() {
    return !myDecals.empty();
}


std::vector<GUISUMOAbstractView::Decal>
GUISettingsHandler::getDecals() throw() {
    return myDecals;
}

/****************************************************************************/

