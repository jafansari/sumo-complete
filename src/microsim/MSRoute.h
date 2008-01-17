/****************************************************************************/
/// @file    MSRoute.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A vehicle route
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
#ifndef MSRoute_h
#define MSRoute_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
#include <utils/common/Named.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class BinaryInputDevice;
class OutputDevice;


typedef std::vector<const MSEdge*> MSEdgeVector;
typedef MSEdgeVector::const_iterator MSRouteIterator;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRoute
 */
class MSRoute : public Named
{
public:
    /// Constructor
    MSRoute(const std::string &id, const MSEdgeVector &edges,
            bool multipleReferenced);

    /// Destructor
    virtual ~MSRoute();

    /// Returns the begin of the list of edges to pass
    MSRouteIterator begin() const;

    /// Returns the end of the list of edges to pass
    MSRouteIterator end() const;

    /// Returns the number of edges to pass
    unsigned size() const;

    /// returns the destination edge
    const MSEdge *getLastEdge() const;

    /** @brief Returns the information whether the route is needed in the future
        This may be the case, when more than a single vehicle use the same route */
    bool inFurtherUse() const;

    bool replaceBy(const MSEdgeVector &edges, MSRouteIterator &currentEdge);

    void writeEdgeIDs(OutputDevice &os) const;

    bool contains(MSEdge *edge) const;
    bool containsAnyOf(const std::vector<MSEdge*> &edgelist) const;

    const MSEdge *operator[](unsigned index);

    static void dict_saveState(std::ostream &os);
    static void dict_loadState(BinaryInputDevice &bis);

    unsigned posInRoute(const MSRouteIterator &currentEdge) const;

    const MSEdgeVector &getEdges() const {
        return myEdges;
    }

public:
    /** @brief Adds a route to the dictionary
        Returns true if the route could be added, fals if a route with the same name already exists */
    static bool dictionary(const std::string &id, MSRoute *route);

    /** @brief Returns the named route
        Returns 0 if no route with the given name exitsts */
    static MSRoute *dictionary(const std::string &id);

    /// Clears the dictionary (delete all known routes, too)
    static void clear();

    /// Destroys the named route, removing it also from the dictionary
    static void erase(std::string id);

    MSRouteIterator find(const MSEdge *e) const;


private:
    /// The list of edges to pass
    MSEdgeVector myEdges;

    /// Information whether the route is used by more than a single vehicle
    bool myMultipleReferenced;

private:
    /// Definition of the dictionary container
    typedef std::map<std::string, MSRoute*> RouteDict;

    /// The dictionary container
    static RouteDict myDict;

};


#endif

/****************************************************************************/

