/****************************************************************************/
/// @file    RORDLoader_TripDefs.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The basic class for loading trip definitions
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
#ifndef RORDLoader_TripDefs_h
#define RORDLoader_TripDefs_h
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
#include <utils/options/OptionsCont.h>
#include <utils/gfx/RGBColor.h>
#include <utils/router/IDSupplier.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "ROTypedXMLRoutesLoader.h"
#include "RONet.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORDLoader_TripDefs
 * A handler for route definitions which consists of the origin and the
 * destination edge only. Additionally, read vehicles may have information
 * about a certain position to leave from and a certain speed to leave with.
 */
class RORDLoader_TripDefs : public ROTypedXMLRoutesLoader
{
public:
    /// Constructor
    RORDLoader_TripDefs(ROVehicleBuilder &vb,
                        RONet &net, SUMOTime begin, SUMOTime end,
                        bool emptyDestinationsAllowed, const std::string &file="");

    /// Destructor
    ~RORDLoader_TripDefs();

    /** @brief Returns the name of the data read.
        "XML-route definitions" is returned here */
    std::string getDataName() const;

    /// Returns the time the current (last read) route starts at
    SUMOTime getCurrentTimeStep() const;

protected:
    /** the user-impemlented handler method for an opening tag */
    void myStartElement(int element, const std::string &name,
                        const Attributes &attrs);

    /** the user-implemented handler method for characters */
    void myCharacters(int element, const std::string &name,
                      const std::string &chars);

    /** the user-implemented handler method for a closing tag */
    void myEndElement(int element, const std::string &name);

protected:
    /// Return the information whether a route was read
    bool nextRouteRead();

    /// Initialises the reading of a further route
    void beginNextRoute();

protected:
    /// Parses the vehicle id
    std::string getVehicleID(const Attributes &attrs);

    /// Parses a named edge frm the attributes
    ROEdge *getEdge(const Attributes &attrs, const std::string &purpose,
                    AttrEnum which, const std::string &id, bool emptyAllowed);

    /** @brief Parses a SUMOReal value which is optional;
        Avoids exceptions */
    SUMOReal getOptionalFloat(const Attributes &attrs,
                              const std::string &name, AttrEnum which, const std::string &place);

    /// Parses and returns the time the vehicle should start at
    SUMOTime getTime(const Attributes &attrs, AttrEnum which,
                     const std::string &id);

    /// Parses and returns the period the trip shall be repeated with
    SUMOTime getPeriod(const Attributes &attrs, const std::string &id);

    /// Parses and returns the number of cars that shall use the same parameter
    int getRepetitionNumber(const Attributes &attrs, const std::string &id);

    /** @brief Parses and returns the name of the lane the vehicle starts at
        This value is optional */
    std::string getLane(const Attributes &attrs);

    RGBColor getRGBColorReporting(const Attributes &attrs,
                                  const std::string &id);

protected:
    /// generates numerical ids
    IDSupplier _idSupplier;

    /// The id of the vehicle
    std::string myID;

    /// The starting edge
    ROEdge *myBeginEdge;

    /// The end edge
    ROEdge *myEndEdge;

    /// The type of the vehicle
    std::string myType;

    /// The optional position the vehicle shall star at
    SUMOReal myPos;

    /// The optional speed the vehicle shall start with
    SUMOReal mySpeed;

    /** @brief Information whether empty destinations are allowed
        This is a feature used for the handling of explicite routes within the
        jp-router where the destination is not necessary */
    bool myEmptyDestinationsAllowed;

    /// The time the vehicle shall start at
    SUMOTime myDepartureTime;

    /// The period the next vehicle with the same route shall be emitted at
    SUMOTime myPeriodTime;

    /// The number of times cars with the current parameter shall be reemitted
    int myNumberOfRepetitions;

    /// The optional name of the lane the vehicle shall start at
    std::string myLane;

    /// The list of edges to take when given
    ROEdgeVector myEdges;

    /// The color of the vehicle
    RGBColor myColor;

    /// The information whether the next route was read
    bool _nextRouteRead;

private:
    /// we made the copy constructor invalid
    RORDLoader_TripDefs(const RORDLoader_TripDefs &src);

    /// we made the assignment operator invalid
    RORDLoader_TripDefs &operator=(const RORDLoader_TripDefs &src);

};


#endif

/****************************************************************************/

