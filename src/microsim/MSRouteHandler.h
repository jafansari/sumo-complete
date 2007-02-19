/****************************************************************************/
/// @file    MSRouteHandler.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Parser and container for routes during their loading
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
#ifndef MSRouteHandler_h
#define MSRouteHandler_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include "MSRoute.h"
#include "MSEmitControl.h"
#include "MSVehicle.h"
#include <utils/sumoxml/SUMOSAXHandler.h>
#include <utils/sumoxml/SUMOBaseRouteHandler.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSVehicle;
class RGBColor;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * MSRouteHandler
 * MSRouteHandler is the container for routes while they are build until
 * their transfering to the MSNet::RouteDict
 * The result of the operations are single MSNet::Route-instances
 */
class MSRouteHandler : public SUMOSAXHandler, public SUMOBaseRouteHandler
{
public:
    /// standard constructor
    MSRouteHandler(const std::string &file, MSVehicleControl &vc,
                   bool addVehiclesDirectly, bool wantsVehicleColor,
                   int incDUABase, int incDUAStage);

    /// standard destructor
    virtual ~MSRouteHandler();

    /// opens a route for the addition of edges
    void openRoute(const std::string &id, bool multiReferenced);

    /// adds an edge to the route
    void addEdge(MSEdge *edge);

    /** closes (ends) the building of a route.
        Afterwards no edges may be added to it;
        this method may throw exceptions when
        a) the route is empty or
        b) another route with the same id already exists */
    virtual void closeRoute();

    SUMOTime getLastDepart() const;

    MSVehicle *retrieveLastReadVehicle();

protected:
    /** implementation of the GenericSAXHandler-myStartElement - interface */
    virtual void myStartElement(int element, const std::string &name,
                                const Attributes &attrs);

    /** implementation of the GenericSAXHandler-myCharacters - interface */
    virtual void myCharacters(int element, const std::string &name,
                              const std::string &chars);

    /** implementation of the GenericSAXHandler-myEndElement - interface */
    virtual void myEndElement(int element, const std::string &name);


    /** parses an occured vehicle type definition */
    virtual void addVehicleType(const Attributes &attrs);

    /** adds the parsed vehicle type */
    virtual void addParsedVehicleType(const std::string &id,
                                      const SUMOReal length, const SUMOReal maxspeed, const SUMOReal bmax,
                                      const SUMOReal dmax, const SUMOReal sigma,
                                      SUMOVehicleClass vclass);


    /* parses an occured vehicle definition */
    //void openVehicle(const Attributes &attrs);

    /** adds the parsed vehicle /
    MSVehicle *addParsedVehicle(const std::string &id,
        const std::string &vtypeid, const std::string &routeid,
        const long &depart, int repNumber, int repOffset, RGBColor &c);
    */

    /** opens a route for reading */
    virtual void openRoute(const Attributes &attrs);

    /** reads the route elements */
    void addRouteElements(const std::string &name,
                          const std::string &chars);

    void closeVehicle();


protected:
    MSVehicleControl &myVehicleControl;

    /// the emission time of the vehicle read last
    SUMOTime myLastDepart;

    /// the last vehicle read
    MSVehicle *myLastReadVehicle;

    /// the current route
    MSEdgeVector myActiveRoute;

    /// the id of the current route
    std::string myActiveRouteID;

    /** information wheter the route shall be kept after being passed
        (otherwise it will be deleted) */
    bool m_IsMultiReferenced;

    /** information whether vehicles shall be directly added to the network
        or kept within the buffer */
    bool myAddVehiclesDirectly;

    /** information whether colors of vehicles shall be loaded */
    bool myWantVehicleColor;

    /// The vehicle's explicite route
    MSRoute *myCurrentEmbeddedRoute;

    /// List of the stops the vehicle will make
    std::vector<MSVehicle::Stop> myVehicleStops;


    /// Information whether not all vehicle shall be emitted due to inc-dua
    bool myAmUsingIncrementalDUA;

    /// The current vehicle number (for inc-dua)
    size_t myRunningVehicleNumber;

    /// The base for inc-dua
    int myIncrementalBase;

    /// The current stage of inc-dua
    int myIncrementalStage;

private:
    /** invalid copy constructor */
    MSRouteHandler(const MSRouteHandler &s);

    /** invalid assignment operator */
    MSRouteHandler &operator=(const MSRouteHandler &s);

};


#endif

/****************************************************************************/

