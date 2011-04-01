/****************************************************************************/
/// @file    GUINet.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A MSNet extended by some values for usage within the gui
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
#ifndef GUINet_h
#define GUINet_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <utility>
#include <microsim/MSNet.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position2D.h>
#include <foreign/rtree/SUMORTree.h>
#include <utils/geom/Position2DVector.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdgeControl;
class MSJunctionControl;
class MSRouteLoaderControl;
class MSTLLogicControl;
class MSTrafficLightLogic;
class MSLink;
class GUIJunctionWrapper;
class GUIDetectorWrapper;
class GUITrafficLightLogicWrapper;
class RGBColor;
class GUIEdge;
class OutputDevice;
class GUIVehicle;
class MSVehicleControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUINet
 * @brief A MSNet extended by some values for usage within the gui
 *
 * This gui version of the network allows the retrieval of some more
 * information than the normal network version does. Due to this, not only
 * these retrieval, but also some further initialisation methods must have
 * been implemented. Nonethenless, this class has almost the same functions
 * as the MSNet-class.
 *
 * Some microsimulation items are wrapped in certain classes to allow their
 * drawing and their enumerated access. This enumeration is realised by
 * inserting the wrapped items into vectors and is needed to fasten the
 * network's drawing as only visible items are being drawn.
 */
class GUINet : public MSNet, public GUIGlObject {
public:
    /** @brief Constructor
     * @param[in] vc The vehicle control to use
     * @param[in] beginOfTimestepEvents The event control to use for simulation step begin events
     * @param[in] endOfTimestepEvents The event control to use for simulation step end events
     * @param[in] insertionEvents The event control to use for insertion events
     * @exception ProcessError If a network was already constructed
     */
    GUINet(MSVehicleControl *vc, MSEventControl *beginOfTimestepEvents,
           MSEventControl *endOfTimestepEvents, MSEventControl *insertionEvents) throw(ProcessError);


    /// @brief Destructor
    ~GUINet() throw();



    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
                                       GUISUMOAbstractView &parent) throw();


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow *getParameterWindow(
        GUIMainWindow &app, GUISUMOAbstractView &parent) throw();


    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const throw();


    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings &s) const throw();
    //@}


    /// returns the bounder of the network
    const Boundary &getBoundary() const;

    /// returns the position of a junction
    Position2D getJunctionPosition(const std::string &name) const;

    /// returns the information whether the vehicle still exists
    bool vehicleExists(const std::string &name) const;

    /// returns the boundary of an edge
    Boundary getEdgeBoundary(const std::string &name) const;

    /// Some further steps needed for gui processing
    void guiSimulationStep();


    /// @name functions for performance measurements
    /// @{

    /** @brief Returns the duration of the last step (sim+visualisation+idle) (in ms)
     * @return How long it took to compute and display the last step
     */
    unsigned int getWholeDuration() const throw();


    /** @brief Returns the duration of the last step's simulation part (in ms)
     * @return How long it took to compute the last step
     */
    unsigned int getSimDuration() const throw();


    /// Returns the simulation speed as a factor to real time
    SUMOReal getRTFactor() const;

    /// Returns the update per seconds rate
    SUMOReal getUPS() const;

    /// Returns the simulation speed as a factor to real time
    SUMOReal getMeanRTFactor(int duration) const;

    /// Returns the update per seconds rate
    SUMOReal getMeanUPS() const;

    // Returns the duration of the last step's visualisation part (in ms)
    //int getVisDuration() const;

    /// Returns the duration of the last step's idle part (in ms)
    unsigned int getIdleDuration() const throw();

    /// Sets the duration of the last step's simulation part
    void setSimDuration(int val);

    // Sets the duration of the last step's visualisation part
    //void setVisDuration(int val);

    /// Sets the duration of the last step's idle part
    void setIdleDuration(int val);
    //}


    /** Returns the gl-id of the traffic light that controls the given link
     * valid only if the link is controlled by a tls */
    unsigned int getLinkTLID(MSLink *link) const;

    /** Returns the index of the link within the junction that controls the given link;
     * Returns -1 if the link is not controlled by a tls */
    int getLinkTLIndex(MSLink *link) const;


    /// @name locator-methods
    //@{

    /// Returns the gl-ids of all junctions within the net
    std::vector<GLuint> getJunctionIDs() const;

    /// Returns the gl-ids of all traffic light logics within the net
    std::vector<GLuint> getTLSIDs() const;

    /// Returns the gl-ids of all shapes within the net
    std::vector<GLuint> getShapeIDs() const;
    //@}


    /// Initialises gui wrappers
    void initGUIStructures();


    /** @brief Returns the RTree used for visualisation speed-up
     * @return The visualisation speed-up
     */
    SUMORTree &getVisualisationSpeedUp() throw() {
        return myGrid;
    }


    /** @brief Returns the RTree used for visualisation speed-up
     * @return The visualisation speed-up
     */
    const SUMORTree &getVisualisationSpeedUp() const throw() {
        return myGrid;
    }


private:
    /// @brief Initialises the detector wrappers
    void initDetectors();

    /// @brief Initialises the tl-logic map and wrappers
    void initTLMap();


protected:
    /// @brief The visualization speed-up
    SUMORTree myGrid;

    /// @brief The networks boundary
    Boundary myBoundary;

    /// @brief Wrapped MS-edges
    std::vector<GUIEdge*> myEdgeWrapper;

    /// @brief Wrapped MS-junctions
    std::vector<GUIJunctionWrapper*> myJunctionWrapper;

    /// @brief Wrapped TL-Logics
    std::vector<MSTrafficLightLogic*> myTLLogicWrappers;

    /// @brief A detector dictionary
    std::map<std::string, GUIDetectorWrapper*> myDetectorDict;


    /// @brief Definition of a link-to-logic-id map
    typedef std::map<MSLink*, std::string> Links2LogicMap;
    /// @brief The link-to-logic-id map
    Links2LogicMap myLinks2Logic;


    /// @brief Definition of a traffic light-to-wrapper map
    typedef std::map<MSTrafficLightLogic*, GUITrafficLightLogicWrapper*> Logics2WrapperMap;
    /// @brief The traffic light-to-wrapper map
    Logics2WrapperMap myLogics2Wrapper;


    /// @brief The step durations (simulation, /*visualisation, */idle)
    unsigned int myLastSimDuration, /*myLastVisDuration, */myLastIdleDuration;

    long myLastVehicleMovementCount, myOverallVehicleCount;
    long myOverallSimDuration;

};


#endif

/****************************************************************************/

