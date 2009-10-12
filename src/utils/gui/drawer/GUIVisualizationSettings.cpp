/****************************************************************************/
/// @file    GUIVisualizationSettings.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Stores the information about how to visualize structures
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

#include <map>
#include <vector>
#include <guisim/GUILaneWrapper.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUIVehicle.h>
#include <utils/gui/drawer/GUIColoringSchemesMap.h>
#include "GUIVisualizationSettings.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
GUIVisualizationSettings::GUIVisualizationSettings() throw()
        : name(""), antialiase(false), dither(false), vehicleQuality(0),
        backgroundColor(RGBColor((SUMOReal) 1, (SUMOReal) 1, (SUMOReal) 1)),
        showGrid(false), gridXSize(100), gridYSize(100), laneEdgeMode(0),
        laneColorings(),
        laneShowBorders(false), showLinkDecals(true), showRails(true),
        drawEdgeName(false), edgeNameSize(50),
        edgeNameColor(RGBColor((SUMOReal) 1, (SUMOReal) .5, (SUMOReal) 0)),
        drawInternalEdgeName(false), internalEdgeNameSize(25),
        internalEdgeNameColor(RGBColor((SUMOReal) .5, (SUMOReal) .25, (SUMOReal) 0)),
        hideConnectors(false), vehicleMode(0), vehicleColorings(),
        minVehicleSize(1), vehicleExaggeration(1), showBlinker(true),
        drawcC2CRadius(false), drawLaneChangePreference(false),
        drawVehicleName(false), vehicleNameSize(50),
        vehicleNameColor(RGBColor((SUMOReal) .8, (SUMOReal) .6, (SUMOReal) 0)),
        junctionMode(0), drawLinkTLIndex(false), drawLinkJunctionIndex(false),
        drawJunctionName(false), junctionNameSize(50),
        junctionNameColor(RGBColor((SUMOReal) 0, (SUMOReal) 1, (SUMOReal) .5)),
        showLane2Lane(false), addMode(0), minAddSize(1), addExaggeration(1),
        drawAddName(false), addNameSize(50),
        minPOISize(0), poiExaggeration(1), drawPOIName(false), poiNameSize(50),
        poiNameColor(RGBColor((SUMOReal) 1., (SUMOReal) 0, (SUMOReal) .5)),
        showSizeLegend(true) {
    initColorMap(GUILaneWrapper::getSchemesMap(), laneColorings);
    initColorMap(GUIVehicle::getSchemesMap(), vehicleColorings);
}


void
GUIVisualizationSettings::initColorMap(const BaseSchemeInfoSource &sm,
                                       std::map<int, std::vector<RGBColor> > &colMap) {
    for (int i=0; i<(int) sm.size(); ++i) {
        colMap[i] = std::vector<RGBColor>();
        switch (sm.getColorSetType(i)) {
        case CST_SINGLE:
            colMap[i].push_back(sm.getColorerInterface(i)->getSingleColor());
            break;
        case CST_MINMAX:
            colMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
            colMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
            break;
        case CST_MINMAX_OPT:
            colMap[i].push_back(sm.getColorerInterface(i)->getMinColor());
            colMap[i].push_back(sm.getColorerInterface(i)->getMaxColor());
            colMap[i].push_back(sm.getColorerInterface(i)->getFallbackColor());
            break;
        default:
            break;
        }
    }
}


void
GUIVisualizationSettings::save(OutputDevice &dev) const throw(IOError) {
    dev << "<viewsettings>\n";
    dev << "    <scheme name=\"" << name << "\">\n";
    dev << "        <opengl antialiase=\"" << antialiase << "\" dither=\"" << dither << "\"/>\n";
    dev << "        <background backgroundColor=\"" << backgroundColor << "\"\n"
    << "                    showGrid=\"" << showGrid
    << "\" gridXSize=\"" << gridXSize << "\" gridYSize=\"" << gridYSize << "\"/>\n";
    dev << "        <edges laneEdgeMode=\"" << laneEdgeMode
    << "\" laneShowBorders=\"" << laneShowBorders
    << "\" showLinkDecals=\"" << showLinkDecals
    << "\" showRails=\"" << showRails << "\"\n"
    << "               drawEdgeName=\"" << drawEdgeName
    << "\" edgeNameSize=\"" << edgeNameSize
    << "\" edgeNameColor=\"" << edgeNameColor << "\"\n"
    << "               drawInternalEdgeName=\"" << drawInternalEdgeName
    << "\" internalEdgeNameSize=\"" << internalEdgeNameSize
    << "\" internalEdgeNameColor=\"" << internalEdgeNameColor
    << "\" hideConnectors=\"" << hideConnectors
    << "\">\n";
    size_t index = 0;
    std::map<int, std::vector<RGBColor> >::const_iterator j;
    for (j=laneColorings.begin(); j!=laneColorings.end(); ++j, ++index) {
        for (size_t k=0; k<(*j).second.size(); ++k) {
            dev << "            <nlcC index=\"" << toString(index) << "\" value=\"" << (*j).second[k] << "\"/>\n";
        }
    }
    laneColorer.save(dev);
#ifdef HAVE_MESOSIM
    edgeColorer.save(dev);
#endif
    dev << "        </edges>\n";

    dev << "        <vehicles vehicleMode=\"" << vehicleMode
    << "\" vehicleQuality=\"" << vehicleQuality
    << "\" minVehicleSize=\"" << minVehicleSize
    << "\" vehicleExaggeration=\"" << vehicleExaggeration
    << "\" showBlinker=\"" << showBlinker << "\"\n"
    << "                  drawVehicleName=\"" << drawVehicleName
    << "\" vehicleNameSize=\"" << vehicleNameSize
    << "\" vehicleNameColor=\"" << vehicleNameColor << "\">\n";
    for (j=vehicleColorings.begin(), index=0; j!=vehicleColorings.end(); ++j, ++index) {
        for (size_t k=0; k<(*j).second.size(); ++k) {
            dev << "            <nvcC index=\"" << toString(index) << "\" value=\"" << (*j).second[k] << "\"/>\n";
        }
    }
    vehicleColorer.save(dev);
    dev << "        </vehicles>\n";

    dev << "        <junctions junctionMode=\"" << junctionMode
    << "\" drawLinkTLIndex=\"" << drawLinkTLIndex
    << "\" drawLinkJunctionIndex=\"" << drawLinkJunctionIndex << "\"\n"
    << "                   drawJunctionName=\"" << drawJunctionName
    << "\" junctionNameSize=\"" << junctionNameSize
    << "\" junctionNameColor=\"" << junctionNameColor
    << "\" showLane2Lane=\"" << showLane2Lane << "\"/>\n";

    dev << "        <additionals addMode=\"" << addMode
    << "\" minAddSize=\"" << minAddSize
    << "\" addExaggeration=\"" << addExaggeration
    << "\" drawAddName=\"" << drawAddName
    << "\" addNameSize=\"" << addNameSize << "\"/>\n";

    dev << "        <pois poiExaggeration=\"" << poiExaggeration
    << "\" minPOISize=\"" << minPOISize
    << "\" drawPOIName=\"" << drawPOIName
    << "\" poiNameSize=\"" << poiNameSize
    << "\" poiNameColor=\"" << poiNameColor << "\"/>\n";

    dev << "        <legend showSizeLegend=\"" << showSizeLegend << "\"/>\n";
    dev << "    </scheme>\n";
    dev << "</viewsettings>\n";
}


