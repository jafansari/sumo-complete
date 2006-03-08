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
// Revision 1.24  2006/03/08 13:12:29  dkrajzew
// real density visualization added (slow, unfinished)
//
// Revision 1.23  2006/01/31 10:56:14  dkrajzew
// new visualization scheme for lanes added
//
// Revision 1.22  2006/01/09 11:53:43  dkrajzew
// further visualization possibilities added
//
// Revision 1.21  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.20  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.19  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.18  2005/07/12 12:18:09  dkrajzew
// further visualisation options added
//
// Revision 1.17  2005/05/04 08:00:34  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; possibility to select lanes around a lane added
//
// Revision 1.16  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.15  2004/07/02 08:54:11  dkrajzew
// some design issues
//
// Revision 1.14  2004/03/19 12:57:54  dkrajzew
// porting to FOX
//
// Revision 1.13  2003/11/12 14:01:54  dkrajzew
// visualisation of tl-logics added
//
// Revision 1.12  2003/11/11 08:11:05  dkrajzew
// logging (value passing) moved from utils to microsim
//
// Revision 1.11  2003/09/05 14:59:54  dkrajzew
// first tries for an implementation of aggregated views
//
// Revision 1.10  2003/08/21 12:50:49  dkrajzew
// retrival of a links direction added
//
// Revision 1.9  2003/07/30 08:54:14  dkrajzew
// the network is capable to display the networks state, now
//
// Revision 1.8  2003/07/22 14:59:27  dkrajzew
// changes due to new detector handling
//
// Revision 1.7  2003/07/07 08:14:48  dkrajzew
// first steps towards the usage of a real lane and junction geometry implemented
//
// Revision 1.6  2003/06/05 06:29:50  dkrajzew
// first tries to build under linux: warnings removed; moc-files included Makefiles added
//
// Revision 1.5  2003/05/20 09:26:57  dkrajzew
// data retrieval for new views added
//
// Revision 1.4  2003/04/14 08:27:17  dkrajzew
// new globject concept implemented
//
// Revision 1.3  2003/03/20 16:19:28  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.2  2003/03/12 16:52:06  dkrajzew
// centering of objects debuggt
//
// Revision 1.1  2003/02/07 10:39:17  dkrajzew
// updated
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

#include <utils/common/DoubleVector.h>
#include <string>
#include <utility>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/output/e2_detectors/MSE2Collector.h>
#include <microsim/MSLink.h>
#include <utils/geom/Position2D.h>
#include <utils/foxtools/FXMutex.h>
#include <microsim/logging/LoggedValue_TimeFloating.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/drawer/GUILaneRepresentation.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSVehicle;
class MSNet;
class GUINet;
class Position2DVector;
class GUIGLObjectPopupMenu;
class MSEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUILaneWrapper
 * An extended MSLane. The extensions consist of information about the position
 * of the lane and it's direction. Further, a mechanism to avoid concurrent
 * visualisation and simulation what may cause problems when vehicles
 * disappear is implemented using a mutex
 */
class GUILaneWrapper : public GUILaneRepresentation {
public:
    /// constructor
    GUILaneWrapper( GUIGlObjectStorage &idStorage,
        MSLane &lane, const Position2DVector &shape);

    /// destructor
    virtual ~GUILaneWrapper();

    /// Returns a popup-menu for lanes
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    GUIParameterTableWindow *getParameterWindow(
        GUIMainWindow &app, GUISUMOAbstractView &parent);

    /// returns the id of the object as known to microsim
    std::string microsimID() const;

    /** returns the length of the lane */
    SUMOReal getLength() const;

	//{
    /** returns the begin position of the lane */
    const Position2D &getBegin() const;

    /** returns the end position of the lane */
    const Position2D &getEnd() const;

    /** returns the direction of the lane (!!! describe better) */
    const Position2D &getDirection() const;

    /** returns the rotation of the lane (!!! describe better) */
    SUMOReal getRotation() const;

    /** returns the "visualisation length"; this length may differ to the
        real length */
    SUMOReal visLength() const;

    const Position2DVector &getShape() const;
    const DoubleVector &getShapeRotations() const;
    const DoubleVector &getShapeLengths() const;
	//}

    /** returns the purpose (source, sink, normal) of the parent edge */
    MSEdge::EdgeBasicFunction getPurpose() const;

    /** returns the lane's maximum speed */
    SUMOReal maxSpeed() const;

    SUMOReal firstWaitingTime() const;
    SUMOReal getDensity() const;

    /// Returns the fastest known lane speed
    static SUMOReal getOverallMaxSpeed();

    /// Returns true if the given lane id the lane this wrapper wraps the geometry of
    bool forLane(const MSLane &lane) const;

    /// Returns the type of the object as coded in GUIGlObjectType
    GUIGlObjectType getType() const;


    const MSLane::VehCont &getVehiclesSecure();
    void releaseVehicles();

    size_t getLinkNumber() const;

    /// Returns the state of the numbered link
    MSLink::LinkState getLinkState(size_t pos) const;

    /// Returns the direction of the numbered link
    MSLink::LinkDirection getLinkDirection(size_t pos) const;

    MSLane *getLinkLane(size_t pos) const;

    /// Returns the tl-logic the numbered link is controlled by
    unsigned int getLinkTLID(const GUINet &net, size_t pos) const;

    SUMOReal getAggregatedNormed(E2::DetType what,
        size_t aggregationPosition) const;

    SUMOReal getAggregatedFloat(E2::DetType what) const;

    const MSEdge& getMSEdge() const;

    SUMOReal getEdgeLaneNumber() const;

	//{
	Boundary getCenteringBoundary() const;
	//}

    void selectSucessors();

protected:
    bool active() const { return true; }

private:
    void buildAggregatedValuesStorage();

protected:
    /// the begin position of the lane
    Position2D _begin;

    /// the end position of the lane
    Position2D _end;

    /// the direction of the lane
    Position2D _direction;

    /// the direction of the lane
    SUMOReal _rotation;

    /** the visualisation length; As sources and sinks may be shorter/longer
        as their visualisation length, a second length information is necessary */
    SUMOReal _visLength;

    /// The assigned lane
    MSLane &myLane;

    /// The shape of the lane
    const Position2DVector &myShape;

    /// The rotations of the shape parts
    DoubleVector myShapeRotations;

    /// The lengths of the shape parts
    DoubleVector myShapeLengths;

    /// The maximum velocity over all lanes
    static SUMOReal myAllMaxSpeed;

    LoggedValue_TimeFloating<SUMOReal> **myAggregatedValues;

    SUMOReal myAggregatedFloats[3];

    static size_t myAggregationSizes[];

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

