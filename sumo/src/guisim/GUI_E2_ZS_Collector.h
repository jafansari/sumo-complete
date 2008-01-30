/****************************************************************************/
/// @file    GUI_E2_ZS_Collector.h
/// @author  Daniel Krajzewicz
/// @date    Okt 2003
/// @version $Id$
///
// The gui-version of the MS_E2_ZS_Collector, together with the according
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
#ifndef GUI_E2_ZS_Collector_h
#define GUI_E2_ZS_Collector_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/common/PhysicalTypeDefs.h>
#include <microsim/output/MSE2Collector.h>
#include <microsim/MSNet.h>
#include <utils/geom/Position2D.h>
#include <utils/geom/Position2DVector.h>
#include <utils/common/VectorHelper.h>
#include <utils/common/ValueSource.h>
#include "GUIDetectorWrapper.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GUIGlObjectStorage;
class GUILaneWrapper;
class GUI_E2_ZS_CollectorOverLanes;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUI_E2_ZS_Collector
 * The gui-version of the MS_E2_ZS_Collector.
 * Allows the building of a wrapper (also declared herein) which draws the
 * detector on the gl-canvas. Beside this, the method "amVisible" is
 * overridden to signalise that this detector is not used for simulation-
 * -internal reasons, but is placed over the simulation by the user.
 */
class GUI_E2_ZS_Collector : public MSE2Collector
{
public:
    /// Constructor
    GUI_E2_ZS_Collector(const std::string &id, DetectorUsage usage,
                        MSLane* lane, SUMOReal startPos, SUMOReal detLength,
                        SUMOTime haltingTimeThreshold, SUMOReal haltingSpeedThreshold,
                        SUMOReal jamDistThreshold);

    /// Destructor
    ~GUI_E2_ZS_Collector() throw ();

    // valid for gui-version only
    virtual GUIDetectorWrapper *buildDetectorWrapper(
        GUIGlObjectStorage &idStorage,
        GUILaneWrapper &wrapper);

    // valid for gui-version and joined collectors only
    virtual GUIDetectorWrapper *buildDetectorWrapper(
        GUIGlObjectStorage &idStorage,
        GUILaneWrapper &wrapper, GUI_E2_ZS_CollectorOverLanes& p,
        size_t glID);

public:
    /**
     * @class GUI_E2_ZS_Collector::MyWrapper
     * A GUI_E2_ZS_Collector-visualiser
     */
class MyWrapper : public GUIDetectorWrapper
    {
    public:
        /// Constructor
        MyWrapper(GUI_E2_ZS_Collector &detector,
                  GUIGlObjectStorage &idStorage, GUILaneWrapper &wrapper);

        /// Constructor for collectors joined over lanes
        MyWrapper(GUI_E2_ZS_Collector &detector,
                  GUIGlObjectStorage &idStorage, size_t glID,
                  GUI_E2_ZS_CollectorOverLanes &mustBe,
                  GUILaneWrapper &wrapper);

        /// Destrutor
        ~MyWrapper();

        /// Returns the boundary of the wrapped detector
        Boundary getBoundary() const;

        /// Draws the detector in full-geometry mode
        void drawGL(SUMOReal scale, SUMOReal upscale);

        /// Draws the detector in full-geometry mode
        GUIParameterTableWindow *getParameterWindow(
            GUIMainWindow &app, GUISUMOAbstractView &parent);

        /// returns the id of the object as known to microsim
        const std::string &microsimID() const;

        /// Returns the information whether this detector is still active
        bool active() const;

        /// Returns the wrapped detector's coordinates
        Position2D getPosition() const;

        /// Returns the detector itself
        GUI_E2_ZS_Collector &getDetector();


    private:
        void myConstruct(GUI_E2_ZS_Collector &detector,
                         GUILaneWrapper &wrapper);

    private:
        /// The wrapped detector
        GUI_E2_ZS_Collector &myDetector;

        /// The detector's boundary
        Boundary myBoundary;

        /// A sequence of positions in full-geometry mode
        Position2DVector myFullGeometry;

        /// A sequence of lengths in full-geometry mode
        DoubleVector myShapeLengths;

        /// A sequence of rotations in full-geometry mode
        DoubleVector myShapeRotations;

    private:
        /// @brief Invalidated copy constructor.
        MyWrapper(const MyWrapper&);

        /// @brief Invalidated assignment operator.
        MyWrapper& operator=(const MyWrapper&);

    };

};


#endif

/****************************************************************************/

