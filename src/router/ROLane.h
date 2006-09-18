#ifndef ROLane_h
#define ROLane_h
//---------------------------------------------------------------------------//
//                        ROLane.h -
//  A single lane the router may use
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
// Revision 1.11  2006/09/18 10:15:17  dkrajzew
// changed vehicle class naming
//
// Revision 1.10  2006/01/26 08:42:50  dkrajzew
// made lanes and edges being aware to vehicle classes
//
// Revision 1.9  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2004/11/23 10:25:51  dkrajzew
// debugging
//
// Revision 1.5  2004/01/26 08:01:10  dkrajzew
// loaders and route-def types are now renamed in an senseful way; further changes in order to make both new routers work; documentation added
//
// Revision 1.4  2003/09/17 10:14:27  dkrajzew
// handling of unset values patched
//
// Revision 1.3  2003/02/07 10:45:07  dkrajzew
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

#include <vector>
#include <utils/common/Named.h>
#include <utils/common/SUMOVehicleClass.h>

class ROEdge;

/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class ROLane
 * The class for an edge's lane. Just a container for the speed and the
 * length of the lane.
 */
class ROLane : public Named {
public:
    /// Constructor
    ROLane(const std::string &id, SUMOReal length, SUMOReal maxSpeed,
		const std::vector<SUMOVehicleClass> &allowed,
		const std::vector<SUMOVehicleClass> &disallowed);

    /// Destructor
    ~ROLane();

    /// Returns the length of the lane
    SUMOReal getLength() const;

    /// Returns the maximum speed allowed on this lane
    SUMOReal getSpeed() const;

	/// Return the Edge
	ROEdge *getEdge();

	/// Returns the list of allowed vehicle classes
	const std::vector<SUMOVehicleClass> &getAllowedClasses() const;

	/// Returns the list of not allowed vehicle classes
	const std::vector<SUMOVehicleClass> &getNotAllowedClasses() const;

protected:

	ROEdge *myEdge;
private:
    /// The length of the lane
    SUMOReal myLength;

    /// The maximum speed allowed on the lane
    SUMOReal myMaxSpeed;

	/// The list of allowed vehicle classes
	std::vector<SUMOVehicleClass> myAllowedClasses;

	/// The list of disallowed vehicle classes
	std::vector<SUMOVehicleClass> myNotAllowedClasses;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

