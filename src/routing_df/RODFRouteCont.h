/****************************************************************************/
/// @file    DFRORouteCont.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// A container for routes
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
#ifndef DFRORouteCont_h
#define DFRORouteCont_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <map>
#include "DFRORouteDesc.h"


// ===========================================================================
// class declarations
// ===========================================================================
class RODFNet;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class DFRORouteCont
 * @brief A container for routes
 */
class DFRORouteCont
{
public:
    DFRORouteCont(const RODFNet &net);

    ~DFRORouteCont();
    void addRouteDesc(DFRORouteDesc &desc);
    bool removeRouteDesc(DFRORouteDesc &desc);
    bool save(std::vector<std::string> &saved,
              const std::string &prependix, OutputDevice& out);
    std::vector<DFRORouteDesc> &get();
    void sortByDistance();
    void setDets2Follow(const std::map<ROEdge*, std::vector<ROEdge*> > &d2f);
    const std::map<ROEdge*, std::vector<ROEdge*> > &getDets2Follow() const;
    void removeIllegal(const std::vector<std::vector<ROEdge*> > &illegals);

    class RoutesMap
    {
    public:
        RoutesMap();
        ~RoutesMap();
        friend std::ostream &operator<<(std::ostream &os, const RoutesMap &rm);
        void write(std::ostream &os, size_t offset) const;

        std::vector<ROEdge*> common;
        std::map<ROEdge*, RoutesMap*> splitMap;
        std::vector<std::string> lastDetectors;
        ROEdge* lastDetectorEdge;
    };

    RoutesMap *getRouteMap(const RODFNet &net) const;

    void determineEndDetector(const RODFNet &net, DFRORouteCont::RoutesMap *rmap) const;

protected:
    class by_distance_sorter
    {
    public:
        /// constructor
        explicit by_distance_sorter() { }

        int operator()(const DFRORouteDesc &p1, const DFRORouteDesc &p2) {
            return p1.distance2Last<p2.distance2Last;
        }
    };

    class route_finder
    {
    public:
        /** constructor */
        explicit route_finder(const DFRORouteDesc &desc) : myDesc(desc) { }

        /** the comparing function */
        bool operator()(const DFRORouteDesc &desc) {
            return myDesc.edges2Pass==desc.edges2Pass;
        }

    private:
        /// The time to search for
        const DFRORouteDesc &myDesc;

    };






    std::vector<DFRORouteDesc> myRoutes;
    std::map<ROEdge*, std::vector<ROEdge*> > myDets2Follow;
    const RODFNet &myNet;

};


#endif

/****************************************************************************/

