#ifndef GUILaneWrapper_h
#define GUILaneWrapper_h
//---------------------------------------------------------------------------//
//                        GUILaneWrapper.h -
//  Holds geometrical values for a lane
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 25 Nov 2002
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
// Revision 1.1  2003/02/07 10:39:17  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <string>
#include <utility>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <utils/geom/Position2D.h>
#include <utils/qutils/NewQMutex.h>
#include <gui/GUIGlObject.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSVehicle;
class MSNet;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * An extended MSLane. The extensions consist of information about the position
 * of the lane and it's direction. Further, a mechanism to avoid concurrent
 * visualisation and simulation what may cause problems when vehicles
 * disappear is implemented using a mutex
 */
class GUILaneWrapper :
            public GUIGlObject {
public:
    /// constructor
    GUILaneWrapper( MSLane &lane, double x1, double y1, double x2, double y2);

    /// destructor
    virtual ~GUILaneWrapper();

    /** returns the length of the lane */
    double getLength() const;

    /** returns the begin position of the lane */
    const Position2D &getBegin() const;

    /** returns the end position of the lane */
    const Position2D &getEnd() const;

    /** returns the direction of the lane (!!! describe better) */
    const Position2D &getDirection() const;

    /** returns the rotation of the lane (!!! describe better) */
    double getRotation() const;

    /** returns the "visualisation length"; this length may differ to the
        real length */
    double visLength() const;

    /** returns the purpose (source, sink, normal) of the parent edge */
    MSEdge::EdgeBasicFunction getPurpose() const;

    /** returns the lane's maximum speed */
    double maxSpeed() const;

    /// Returns the fastest known lane speed
    static double getOverallMaxSpeed();

protected:
    /// the begin position of the lane
    Position2D _begin;

    /// the end position of the lane
    Position2D _end;

    /// the direction of the lane
    Position2D _direction;

    /// the direction of the lane
    double _rotation;

    /** the visualisation length; As sources and sinks may be shorter/longer
        as their visualisation length, a second length information is necessary */
    double _visLength;

    /// The assigned lane
    MSLane &myLane;

    /// The maximum velocity over all lanes
    static double myAllMaxSpeed;

};

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifndef DISABLE_INLINE
//#include "GUILaneWrapper.icc"
//#endif

#endif

// Local Variables:
// mode:C++
// End:

