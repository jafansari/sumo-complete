//---------------------------------------------------------------------------//
//                        GUIHelpingJunction.cpp -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Tue, 29.05.2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.7  2006/09/18 10:00:07  dkrajzew
// patching junction-internal state simulation
//
// Revision 1.6  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.5  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/07/12 12:16:57  dkrajzew
// code style adapted; inclusion of config patched
//
// Revision 1.3  2005/07/12 12:00:09  dkrajzew
// level 3 warnings removed; code style adapted
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
#include <microsim/MSJunction.h>
#include "GUIHelpingJunction.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * member method definitions
 * ======================================================================= */
void
GUIHelpingJunction::fill(std::vector<GUIJunctionWrapper*> &list,
                         GUIGlObjectStorage &idStorage)
{
    size_t size = MSJunction::dictSize();
    list.reserve(size);
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        GUIJunctionWrapper *wrapper = (*i).second->buildJunctionWrapper(idStorage);
        if(wrapper!=0) {
            list.push_back(wrapper);
        }
    }
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
