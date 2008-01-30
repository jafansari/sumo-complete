/****************************************************************************/
/// @file    GUIE3Collector.h
/// @author  Daniel Krajzewicz
/// @date    Jan 2004
/// @version $Id$
///
// The gui-version of a MSE3Collector
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
#ifndef GUIE3Collector_h
#define GUIE3Collector_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <vector>
#include <utils/common/PhysicalTypeDefs.h>
#include <microsim/output/e3_detectors/MSE3Collector.h>
#include "GUIDetectorWrapper.h"
#include <utils/geom/Position2DVector.h>
#include <utils/common/ValueSource.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIE3Collector
 * @brief The gui-version of the MSE3Collector.
 *
 * Allows the building of a wrapper (also declared herein) which draws the
 * detector on the gl-canvas.
 */
class GUIE3Collector : public MSE3Collector
{
public:
    /// Constructor
    GUIE3Collector(const std::string &id,
                   const CrossSectionVector &entries, const CrossSectionVector &exits,
                   MetersPerSecond haltingSpeedThreshold);

    /// Destructor
    ~GUIE3Collector() throw ();

    /// Returns the list of entry points
    const CrossSectionVector &getEntries() const;

    /// Returns the list of exit points
    const CrossSectionVector &getExits() const;

    /// Builds the wrapper
    GUIDetectorWrapper *buildDetectorWrapper(GUIGlObjectStorage &idStorage);

public:
    /**
     * @class GUIE3Collector::MyWrapper
     * A GUIE3Collector-visualiser
     */
class MyWrapper : public GUIDetectorWrapper
    {
    public:
        /// Constructor
        MyWrapper(GUIE3Collector &detector,
                  GUIGlObjectStorage &idStorage);

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
        GUIE3Collector &getDetector();

    protected:
        struct SingleCrossingDefinition {
            /// The position in full-geometry mode
            Position2D myFGPosition;
            /// The rotation in full-geometry mode
            SUMOReal myFGRotation;
        };

    protected:
        /// Builds the description about the position of the entry/exit point
        SingleCrossingDefinition buildDefinition(const MSCrossSection &section);

        /// Draws a single entry/exit point
        void drawSingleCrossing(const Position2D &pos, SUMOReal rot,
                                SUMOReal upscale) const;

    private:
        /// The wrapped detector
        GUIE3Collector &myDetector;

        /// The detector's boundary
        Boundary myBoundary;

        /// Definition of a list of cross (entry/exit-point) positions
        typedef std::vector<SingleCrossingDefinition> CrossingDefinitions;

        /// The list of entry positions
        CrossingDefinitions myEntryDefinitions;

        /// The list of exit positions
        CrossingDefinitions myExitDefinitions;

    };

};


#endif

/****************************************************************************/

