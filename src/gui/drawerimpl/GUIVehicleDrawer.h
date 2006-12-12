#ifndef GUIVehicleDrawer_h
#define GUIVehicleDrawer_h
//---------------------------------------------------------------------------//
//                        GUIVehicleDrawer.h -
//  Base class for vehicle drawing
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 02.09.2003
//  copyright            : (C) 2003 by Daniel Krajzewicz
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
// $Log$
// Revision 1.1  2006/12/12 12:10:43  dkrajzew
// removed simple/full geometry options; everything is now drawn using full geometry
//
// Revision 1.11  2006/07/06 06:26:44  dkrajzew
// added blinker visualisation and vehicle tracking (unfinished)
//
// Revision 1.10  2006/01/31 10:51:18  dkrajzew
// unused methods documented out (kept for further use)
//
// Revision 1.9  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.8  2005/10/07 11:36:48  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/15 11:05:28  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/04/27 09:44:26  dkrajzew
// level3 warnings removed
//
// Revision 1.5  2004/11/23 10:05:21  dkrajzew
// removed some warnings and adapted the new class hierarchy
//
// Revision 1.4  2004/08/02 11:30:54  dkrajzew
// refactored vehicle and lane coloring scheme usage to allow optional coloring schemes
//
// Revision 1.3  2004/03/19 12:34:30  dkrajzew
// porting to FOX
//
// Revision 1.2  2003/09/17 06:45:11  dkrajzew
// some documentation added/patched
//
// Revision 1.1  2003/09/05 14:50:39  dkrajzew
// implementations of artefact drawers moved to folder "drawerimpl"
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

#include <vector>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/drawer/GUIColoringSchemesMap.h>
#include <utils/glutils/GLHelper.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUILaneWrapper;
class GUIVehicle;
class GUIEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * Draws vehicles as coloured triangles
 */
class GUIVehicleDrawer {
public:
    /// constructor
    GUIVehicleDrawer(const std::vector<GUIEdge*> &edges);

    /// destructor
    virtual ~GUIVehicleDrawer();

    /// Draws the vehicles that are on the marked edges
    void drawGLVehicles(size_t *onWhich, size_t maxEdges,
        GUISUMOAbstractView::VisualizationSettings &settings/*,
        GUIBaseColorer<GUIVehicle> &colorer, float upscale*/);

    void setGLID(bool val);

    /// Returns the list of available coloring schemes
    static GUIColoringSchemesMap<GUIVehicle> &getSchemesMap();

protected:
    /// initialises the drawing
    void initStep();

    /// Draws all vehicles that are on the given lane
    virtual void drawLanesVehicles(GUILaneWrapper &lane,
        const GUISUMOAbstractView::VisualizationSettings &settings);

protected:
    /// The list of edges to consider at drawing
    const std::vector<GUIEdge*> &myEdges;

    /** @brief The list of coloring schemes that may be used
        They are not fixed as they may change in dependence to the available parameter */
    static GUIColoringSchemesMap<GUIVehicle> myColoringSchemes;

    /// Information whether the gl-id shall be set
    bool myShowToolTips;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

