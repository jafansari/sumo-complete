#ifndef GUIEdge_h
#define GUIEdge_h
//---------------------------------------------------------------------------//
//                        GUIEdge.h -
//  An MSEdge extended by values needed for the gui
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
// Revision 1.23  2007/01/09 12:05:52  dkrajzew
// names are now drawn after all lanes and junctions have been drawn
//
// Revision 1.22  2006/09/18 09:57:01  dkrajzew
// removed unneeded code
//
// Revision 1.21  2006/07/06 06:36:11  dkrajzew
// removed some old code
//
// Revision 1.20  2006/04/18 08:12:04  dkrajzew
// consolidation of interaction with gl-objects
//
// Revision 1.19  2006/04/11 10:56:32  dkrajzew
// microsimID() now returns a const reference
//
// Revision 1.18  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.17  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.16  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.15  2005/07/12 12:16:57  dkrajzew
// code style adapted; inclusion of config patched
//
// Revision 1.14  2005/05/04 07:59:59  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.13  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.2  2004/10/29 06:01:54  dksumo
// renamed boundery to boundary
//
// Revision 1.1  2004/10/22 12:49:17  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.12  2004/04/02 11:20:35  dkrajzew
// changes needed to visualise the selection status
//
// Revision 1.11  2004/03/19 12:57:54  dkrajzew
// porting to FOX
//
// Revision 1.10  2004/01/26 06:59:37  dkrajzew
// work on detectors: e3-detectors loading and visualisation; variable offsets
//  and lengths for lsa-detectors; coupling of detectors to tl-logics;
//  different detector visualistaion in dependence to his controller
//
// Revision 1.9  2003/11/11 08:13:23  dkrajzew
// consequent usage of Position2D instead of two SUMOReals
//
// Revision 1.8  2003/09/22 12:38:51  dkrajzew
// detectors need const Lanes
//
// Revision 1.7  2003/09/05 14:58:50  dkrajzew
// first tries for an implementation of aggregated views
//
// Revision 1.6  2003/07/22 14:59:27  dkrajzew
// changes due to new detector handling
//
// Revision 1.5  2003/07/16 15:24:55  dkrajzew
// GUIGrid now handles the set of things to draw in another manner
//  than GUIEdgeGrid did; Further things to draw implemented
//
// Revision 1.4  2003/03/17 14:09:10  dkrajzew
// Windows eol removed
//
// Revision 1.3  2003/03/12 16:52:06  dkrajzew
// centering of objects debuggt
//
// Revision 1.2  2003/02/07 10:39:17  dkrajzew
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

#include <vector>
#include <string>
#include <utils/geom/Position2D.h>
#include <utils/geom/Boundary.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include "GUILaneWrapper.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSJunction;
class GUILane;
class GUIBasicLane;
class GUIGlObjectStorage;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * This is the gui-version of the MSEdge
 */
class GUIEdge : public MSEdge, public GUIGlObject
{
public:
    /// constructor
    GUIEdge(const std::string &id, size_t numericalID,
        GUIGlObjectStorage &idStorage);

    /// destructor
    ~GUIEdge();

    /// sets the junctions allowing the retrieval of the edge position (!!! why not private with a friend?)
    void initGeometry(GUIGlObjectStorage &idStorage);

    /** Returns the list of all known junctions as their names */
    static std::vector<std::string> getNames();

    /** Returns the list of all known junctions as their ids */
    static std::vector<size_t> getIDs();

    /// Returns the street's geometry
    Boundary getBoundary() const;

    /// returns the enumerated lane (!!! why not private with a friend?)
    MSLane &getLane(size_t laneNo);



    /// returns the enumerated lane's geometry (!!! why not private with a friend?)
    GUILaneWrapper &getLaneGeometry(size_t laneNo) const;

    GUILaneWrapper &getLaneGeometry(const MSLane *lane) const;

    /** returns the position on the line given by the coordinates where "prev"
        is the length of the line and "wanted" the distance from the begin
        !!! should be within another class */
    static std::pair<SUMOReal, SUMOReal> getLaneOffsets(SUMOReal x1, SUMOReal y1,
        SUMOReal x2, SUMOReal y2, SUMOReal prev, SUMOReal wanted);

    static void fill(std::vector<GUIEdge*> &netsWrappers);


    //@{ From GUIGlObject
    /// Returns a popup-menu
    virtual GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// Returns the parameter window
    virtual GUIParameterTableWindow *getParameterWindow(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// Returns the type of the object as coded in GUIGlObjectType
    GUIGlObjectType getType() const;

    /// returns the id of the object as known to microsim
    const std::string &microsimID() const;

    /// Returns the information whether this object is still active
    bool active() const;

    /// Returns the boundary to which the object shall be centered
	Boundary getCenteringBoundary() const;
    //@}


private:
    /// Definition of the lane's positions vector
    typedef std::vector<GUILaneWrapper*> LaneWrapperVector;

    /// List of the edge's lanes geometrical information
    LaneWrapperVector _laneGeoms;

    /**
     * A class to find the matching lane wrapper
     */
    class lane_wrapper_finder {
    public:
        /** constructor */
        explicit lane_wrapper_finder(const MSLane &lane) : myLane(lane) { }

        /** the comparing function */
        bool operator() (const GUILaneWrapper * const wrapper) {
            return wrapper->forLane(myLane);
        }

    private:
        /// The time to search for
        const MSLane &myLane;

    };


private:
    /// invalidated copy constructor
    GUIEdge(const GUIEdge &s);

    /// invalidated assignment operator
    GUIEdge &operator=(const GUIEdge &s);

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

