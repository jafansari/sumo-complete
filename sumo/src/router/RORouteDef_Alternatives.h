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
// debugging while working on INVENT; preparation of classes to be derived for an online-routing
//
// Revision 1.1  2004/01/26 08:02:27  dkrajzew
// loaders and route-def types are now renamed in an senseful way; further changes in order to make both new routers work; documentation added
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
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
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
 * A route definition which has some alternatives, already.
 */
class RORouteDef_Alternatives : public RORouteDef {
public:
    /// Constructor
    RORouteDef_Alternatives(const std::string &id, const RGBColor &color,
        size_t lastUsed, double gawronBeta, double gawronA, int maxRoutes);

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
    RORoute *buildCurrentRoute(ROAbstractRouter &router, long begin,
        bool continueOnUnbuild, ROVehicle &veh,
        ROAbstractRouter::ROAbstractEdgeEffortRetriever * const retriever);

    /// Adds a build alternative
    void addAlternative(RORoute *current, long begin);

    /// Writes the currently chosen route to the stream
    void xmlOutCurrent(std::ostream &res, bool isPeriodical) const;

    /// Writes all the alternatives to the stream
    void xmlOutAlternatives(std::ostream &altres) const;

    /** @brief Returns a copy of the route definition */
    RORouteDef *copy(const std::string &id) const;

    const ROEdgeVector &getCurrentEdgeVector() const;

    void invalidateLast();

    void addExplicite(RORoute *current, long begin);

    void removeLast();

private:
    /// Searches for the route within the list of alternatives
    int findRoute(RORoute *opt) const;

    /** @brief Performs the gawron - f() function
        From "Dynamic User Equilibria..." */
    double gawronF(double pdr, double pds, double x);

    /** @brief Performs the gawron - g() function
        From "Dynamic User Equilibria..." */
    double gawronG(double a, double x);

private:
    /// Information whether a new route was generated
    bool _newRoute;

    /// Index of the route used within the last step
    int _lastUsed;

    /// Definition of the storage for alternatives
    typedef std::vector<RORoute*> AlternativesVector;

    /// The alternatives
    AlternativesVector _alternatives;

    /// gawron beta - value
    double _gawronBeta;

    /// gawron a - value
    double _gawronA;

    /// The maximum route number
    int myMaxRouteNumber;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

