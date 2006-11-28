#ifndef GUISUMOAbstractView_h
#define GUISUMOAbstractView_h
//---------------------------------------------------------------------------//
//                        GUISUMOAbstractView.h -
//  The base class for a view
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
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
// Revision 1.21  2006/11/28 12:10:45  dkrajzew
// got rid of FXEX-Mutex (now using the one supplied in FOX)
//
// Revision 1.20  2006/11/16 10:50:53  dkrajzew
// warnings removed
//
// Revision 1.19  2006/09/18 10:18:23  dkrajzew
// debugging
//
// Revision 1.18  2006/08/01 05:43:46  dkrajzew
// cartesian and geocoordinates are shown; changed the APIs for this
//
// Revision 1.17  2006/07/06 05:55:59  dkrajzew
// added the "show blinker" option
//
// Revision 1.16  2006/04/18 08:08:21  dkrajzew
// added Danilot Tete-Boyoms poi-interaction
//
// Revision 1.15  2006/03/27 07:34:19  dkrajzew
// shape layers added
//
// Revision 1.14  2006/03/08 13:17:56  dkrajzew
// possibility to insert pois on the gui added (danilot tete-boyom)
//
// Revision 1.13  2006/02/23 11:37:55  dkrajzew
// adding pois on the gui implemented (Danilot Tete Boyom)
//
// Revision 1.12  2006/01/31 11:03:01  dkrajzew
// added the possibility to exaggerate pois; debugging the grid
//
// Revision 1.11  2006/01/19 09:27:12  dkrajzew
// cursor position output finished
//
// Revision 1.10  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.9  2005/11/09 06:46:34  dkrajzew
// added cursor position output (unfinished)
//
// Revision 1.8  2005/10/07 11:46:08  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/23 06:11:14  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.6  2005/09/15 12:20:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/07/12 12:52:07  dkrajzew
// code style adapted
//
// Revision 1.4  2005/05/04 09:24:43  dkrajzew
// entries for viewport definition added; popups now popup faster
//
// Revision 1.3  2004/12/15 09:20:19  dkrajzew
// made guisim independent of giant/netedit
//
// Revision 1.2  2004/12/12 17:23:59  agaubatz
// Editor Tool Widgets included
//
// Revision 1.1  2004/11/23 10:38:32  dkrajzew
// debugging
//
// Revision 1.3  2004/11/22 12:56:02  dksumo
// coloring of lanes by loaded weights added
//
// Revision 1.2  2004/10/29 06:01:56  dksumo
// renamed boundery to boundary
//
// Revision 1.1  2004/10/22 12:50:57  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.15  2004/08/02 11:54:52  dkrajzew
// added the possibility to take snapshots
//
// Revision 1.14  2004/07/02 08:32:10  dkrajzew
// changes due to the global object selection applied; some debugging (on zoom)
//
// Revision 1.13  2004/06/17 13:06:55  dkrajzew
// Polygon visualisation added
//
// Revision 1.12  2004/04/02 11:11:24  dkrajzew
// visualisation whether an item is selected added
//
// Revision 1.11  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.10  2004/01/26 06:47:11  dkrajzew
// Added the possibility to draw arrows by a detector drawer;
//  documentation added
//
// Revision 1.9  2003/11/12 14:07:46  dkrajzew
// clean up after recent changes
//
// Revision 1.8  2003/09/23 14:25:13  dkrajzew
// possibility to visualise detectors using different geometry complexities
//  added
//
// Revision 1.7  2003/09/05 14:54:06  dkrajzew
// implementations of artefact drawers moved to folder "drawerimpl"
//
// Revision 1.6  2003/08/15 12:19:16  dkrajzew
// legend display patched
//
// Revision 1.5  2003/08/14 13:42:43  dkrajzew
// definition of the tls/row - drawer added
//
// Revision 1.4  2003/07/30 08:52:16  dkrajzew
// further work on visualisation of all geometrical objects
//
// Revision 1.3  2003/07/22 14:56:46  dkrajzew
// changes due to new detector handling
//
// Revision 1.2  2003/07/16 15:18:23  dkrajzew
// new interfaces for drawing classes; junction drawer interface added
//
// Revision 1.1  2003/05/20 09:25:13  dkrajzew
// new view hierarchy; some debugging done
//
// Revision 1.6  2003/04/16 09:50:05  dkrajzew
// centering of the network debugged; additional parameter of maximum display
//  size added
//
// Revision 1.4  2003/04/02 11:50:28  dkrajzew
// a working tool tip implemented
//
// Revision 1.3  2003/02/07 10:34:15  dkrajzew
// files updated
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

