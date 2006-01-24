#ifndef RORouteDef_Alternatives_h
#define RORouteDef_Alternatives_h
//---------------------------------------------------------------------------//
//                        RORouteDef_Alternatives.h -
//  A route with alternative routes
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
// Revision 1.9  2006/01/24 13:43:53  dkrajzew
// added vehicle classes to the routing modules
//
// Revision 1.8  2006/01/09 12:00:59  dkrajzew
// debugging vehicle color usage
//
// Revision 1.7  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/05/04 08:52:12  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; trying to debug invalid vehicles handling
//
// Revision 1.3  2004/11/23 10:25:52  dkrajzew
// debugging
//
// Revision 1.2  2004/10/29 06:18:52  dksumo
// max-alternatives options added
//
// Revision 1.1  2004/10/22 12:50:27  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.2  2004/07/02 09:39:41  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived
//  for an online-routing
//
// Revision 1.1  2004/01/26 08:02:27  dkrajzew
// loaders and route-def types are now renamed in an senseful way;
//  further changes in order to make both new routers work;
//  documentation added
//
// ------------------------------------------------
// Revision 1.6  2003/11/11 08:04:46  dkrajzew
// avoiding emissions of vehicles on too short edges
//
// Revision 1.5  2003/07/30 09:26:33  dkrajzew
// all vehicles, routes and vehicle types may now have specific colors
//
// Revision 1.4  2003/06/18 11:36:50  dkrajzew
// a new interface which allows to choose whether to stop after a route could
//  not be computed or not; not very sphisticated, in fact
//
// Revision 1.3  2003/03/20 16:39:17  dkrajzew
// periodical car emission implemented; windows eol removed
//
// Revision 1.2  2003/02/07 10:45:07  dkrajzew
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

#include <string>
#include <vector>
#include "ROEdgeVector.h"
#include "RORouteDef.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
class ROAbstractRouter;
class ROEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class RORouteDef_Alternatives
 * @brief A route definition which has some alternatives, already.
 */
class RORouteDef_Alternatives : public RORouteDef {
public:
    /// Constructor
    RORouteDef_Alternatives(const std::string &id, const RGBColor &color,
        size_t lastUsed, SUMOReal gawronBeta, SUMOReal gawronA, int maxRoutes);

    /// Destructor
    virtual ~RORouteDef_Alternatives();

    /** @brief Adds an alternative loaded from the file
        An alternative may also be generated whicle DUA */
    virtual void addLoadedAlternative(RORoute *alternative);

    /// Returns the trip's origin edge
    ROEdge *getFrom() const;

    /// returns the trip's destination edge
    ROEdge *getTo() const;

    /// Build the next route
    RORoute *buildCurrentRoute(ROAbstractRouter &router, SUMOTime begin,
        bool continueOnUnbuild, ROVehicle &veh,
		ROAbstractRouter::ROAbstractEdgeEffortRetriever * const retriever) const;

    /// Adds a build alternative
    void addAlternative(RORoute *current, SUMOTime begin);

    /** @brief Returns a copy of the route definition */
    RORouteDef *copy(const std::string &id) const;

    const ROEdgeVector &getCurrentEdgeVector() const;

    void invalidateLast();

	void addExplicite(RORoute *current, SUMOTime begin);

	void removeLast();

    /** @brief returns the index of the route that was used as last */
    virtual int getLastUsedIndex() const;

    /** @brief returns the number of alternatives */
    virtual size_t getAlternativesSize() const;

    /// Returns the alternative at the given index
    virtual const RORoute &getAlternative(size_t i) const;

private:
    /// Searches for the route within the list of alternatives
    int findRoute(RORoute *opt) const;

    /** @brief Performs the gawron - f() function
        From "Dynamic User Equilibria..." */
    SUMOReal gawronF(SUMOReal pdr, SUMOReal pds, SUMOReal x);

    /** @brief Performs the gawron - g() function
        From "Dynamic User Equilibria..." */
    SUMOReal gawronG(SUMOReal a, SUMOReal x);

private:
    /// Information whether a new route was generated
    mutable bool _newRoute;

    /// Index of the route used within the last step
    mutable int _lastUsed;

    /// Definition of the storage for alternatives
    typedef std::vector<RORoute*> AlternativesVector;

    /// The alternatives
    AlternativesVector _alternatives;

    /// gawron beta - value
    SUMOReal _gawronBeta;

    /// gawron a - value
    SUMOReal _gawronA;

    /// The maximum route number
    int myMaxRouteNumber;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

