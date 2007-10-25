/****************************************************************************/
/// @file    GUIEdgeControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A builder for edges during the loading derived from the
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <guisim/GUIEdge.h>
#include <guisim/GUINet.h>
#include <guisim/GUILane.h>
#include <guisim/GUISourceLane.h>
#include <guisim/GUIInternalLane.h>
#include <microsim/MSJunction.h>
#include <netload/NLBuilder.h>
#include "GUIEdgeControlBuilder.h"
#include <gui/GUIGlobals.h>
#include <utils/gui/globjects/GUIGlObjectGlobals.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
GUIEdgeControlBuilder::GUIEdgeControlBuilder(
    GUIGlObjectStorage &glObjectIDStorage, unsigned int storageSize)
        : NLEdgeControlBuilder(storageSize),
        myGlObjectIDStorage(glObjectIDStorage)
{}

GUIEdgeControlBuilder::~GUIEdgeControlBuilder()
{}


MSEdge *
GUIEdgeControlBuilder::addEdge(const string &id)
{
    MSEdge *edge = new GUIEdge(id, myCurrentNumericalEdgeID++, myGlObjectIDStorage);
    if (!MSEdge::dictionary(id, edge)) {
        throw InvalidArgument("Another edge with the id '" + id + "' exists.");
    }
    m_pEdges->push_back(edge);
    return edge;
}


MSEdge *
GUIEdgeControlBuilder::closeEdge()
{
    MSEdge *ret = NLEdgeControlBuilder::closeEdge();
    static_cast<GUIEdge*>(ret)->initGeometry(gIDStorage);
    return ret;
}


MSLane *
GUIEdgeControlBuilder::addLane(/*MSNet &net, */const std::string &id,
        SUMOReal maxSpeed, SUMOReal length, bool isDepart,
        const Position2DVector &shape,
        const std::string &vclasses)
{
    // checks if the depart lane was set before
    if (isDepart&&m_pDepartLane!=0) {
        throw InvalidArgument("Lane's '" + id + "' edge already has a depart lane.");
    }
    std::vector<SUMOVehicleClass> allowed, disallowed;
    parseVehicleClasses(vclasses, allowed, disallowed);
    MSLane *lane = 0;
    switch (m_Function) {
    case MSEdge::EDGEFUNCTION_SOURCE:
        lane = new GUISourceLane(/*net, */id, maxSpeed, length, m_pActiveEdge,
                                          myCurrentNumericalLaneID++, shape, allowed, disallowed);
        break;
    case MSEdge::EDGEFUNCTION_INTERNAL:
        lane = new GUIInternalLane(/*net, */id, maxSpeed, length, m_pActiveEdge,
                                            myCurrentNumericalLaneID++, shape, allowed, disallowed);
        break;
    case MSEdge::EDGEFUNCTION_NORMAL:
    case MSEdge::EDGEFUNCTION_SINK:
        lane = new GUILane(/*net, */id, maxSpeed, length, m_pActiveEdge,
                                    myCurrentNumericalLaneID++, shape, allowed, disallowed);
        break;
    default:
        throw InvalidArgument("A lane with an unknown type occured (" + toString(m_Function) + ")");
    }
    m_pLaneStorage->push_back(lane);
    if (isDepart) {
        m_pDepartLane = lane;
    }
    return lane;
}



/****************************************************************************/