#include <string>
#include <vector>
#include <fx.h>
#include <fx3d.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position2D.h>
#include <utils/gfx/RGBColor.h>
#include <utils/shapes/Polygon2D.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include "GUIGrid.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#endif


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIGLObjectToolTip;
class MSVehicle;
class GUINet;
class GUIGlChildWindow;
class GUIVehicle;
class GUILaneWrapper;
class GUIPerspectiveChanger;
class GUIMainWindow;
class GUIJunctionWrapper;
class GUIDetectorWrapper;
class GUIGLObjectPopupMenu;
class GUIGlObject;
class GUIGlObjectStorage;
class PointOfInterest;
class ShapeContainer;
class GUIDialog_EditViewport;
class GUIDialog_ViewSettings;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUISUMOAbstractView
 * This class is meant to be pure virtual later;
 * It shall be the main class to inherit views of the simulation (micro-
 * or macroscopic ones) from it.
 */
class GUISUMOAbstractView : public FXGLCanvas {
  FXDECLARE(GUISUMOAbstractView)
public:
    /// constructor
    GUISUMOAbstractView(FXComposite *p, GUIMainWindow &app,
        GUIGlChildWindow *parent, const GUIGrid &grid,
        FXGLVisual *glVis);

    /// constructor
    GUISUMOAbstractView(FXComposite *p, GUIMainWindow &app,
        GUIGlChildWindow *parent, const GUIGrid &grid,
        FXGLVisual *glVis, FXGLCanvas *share);

    /// destructor
    virtual ~GUISUMOAbstractView();

    /// builds the view toolbars
    virtual void buildViewToolBars(GUIGlChildWindow &) { }

    /// recenters the view
    void recenterView();

    /// centers to the chosen artifact
    virtual void centerTo(GUIGlObject *o);

    /// rename the chosen artifact
    virtual void rename(GUIGlObject *o);

    /// move the chosen artifact
    virtual void moveTo(GUIGlObject *o);

    /// change the color of the chosen artifact
    virtual void changeCol(GUIGlObject *o);

    /// change typ of the chosen artifact
    virtual void changeTyp(GUIGlObject *o);

    /// delete the chosen artifact
    virtual void deleteObj(GUIGlObject *o);

    /// applies the given viewport settings
    virtual void setViewport(SUMOReal zoom, SUMOReal xPos, SUMOReal yPos);

    /// meter-to-pixels conversion method
    SUMOReal m2p(SUMOReal meter);

    /// pixels-to-meters conversion method
    SUMOReal p2m(SUMOReal pixel);

    /// Returns the information whether rotation is allowd
    bool allowRotation() const;

    /// Returns the gl-id of the object under the given coordinates
    void setTooltipPosition(size_t x, size_t y, size_t mouseX, size_t mouseY);

    /// A reimplementation due to some internal reasons
    FXbool makeCurrent();

	/// returns true, if the edit button was pressed
	bool isInEditMode();

    //GUINet &getNet() const;


