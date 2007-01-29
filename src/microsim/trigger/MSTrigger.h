#ifndef MSTrigger_h
#define MSTrigger_h
//---------------------------------------------------------------------------//
//                        MSTrigger.h -
//  The basic class for triggering artifacts
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
// Revision 1.2  2005/10/07 11:37:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.1  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/01/06 10:48:07  dksumo
// 0.8.2.1 patches
//
// Revision 1.1  2004/10/22 12:49:31  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.4  2004/07/02 09:56:40  dkrajzew
// debugging while implementing the vss visualisation
//
// Revision 1.3  2003/05/20 09:31:46  dkrajzew
// emission debugged; movement model reimplemented (seems ok);
//  detector output debugged; setting and retrieval of some parameter added
//
// Revision 1.2  2003/02/07 10:41:51  dkrajzew
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

#include <utils/common/Named.h>


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class MSTrigger
 */
class MSTrigger : public Named {
public:
    /// Constructor
    MSTrigger(const std::string &id) : Named(id) { }

    /// Destructor
    virtual ~MSTrigger() { }

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

