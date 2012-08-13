/****************************************************************************/
/// @file    SUMORTree.h
/// @author  Daniel Krajzewicz
/// @date    27.10.2008
/// @version $Id$
///
// An rtree for networks
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef SUMORTree_h
#define SUMORTree_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/geom/Boundary.h>
#include <utils/foxtools/MFXMutex.h>

#include "RTree.h"


// specialized implementation for speedup and avoiding warnings
template<>
inline float RTree<GUIGlObject*, GUIGlObject, float, 2, GUIVisualizationSettings, float, 8, 4>::RectSphericalVolume(Rect* a_rect)
{
  ASSERT(a_rect);
  const float extent0 = a_rect->m_max[0] - a_rect->m_min[0];
  const float extent1 = a_rect->m_max[1] - a_rect->m_min[1];
  return .78539816f * (extent0 * extent0 + extent1 * extent1);
}


// ===========================================================================
// class definitions
// ===========================================================================
class SUMORTree : private RTree<GUIGlObject*, GUIGlObject, float, 2, GUIVisualizationSettings>, public Boundary
{
public:
    SUMORTree() 
        : RTree<GUIGlObject*, GUIGlObject, float, 2, GUIVisualizationSettings, float>(&GUIGlObject::drawGL){
    }

    ~SUMORTree() {
    }

    int Search(const float a_min[2], const float a_max[2], const GUIVisualizationSettings& c) {
        Locker lock = lockInScope();
        return RTree::Search(a_min, a_max, c);
    }

    void Insert(const float a_min[2], const float a_max[2], GUIGlObject* a_dataId) {
        Locker lock = lockInScope();
        RTree::Insert(a_min, a_max, a_dataId);
    }

    void Remove(const float a_min[2], const float a_max[2], GUIGlObject* a_dataId) {
        Locker lock = lockInScope();
        RTree::Remove(a_min, a_max, a_dataId);
    }

    /** @brief Adds an additional object (detector/shape/trigger) for visualisation
     * @param[in] o The object to add
     */
    void addAdditionalGLObject(GUIGlObject *o) {
        Boundary b = o->getCenteringBoundary();
        const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
        const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
        Insert(cmin, cmax, o);
    }

    /** @brief Removes an additional object (detector/shape/trigger) from being visualised
     * @param[in] o The object to remove
     */
    void removeAdditionalGLObject(GUIGlObject *o) {
        Boundary b = o->getCenteringBoundary();
        const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
        const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
        Remove(cmin, cmax, o);
    }

protected:
    class Locker {
            friend class SUMORTree;
            public:
                    ~Locker() {
                            myLock.unlock();
                    }
            private:
                    Locker(MFXMutex& lock) : myLock(lock) {
                            myLock.lock();
                    }
            private:
                    MFXMutex& myLock;
    };
    Locker lockInScope() {
            return Locker(myLock);
    }
    MFXMutex myLock;

};


#endif

/****************************************************************************/

