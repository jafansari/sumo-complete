//---------------------------------------------------------------------------//
//                        GUISourceLane.cpp -
//  A grid of edges for faster drawing
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 25 Nov 2002
//  copyright            : (C) 2001 by DLR http://ivf.dlr.de/
//  author               : Daniel Krajzewicz
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.2  2003/07/07 08:14:48  dkrajzew
// first steps towards the usage of a real lane and junction geometry implemented
//
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
#include <iostream> // !!!
#include <utility>
#include <utils/qutils/NewQMutex.h>
#include <microsim/MSLane.h>
#include <utils/geom/Position2D.h>
#include <microsim/MSNet.h>
#include "GUILaneWrapper.h"
#include "GUISourceLane.h"


/* =========================================================================
 * method definitions
 * ======================================================================= */
GUISourceLane::GUISourceLane(MSNet &net, std::string id,
                             double maxSpeed, double length,
                             MSEdge* edge, const Position2DVector &shape )
    : MSSourceLane(net, id, maxSpeed, length, edge), myShape(shape)
{
}


GUISourceLane::~GUISourceLane()
{
}


void
GUISourceLane::moveNonCriticalSingle()
{
    _lock.lock();//Display();
    MSSourceLane::moveNonCriticalSingle();
    _lock.unlock();//Display();
}


void
GUISourceLane::moveCriticalSingle()
{
    _lock.lock();//Display();
    MSSourceLane::moveCriticalSingle();
    _lock.unlock();//Display();
}


void
GUISourceLane::moveNonCriticalMulti()
{
    _lock.lock();//Display();
    MSSourceLane::moveNonCriticalMulti();
    _lock.unlock();//Display();
}


void
GUISourceLane::moveCriticalMulti()
{
    _lock.lock();//Display();
    MSSourceLane::moveCriticalMulti();
    _lock.unlock();//Display();
}


void
GUISourceLane::moveNonCriticalMulti(MSEdge::LaneCont::const_iterator firstNeighLane,
                               MSEdge::LaneCont::const_iterator lastNeighLane )
{
    _lock.lock();//Display();
    MSSourceLane::moveNonCriticalMulti(firstNeighLane, lastNeighLane);
    _lock.unlock();//Display();
}


void
GUISourceLane::moveCriticalMulti(MSEdge::LaneCont::const_iterator firstNeighLane,
                               MSEdge::LaneCont::const_iterator lastNeighLane )
{
    _lock.lock();//Display();
    MSSourceLane::moveCriticalMulti(firstNeighLane, lastNeighLane);
    _lock.unlock();//Display();
}


void
GUISourceLane::setCritical()
{
    _lock.lock();//Display();
    MSSourceLane::setCritical();
    _lock.unlock();//Display();
}





bool
GUISourceLane::emit( MSVehicle& newVeh )
{
    _lock.lock();//Display();
    bool ret = MSSourceLane::emit(newVeh);
    _lock.unlock();//Display();
    return ret;
}


bool
GUISourceLane::isEmissionSuccess( MSVehicle* aVehicle )
{
    _lock.lock();//Display();
    bool ret = MSSourceLane::isEmissionSuccess(aVehicle);
    _lock.unlock();//Display();
    return ret;
}


bool
GUISourceLane::push( MSVehicle* veh )
{
    throw 1;
}


void
GUISourceLane::releaseVehicles()
{
    _lock.unlock();
}



const MSSourceLane::VehCont &
GUISourceLane::getVehiclesSecure()
{
    _lock.lock();
    return myVehicles;
}


void
GUISourceLane::swapAfterLaneChange()
{
    _lock.lock();
    myVehicles = myTmpVehicles;
    myTmpVehicles.clear();
    _lock.unlock();
}


void
GUISourceLane::integrateNewVehicle()
{
    _lock.lock();
    MSLane::integrateNewVehicle();
    _lock.unlock();
}


GUILaneWrapper *
GUISourceLane::buildLaneWrapper(GUIGlObjectStorage &idStorage)
{
    return new GUILaneWrapper(idStorage, *this, myShape);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "GUISourceLane.icc"
//#endif

// Local Variables:
// mode:C++
// End:


