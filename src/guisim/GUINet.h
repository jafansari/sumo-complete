/****************************************************************************/
/// @file    GUINet.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A MSNet extended by some values for usage within the gui
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
#include <utils/gui/windows/GUIGrid.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdgeControl;
class MSJunctionControl;
class MSEmitControl;
class MSEventControl;
class MSRouteLoaderControl;
class MSTLLogicControl;
class MSTrafficLightLogic;
class MSLink;
class GUIJunctionWrapper;
class GUIDetectorWrapper;
class GUITriggerWrapper;
class GUINetWrapper;
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
 * these retrival, but also some further initialisation methods must have
 * been implemented. Nonethenless, this class has almost the same functions
 * as the MSNet-class.
 *
 * Some microsimulation items are wrapped in certain classes to allow their
 * drawing and their enumerated access. This enumeration is realised by
 * inserting the wrapped items into vectors and is needed to fasten the
 * network's drawing as only visible items are being drawn.
 */
class GUINet
            : public MSNet
{
public:
    GUINet(SUMOTime startTimestep, SUMOTime stopTimestep,
           MSVehicleControl *vc, SUMOReal tooSlowRTF, bool logExecTime);

    /// destructor
    ~GUINet();

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

    //{@ functions for performace measurements
    /// Returns the duration of the last step (sim+visualisation+idle) (in ms)
    int getWholeDuration() const;

    /// Returns the duration of the last step's simulation part (in ms)
    int getSimDuration() const;

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
    int getIdleDuration() const;

    /// Sets the duration of the last step's simulation part
    void setSimDuration(int val);

    // Sets the duration of the last step's visualisation part
    //void setVisDuration(int val);

    /// Sets the duration of the last step's idle part
    void setIdleDuration(int val);
    //}


    /// Returns the gui network wrapper
    GUINetWrapper * const getWrapper() const;

    /** Returns the gl-id of the traffic light that controls the given link
     * valid only if the link is controlled by a tls */
    unsigned int getLinkTLID(MSLink *link) const;

    /** Returns the index of the link within the junction that controls the given link;
     * Returns -1 if the link is not controlled by a tls */
    int getLinkTLIndex(MSLink *link) const;


    /// @name locator-methods
    //@{ 
    /// Returns the gl-ids of all junctions within the net
    std::vector<size_t> getJunctionIDs() const;

    /// Returns the gl-ids of all traffic light logics within the net
    std::vector<size_t> getTLSIDs() const;

    /// Returns the gl-ids of all additional things within the net
    std::vector<size_t> getAdditionalIDs() const;

    /// Returns the gl-ids of all shapes within the net
    std::vector<size_t> getShapeIDs() const;
    //@}


    /// Initialises gui wrappers
    void initGUIStructures();

    /// Builds an appropriate route loader for this type of a net
    MSRouteLoader *buildRouteLoader(const std::string &file, int incDUABase, int incDUAStage);


    friend class GUIViewTraffic; // !!!
    friend class GUISUMOAbstractView; // !!!
    friend class GUIViewMesoEdges; // !!!
    friend class GUIGridBuilder;


private:
    /// Initialises the detector wrappers
    void initDetectors();

    /// Initialises the tl-logic map and wrappers
    void initTLMap();

protected:
    /** A grid laid over the network to allow the drawing of visible items only */
    GUIGrid _grid;

    /// the networks boundary
    Boundary _boundary;

    /// The wrapper for the network
    GUINetWrapper *myWrapper;

    /// Wrapped MS-edges
    std::vector<GUIEdge*> myEdgeWrapper;

    /// Wrapped MS-junctions
    std::vector<GUIJunctionWrapper*> myJunctionWrapper;

    /// Wrapped TL-Logics
    std::vector<MSTrafficLightLogic*> myTLLogicWrappers;

    /// A detector dictionary
    std::map<std::string, GUIDetectorWrapper*> myDetectorDict;


    /// Definition of a link-to-logic-id map
    typedef std::map<MSLink*, std::string> Links2LogicMap;
    /// The link-to-logic-id map
    Links2LogicMap myLinks2Logic;


    /// Definition of a traffic light-to-wrapper map
    typedef std::map<MSTrafficLightLogic*, GUITrafficLightLogicWrapper*> Logics2WrapperMap;
    /// The traffic light-to-wrapper map
    Logics2WrapperMap myLogics2Wrapper;


    /// The step durations (simulation, /*visualisation, */idle)
    int myLastSimDuration, /*myLastVisDuration, */myLastIdleDuration;

    long myLastVehicleMovementCount, myOverallVehicleCount;
    long myOverallSimDuration;

};


#endif

/****************************************************************************/

