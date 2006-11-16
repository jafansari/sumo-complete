//---------------------------------------------------------------------------//
//                        GUIGlObjectStorage.cpp -
//  A storage for retrival of displayed object using a numerical id
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.7  2006/11/16 10:50:52  dkrajzew
// warnings removed
//
// Revision 1.6  2006/07/06 05:51:39  dkrajzew
// debugged handling of vehicles that left the simulation
//
// Revision 1.5  2006/01/09 13:36:47  dkrajzew
// removed unneeded includes
//
// Revision 1.4  2005/10/07 11:45:32  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 12:19:44  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/05/04 09:19:51  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.1  2004/11/23 10:38:31  dkrajzew
// debugging
//
// Revision 1.1  2004/10/22 12:50:50  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.8  2004/07/02 08:36:10  dkrajzew
// false lock on unexisting ojbect retrieval patched
//
// Revision 1.7  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.6  2004/02/10 07:05:05  dkrajzew
// returning a value before releasing the lock-bug patched
//
// Revision 1.5  2003/11/18 14:28:14  dkrajzew
// debugged and completed lane merging detectors
//
// Revision 1.4  2003/06/05 11:37:30  dkrajzew
// class templates applied
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

#include <cassert>
#include "GUIGlObject.h"
#include <map>
#include "GUIGlObjectStorage.h"
#include <iostream>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
GUIGlObjectStorage::GUIGlObjectStorage()
    : myAktID(0)
{
}


GUIGlObjectStorage::~GUIGlObjectStorage()
{
}


void
GUIGlObjectStorage::registerObject(GUIGlObject *object)
{
    _lock.lock();
    object->setGlID(myAktID);
    myMap[myAktID++] = object;
    _lock.unlock();
}


void
GUIGlObjectStorage::registerObject(GUIGlObject *object, size_t id)
{
    _lock.lock();
    object->setGlID(id);
    myMap[id] = object;
    _lock.unlock();
}


size_t
GUIGlObjectStorage::getUniqueID()
{
    _lock.lock();
    size_t ret = myAktID++;
    _lock.unlock();
    return ret;
}


GUIGlObject *
GUIGlObjectStorage::getObjectBlocking(size_t id)
{
    _lock.lock();
    ObjectMap::iterator i=myMap.find(id);
    if(i==myMap.end()) {
        i = myBlocked.find(id);
        if(i!=myBlocked.end()) {
            GUIGlObject *o = (*i).second;
            _lock.unlock();
            return o;
        }
        _lock.unlock();
        return 0;
    }
    GUIGlObject *o = (*i).second;
    myMap.erase(id);
    myBlocked[id] = o;
    _lock.unlock();
    return o;
}


bool
GUIGlObjectStorage::remove(size_t id)
{
    _lock.lock();
    ObjectMap::iterator i=myMap.find(id);
    if(i==myMap.end()) {
        i = myBlocked.find(id);
        assert(i!=myBlocked.end());
        GUIGlObject *o = (*i).second;
        myBlocked.erase(id);
        my2Delete[id] = o;
        _lock.unlock();
		return false;
    } else {
        myMap.erase(id);
        _lock.unlock();
		return true;
    }
}


void
GUIGlObjectStorage::clear()
{
    _lock.lock();
    myMap.clear();
    myAktID = 0;
    _lock.unlock();
}


void
GUIGlObjectStorage::unblockObject(size_t id)
{
    _lock.lock();
    ObjectMap::iterator i=myBlocked.find(id);
    if(i==myBlocked.end()) {
        _lock.unlock();
        return;
    }
    GUIGlObject *o = (*i).second;
    myBlocked.erase(id);
    myMap[id] = o;
    _lock.unlock();
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


