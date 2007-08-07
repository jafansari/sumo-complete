/****************************************************************************/
/// @file    NBTrafficLightLogicVector.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A vector of traffic lights logics
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
#ifndef NBTrafficLightLogicVector_h
#define NBTrafficLightLogicVector_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include "NBConnectionDefs.h"

// ===========================================================================
// class declarations
// ===========================================================================
class NBTrafficLightLogic;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBTrafficLightLogicVector
 * This class holds all computed traffic light phases (with changing cliques)
 * valid for the given set of connections
 */
class NBTrafficLightLogicVector
{
public:
    /// Constructor
    NBTrafficLightLogicVector(const NBConnectionVector &inLanes,
                              std::string type);

    /// Destructor
    ~NBTrafficLightLogicVector();

    /// Adds a further traffic light phaselists to the list
    void add(NBTrafficLightLogic *logic);

    /// Adds all phaselists stored within the given list to the list of logics of this type
    void add(const NBTrafficLightLogicVector &cont);

    /// Writes the logics for this junction
    void writeXML(OutputDevice &os) const;

    /// returns the information whether the given phaselists is already within this container
    bool contains(NBTrafficLightLogic *logic) const;

    /// Returns the number of phaselists within this container
    int size() const;

    /// Returns the type of this traffic light logic
    const std::string &getType() const;

private:
    /// The links participating in this junction
    NBConnectionVector myInLinks;

    /// Definition of the container type for theknown phaselist
    typedef std::vector<NBTrafficLightLogic*> LogicVector;

    /// Container type for the phaselist
    LogicVector myCont;

    /// The type of the logic (traffic-light, actuated, agentbased)
    std::string myType;

};


#endif

/****************************************************************************/

