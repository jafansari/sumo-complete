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
#include <utils/common/RandomDistributor.h>


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
            bool multipleReferenced) throw();

    /// Destructor
    virtual ~MSRoute() throw();

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

    /// output the edge ids up to but not including the id of the given edge
    void writeEdgeIDs(OutputDevice &os, const MSEdge *upTo=0) const;

    bool contains(const MSEdge * const edge) const throw() {
        return find(edge)!=myEdges.end();
    }

    bool containsAnyOf(const std::vector<MSEdge*> &edgelist) const;

    const MSEdge *operator[](unsigned index) const;

#ifdef HAVE_MESOSIM
    /// @name State I/O (mesosim only)
    /// @{

    /** @brief Saves all known routes into the given stream
     *
     * @param[in] os The stream to write the routes into (binary)
     */
    static void dict_saveState(std::ostream &os) throw();


    /** @brief Loads routes from the state
     *
     * @param[in] bis The input to read the routes from (binary)
     */
    static void dict_loadState(BinaryInputDevice &bis) throw();
    /// @}
#endif

    unsigned posInRoute(const MSRouteIterator &currentEdge) const;

    const MSEdgeVector &getEdges() const {
        return myEdges;
    }

    SUMOReal getLength() const;

    /**
     * Compute the distance between 2 given edges on this route, including the length of internal lanes.
     * @param fromPos:		position on the first edge, at wich the computed distance begins
     * @param toPos:		position on the last edge, at which the coumputed distance endsance
     * @param fromEdge:		edge at wich computation begins
     * @param toEdge:		edge at which distance computation shall stop
     * @return				distance between the position fromPos on fromEdge and toPos on toEdge
     */
    SUMOReal getDistanceBetween(SUMOReal fromPos, SUMOReal toPos, const MSEdge* fromEdge, const MSEdge* toEdge) const;

public:
    /** @brief Adds a route to the dictionary
        Returns true if the route could be added, fals if a route with the same name already exists */
    static bool dictionary(const std::string &id, MSRoute *route);

    /** @brief Adds a route distribution to the dictionary
        Returns true if the route could be added, fals if a route with the same name already exists */
    static bool dictionary(const std::string &id, RandomDistributor<MSRoute*> *routeDist);

    /** @brief Returns the named route or a sample from the named distribution
        Returns 0 if no route (distribution) with the given name exitsts */
    static MSRoute *dictionary(const std::string &id);

    /// Clears the dictionary (delete all known routes, too)
    static void clear();

    /// Destroys the named route, removing it also from the dictionary
    static void erase(std::string id);

    MSRouteIterator find(const MSEdge *e) const;
    MSRouteIterator find(const MSEdge *e, const MSRouteIterator &startingAt) const;


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

    /// Definition of the dictionary container
    typedef std::map<std::string, RandomDistributor<MSRoute*>*> RouteDistDict;

    /// The dictionary container
    static RouteDistDict myDistDict;

};


#endif

/****************************************************************************/