    long onConfigure(FXObject*,FXSelector,void*);
    long onPaint(FXObject*,FXSelector,void*);
    virtual long onLeftBtnPress(FXObject*,FXSelector,void*);
    virtual long onLeftBtnRelease(FXObject*,FXSelector,void*);
    virtual long onRightBtnPress(FXObject*,FXSelector,void*);
    virtual long onRightBtnRelease(FXObject*,FXSelector,void*);
    virtual long onMouseMove(FXObject*,FXSelector,void*);
    virtual long onCmdShowToolTips(FXObject*,FXSelector,void*);
    virtual long onCmdEditViewport(FXObject*,FXSelector,void*);
    virtual long onCmdEditView(FXObject*,FXSelector,void*) = 0;
    long onCmdShowGrid(FXObject*,FXSelector,void*);
    long onSimStep(FXObject*sender,FXSelector,void*);

    long onKeyPress(FXObject *o,FXSelector sel,void *data);
    long onKeyRelease(FXObject *o,FXSelector sel,void *data);


    virtual void openObjectDialog();

    /// A method that updates the tooltip
    void updateToolTip();

    /// Returns the maximum width of gl-windows
    int getMaxGLWidth() const;

    /// Returns the maximum height of gl-windows
    int getMaxGLHeight() const;

    /// paints the area to a buffer
    FXColor *getSnapshot();

    /// Builds the popup-menu containing the location-menu
    virtual FXPopup *getLocatorPopup(GUIGlChildWindow &p);

    void drawShapes(const ShapeContainer &sc, int maxLayer) ;

    void remove(GUIDialog_EditViewport *) { myViewportChooser = 0; }

    void remove(GUIDialog_ViewSettings *) { myVisualizationChanger = 0; }


    SUMOReal getGridWidth() const;
    SUMOReal getGridHeight() const;

public:
    /**
     * JunctionColoringScheme
     * This enumeration holds the possible vehicle colouring schemes
     */
    enum JunctionColoringScheme {
        /// colouring by vehicle speed
        VCS_BY_TYPE = 0
    };

    struct Decal {
        std::string filename;
        SUMOReal left;
        SUMOReal top;
        SUMOReal right;
        SUMOReal bottom;
        SUMOReal rot;
        bool initialised;
        int glID;
    };

public:
    /**
     * @class ViewSettings
     * This class stores the viewport information for an easier checking whether
     *  it has changed.
     */
    class ViewportSettings {
    public:
        /// Constructor
        ViewportSettings();

        /// Parametrised Constructor
        ViewportSettings(SUMOReal xmin, SUMOReal ymin, SUMOReal xmax, SUMOReal ymax);

        /// Destructor
        ~ViewportSettings();

        /// Returns the information whether the stored setting differs from the given
        bool differ(SUMOReal xmin, SUMOReal ymin, SUMOReal xmax, SUMOReal ymax);

        /// Sets the setting information to the given values
        void set(SUMOReal xmin, SUMOReal ymin, SUMOReal xmax, SUMOReal ymax);

    private:
        /// Position and size information to describe the viewport
        SUMOReal myXMin, myYMin, myXMax, myYMax;

    };

    /**
     * @struct VisualizationSettings
     * This class stores the information about how to visualize the structures
     */
    struct VisualizationSettings {
        bool antialiase;
        bool dither;

        RGBColor backgroundColor;
        bool showBackgroundDecals;

        int laneEdgeMode;
        RGBColor singleLaneColor;
        RGBColor minLaneColor;
        RGBColor maxLaneColor;
        bool laneShowBorders;
        std::vector<RGBColor> laneGradient;
        bool showLinkDecals;
        int laneEdgeExaggMode;
        SUMOReal minExagg;
        SUMOReal maxExagg;
        bool showRails;

        int vehicleMode;
        float minVehicleSize;
        float vehicleExaggeration;
        RGBColor singleVehicleColor;
        RGBColor minVehicleColor;
        RGBColor maxVehicleColor;
        std::vector<RGBColor> vehicleGradient;
        bool showBlinker;

        int junctionMode;

        bool showLane2Lane;

        int addMode;
        float minAddSize;
        float addExaggeration;

        float minPOISize;
        float poiExaggeration;

    };

protected:

    /// performs the painting of the simulation
    void paintGL();