bool
GUIVisualizationSettings::operator==(const GUIVisualizationSettings &v2) {
    if (antialiase!=v2.antialiase) return false;
    if (dither!=v2.dither) return false;
    if (backgroundColor!=v2.backgroundColor) return false;
    if (showBackgroundDecals!=v2.showBackgroundDecals) return false;
    if (showGrid!=v2.showGrid) return false;
    if (gridXSize!=v2.gridXSize) return false;
    if (gridYSize!=v2.gridYSize) return false;

#ifdef HAVE_MESOSIM
    if (!(edgeColorer==v2.edgeColorer)) return false;
#endif
    if (laneEdgeMode!=v2.laneEdgeMode) return false;
    if (laneColorings!=v2.laneColorings) return false;
    if (laneShowBorders!=v2.laneShowBorders) return false;
    if (showLinkDecals!=v2.showLinkDecals) return false;
    if (showRails!=v2.showRails) return false;
    if (drawEdgeName!=v2.drawEdgeName) return false;
    if (edgeNameSize!=v2.edgeNameSize) return false;
    if (edgeNameColor!=v2.edgeNameColor) return false;
    if (drawInternalEdgeName!=v2.drawInternalEdgeName) return false;
    if (internalEdgeNameSize!=v2.internalEdgeNameSize) return false;
    if (internalEdgeNameColor!=v2.internalEdgeNameColor) return false;
    if (hideConnectors!=v2.hideConnectors) return false;

    if (vehicleMode!=v2.vehicleMode) return false;
    if (vehicleQuality!=v2.vehicleQuality) return false;
    if (minVehicleSize!=v2.minVehicleSize) return false;
    if (vehicleExaggeration!=v2.vehicleExaggeration) return false;
    if (vehicleColorings!=v2.vehicleColorings) return false;
    if (showBlinker!=v2.showBlinker) return false;
    if (drawcC2CRadius!=v2.drawcC2CRadius) return false;
    if (drawLaneChangePreference!=v2.drawLaneChangePreference) return false;
    if (drawVehicleName!=v2.drawVehicleName) return false;
    if (vehicleNameSize!=v2.vehicleNameSize) return false;
    if (vehicleNameColor!=v2.vehicleNameColor) return false;

    if (junctionMode!=v2.junctionMode) return false;
    if (drawLinkTLIndex!=v2.drawLinkTLIndex) return false;
    if (drawLinkJunctionIndex!=v2.drawLinkJunctionIndex) return false;
    if (drawJunctionName!=v2.drawJunctionName) return false;
    if (junctionNameSize!=v2.junctionNameSize) return false;
    if (junctionNameColor!=v2.junctionNameColor) return false;

    if (showLane2Lane!=v2.showLane2Lane) return false;

    if (addMode!=v2.addMode) return false;
    if (minAddSize!=v2.minAddSize) return false;
    if (addExaggeration!=v2.addExaggeration) return false;
    if (drawAddName!=v2.drawAddName) return false;
    if (addNameSize!=v2.addNameSize) return false;

    if (minPOISize!=v2.minPOISize) return false;
    if (poiExaggeration!=v2.poiExaggeration) return false;
    if (drawPOIName!=v2.drawPOIName) return false;
    if (poiNameSize!=v2.poiNameSize) return false;
    if (poiNameColor!=v2.poiNameColor) return false;

    if (showSizeLegend!=v2.showSizeLegend) return false;

    return true;
}



/****************************************************************************/

