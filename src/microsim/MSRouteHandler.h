/****************************************************************************/
/// @file    MSRouteHandler.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Parser and container for routes during their loading
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
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
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include "MSVehicle.h"
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSVehicleType;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRouteHandler
 * @brief Parser and container for routes during their loading
 *
 * MSRouteHandler is the container for routes while they are build until
 * their transfering to the MSNet::RouteDict
 * The result of the operations are single MSNet::Route-instances
 */
class MSRouteHandler : public SUMOSAXHandler {
public:
    /// standard constructor
    MSRouteHandler(const std::string &file,
                   bool addVehiclesDirectly);

    /// standard destructor
    virtual ~MSRouteHandler() throw();

    /// Returns the last loaded depart time
    SUMOTime getLastDepart() const;

    /// Returns the last loaded vehicle
    MSVehicle *retrieveLastReadVehicle();

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    virtual void myStartElement(SumoXMLTag element,
                                const SUMOSAXAttributes &attrs) throw(ProcessError);


    /** @brief Called when characters occure
     *
     * @param[in] element ID of the last opened element
     * @param[in] chars The read characters (complete)
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myCharacters
     */
    virtual void myCharacters(SumoXMLTag element,
                              const std::string &chars) throw(ProcessError);


    /** @brief Called when a closing tag occures
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    virtual void myEndElement(SumoXMLTag element) throw(ProcessError);
    //@}


    /** parses an occured vehicle type definition */
    void addVehicleType(const SUMOSAXAttributes &attrs);

    /** opens a type distribution for reading */
    void openVehicleTypeDistribution(const SUMOSAXAttributes &attrs);

    /** closes (ends) the building of a distribution */
    void closeVehicleTypeDistribution();

    /** opens a route for reading */
    virtual void openRoute(const SUMOSAXAttributes &attrs);

    /** wrapper for constructor call to allow override in (gui) subclasses */
    virtual MSRoute* buildRoute() throw();

    /** closes (ends) the building of a route.
        Afterwards no edges may be added to it;
        this method may throw exceptions when
        a) the route is empty or
        b) another route with the same id already exists */
    void closeRoute() throw(ProcessError);

    /** reads the route elements */
    void addRouteElements(const std::string &chars);

    /** opens a route distribution for reading */
    void openRouteDistribution(const SUMOSAXAttributes &attrs);

    /** closes (ends) the building of a distribution */
    void closeRouteDistribution();

    /// Ends the processing of a vehicle
    virtual bool closeVehicle() throw(ProcessError);

protected:
    SUMOVehicleParameter *myVehicleParameter;

    /// the emission time of the vehicle read last
    SUMOTime myLastDepart;

    /// the last vehicle read
    MSVehicle *myLastReadVehicle;


    /// the current route
    MSEdgeVector myActiveRoute;

    /// the id of the current route
    std::string myActiveRouteID;

    /// the id of the current route
    SUMOReal myActiveRouteProbability;

    /** information whether vehicles shall be directly added to the network
        or kept within the buffer */
    bool myAddVehiclesDirectly;

    /// List of the stops the vehicle will make
    std::vector<MSVehicle::Stop> myVehicleStops;

    /// @brief The currently parsed distribution of vehicle types (probability->vehicle type)
    RandomDistributor<MSVehicleType*> *myCurrentVTypeDistribution;

    /// @brief The id of the currently parsed vehicle type distribution
    std::string myCurrentVTypeDistributionID;

    /// @brief The currently parsed distribution of routes (probability->route)
    RandomDistributor<const MSRoute*> *myCurrentRouteDistribution;

    /// @brief The id of the currently parsed route distribution
    std::string myCurrentRouteDistributionID;

    /// Information whether not all vehicle shall be emitted due to inc-dua
    bool myAmUsingIncrementalDUA;

    /// The current vehicle number (for inc-dua)
    unsigned int myRunningVehicleNumber;

    /// The base for inc-dua
    int myIncrementalBase;

    /// The current stage of inc-dua
    int myIncrementalStage;

    /// whether we have warned about routes defined as character strings
    bool myHaveWarned;

private:
    /** invalid copy constructor */
    MSRouteHandler(const MSRouteHandler &s);

    /** invalid assignment operator */
    MSRouteHandler &operator=(const MSRouteHandler &s);

};


#endif

/****************************************************************************/