    /// Draws the given polygon
    void drawPolygon2D(const Polygon2D &polygon) const;

    /// Draws the given polygon
    void drawPOI2D(const PointOfInterest &p) const;

    void updatePositionInformation() const;

    Position2D getPositionInformation() const;

    Position2D getPositionInformation(int x, int y) const;

    void drawShapesLayer(const ShapeContainer &sc, int layer);


protected:
    virtual void doPaintGL(int /*mode*/, SUMOReal /*scale*/) { }

    virtual void doInit() { }

    /// paints a grid
    void paintGLGrid();

    /** applies the changes arised from window resize or movement */
    void applyChanges(SUMOReal scale, size_t xoff, size_t yoff);

    /// draws the legend
    void displayLegend(bool flip=false);

    /// centers the view to the given position and size
//    void centerTo(Position2D pos, SUMOReal radius);

    /// centers the given boundary
    void centerTo(Boundary bound);

    /// returns the id of the object under the cursor using GL_SELECT
    unsigned int getObjectUnderCursor();

    /// invokes the tooltip for the given object
    void showToolTipFor(unsigned int id);

    /// Clears the usetable, filling it with false
    void clearUsetable(size_t *_edges2Show, size_t _edges2ShowSize);

protected:
    /// The application
    GUIMainWindow *myApp;

    /// the parent window
    GUIGlChildWindow *_parent;

    /// the network used (stored here for a faster access)
    GUIGrid *myGrid;

    /// the sizes of the window
    int _widthInPixels, _heightInPixels;

    /// the scale of the net (the maximum size, either width or height)
    SUMOReal myNetScale;

    /// information whether the grid shall be displayed
    bool _showGrid;

    /// The perspective changer
    GUIPerspectiveChanger *_changer;

    /// Information whether too-tip informations shall be generated
    bool _useToolTips;
	bool _inEditMode;

    /// The used tooltip-class
    GUIGLObjectToolTip *_toolTip;

    /// position to display the tooltip at
    size_t _toolTipX, _toolTipY;

    /// The current mouse position (if the mouse is over this canvas)
    size_t _mouseX, _mouseY;

    /// Offset to the mouse-hotspot from the mouse position
    int _mouseHotspotX, _mouseHotspotY;

    /// _widthInPixels / _heightInPixels
    SUMOReal _ratio;

    /// Additional scaling factor for meters-to-pixels conversion
    SUMOReal _addScl;

    /// The timer id
    int _timer;

    /// The reason (mouse state) of the timer
    int _timerReason;

    /// The current popup-menu
    GUIGLObjectPopupMenu *_popup;

    /// the description of the viewport
    ViewportSettings myViewportSettings;

    VisualizationSettings myVisualizationSettings;

    /// Internal information whether doInit() was called
    bool myAmInitialised;

    /// The locator menu
    FXPopup *myLocatorPopup;


    GUIDialog_EditViewport *myViewportChooser;
    GUIDialog_ViewSettings *myVisualizationChanger;

    std::vector<Decal> myDecals;
    FXMutex myDecalsLock;

	mutable FXMutex myPolyDrawLock;

    enum VehicleOperationType {
        VO_TRACK,
        VO_SHOW_ROUTE
    };

    struct VehicleOps {
        VehicleOperationType type;
        GUIVehicle *vehicle;
        int routeNo;
    };

    /// List of vehicles for which something has to be done with
    std::vector<VehicleOps> myVehicleOps;

    /**
     * A class to find the matching lane wrapper
     */
    class vehicle_in_ops_finder {
    public:
        /** constructor */
        explicit vehicle_in_ops_finder(const GUIVehicle * const v)
            : myVehicle(v) { }

        /** the comparing function */
        bool operator() (const VehicleOps &vo) {
            return vo.vehicle == myVehicle;
        }

    private:
        /// The vehicle to search for
        const GUIVehicle * const myVehicle;

    };

protected:
    GUISUMOAbstractView() { }

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

