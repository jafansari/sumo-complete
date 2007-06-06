/****************************************************************************/
/// @file    GUISourceLane.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 25 Nov 2002
/// @version $Id$
///
// A MSLane extended by some values needed by the gui
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
#ifndef GUISourceLane_h
#define GUISourceLane_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <fx.h>
#include <string>
#include <utility>
#include <microsim/MSSourceLane.h>
#include <microsim/MSEdge.h>
#include <utils/geom/Position2D.h>
#include <utils/geom/Position2DVector.h>
#include <utils/foxtools/MFXMutex.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSNet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUISourceLane
 * An extended MSSourceLane. A mechanism to avoid concurrent
 * visualisation and simulation what may cause problems when vehicles
 * disappear is implemented using a mutex
 */
class GUISourceLane :
            public MSSourceLane
{
public:
    /// constructor
    GUISourceLane(/*MSNet &net, */std::string id, SUMOReal maxSpeed,
                                  SUMOReal length, MSEdge* edge, size_t numericalID,
                                  const Position2DVector &shape,
                                  const std::vector<SUMOVehicleClass> &allowed,
                                  const std::vector<SUMOVehicleClass> &disallowed);

    /// destructor
    ~GUISourceLane();

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    void moveNonCritical(/*
                                const MSEdge::LaneCont::const_iterator &firstNeighLane,
                                const MSEdge::LaneCont::const_iterator &lastNeighLane */);

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    void moveCritical(/*
                                const MSEdge::LaneCont::const_iterator &firstNeighLane,
                                const MSEdge::LaneCont::const_iterator &lastNeighLane */);

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    void setCritical();

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    bool emit(MSVehicle& newVeh);

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    bool isEmissionSuccess(MSVehicle* aVehicle, const MSVehicle::State &vstate);

    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    void integrateNewVehicle();

    /// allows the processing of vehicles for threads
    void releaseVehicles();

    /// returns the vehicles closing their processing for other threads
    const VehCont &getVehiclesSecure();

    GUILaneWrapper *buildLaneWrapper(GUIGlObjectStorage &idStorage);
    SUMOReal getDensity() const;
    SUMOReal getVehLenSum() const;

    void detectCollisions(SUMOTime timestep);

protected:
    /** the same as in MSLane, but locks the access for the visualisation
        first; the access will be granted at the end of this method */
    bool push(MSVehicle* veh);

    MSVehicle* pop();

    /// moves myTmpVehicles int myVehicles after a lane change procedure
    void swapAfterLaneChange();

private:
    /// The mutex used to avoid concurrent updates of the vehicle buffer
    mutable MFXMutex _lock;

};


#endif

/****************************************************************************/

