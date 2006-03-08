#ifndef GUILaneDrawer_SGnT_h
#define GUILaneDrawer_SGnT_h
//---------------------------------------------------------------------------//
//                        GUILaneDrawer_SGnT.h -
//  Class for drawing lanes with simple geometry and no tooltip information
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
// Revision 1.6  2006/03/08 13:16:23  dkrajzew
// some work on lane visualization
//
// Revision 1.5  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.4  2005/10/07 11:45:09  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/23 06:07:54  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.2  2005/09/15 12:19:10  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.1  2004/11/23 10:38:30  dkrajzew
// debugging
//
// Revision 1.1  2004/10/22 12:50:49  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.2  2003/09/17 06:45:11  dkrajzew
// some documentation added/patched
//
// Revision 1.1  2003/09/05 14:50:39  dkrajzew
// implementations of artefact drawers moved to folder "drawerimpl"
//
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

#include <map>
#include <utils/gfx/RGBColor.h>
#include <utils/common/StdDefs.h>
#include "GUIBaseLaneDrawer.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUILaneWrapper;
class Position2D;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * Draws lanes as simple, one-colored straights
 */
template<class _E1, class _E2, class _L1>
class GUILaneDrawer_SGnT : public GUIBaseLaneDrawer<_E1, _E2, _L1> {
public:
    /// constructor
    GUILaneDrawer_SGnT(const std::vector<_E1*> &edges)
		: GUIBaseLaneDrawer<_E1, _E2, _L1>(edges) { }

    /// destructor
		~GUILaneDrawer_SGnT() { }

private:
    /// draws a single lane as a box
    void drawLane(const _L1 &lane, SUMOReal mult) const
	{
        glPushMatrix();
	    const Position2D &beg = lane.getBegin();
		glTranslated(beg.x(), beg.y(), 0);
		glRotated( lane.getRotation(), 0, 0, 1 );
	    SUMOReal visLength = -lane.visLength();
		glBegin( GL_QUADS );
		glVertex2d(-SUMO_const_halfLaneWidth*mult, 0);
	    glVertex2d(-SUMO_const_halfLaneWidth*mult, visLength);
		glVertex2d(SUMO_const_halfLaneWidth*mult, visLength);
	    glVertex2d(SUMO_const_halfLaneWidth*mult, 0);
		glEnd();
		glBegin( GL_LINES);
	    glVertex2d(0, 0);
		glVertex2d(0, visLength);
		glEnd();
	    glPopMatrix();
	}

    /// draws a single edge as a box
    void drawEdge(const _E2 &edge, SUMOReal mult) const
	{
        throw 1;
	}

    /// draws a lane as a line
    void drawLine(const _L1 &lane) const
	{
        const Position2D &begin = lane.getBegin();
	    const Position2D &end = lane.getEnd();
		glBegin( GL_LINES);
	    glVertex2d(begin.x(), begin.y());
		glVertex2d(end.x(), end.y());
	    glEnd();
	}

    /// draws an edge as a line
    void drawLine(const _E2 &edge) const
	{
        throw 1;
	}

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

