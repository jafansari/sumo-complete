#ifndef RODUAEdgeBuilder_h
#define RODUAEdgeBuilder_h
//---------------------------------------------------------------------------//
//                        RODUAEdgeBuilder.h -
//      The builder for due-edges
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 20 Jan 2004
//  copyright            : (C) 2004 by Daniel Krajzewicz
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
// Revision 1.5  2006/11/29 07:51:22  dkrajzew
// added the possibility to use the loaded weights outside their boundaries
//
// Revision 1.4  2005/10/07 11:42:28  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 12:05:23  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/05/04 08:57:12  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.1  2004/01/26 06:08:38  dkrajzew
// initial commit for dua-classes
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

#include <router/ROAbstractEdgeBuilder.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class ROEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class RODUAEdgeBuilder
 * This class builds edges that may be used by the dynamic user assignment-
 *  router.
 */
class RODUAEdgeBuilder : public ROAbstractEdgeBuilder {
public:
    /// Constructor
    RODUAEdgeBuilder(bool useBoundariesOnOverride);

    /// Destructor
    ~RODUAEdgeBuilder();

    /// Builds the dua-edge
    ROEdge *buildEdge(const std::string &name);

protected:
    /// Information whether to use the first/last entry of the time in case it is too short
    bool myUseBoundariesOnOverride;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
