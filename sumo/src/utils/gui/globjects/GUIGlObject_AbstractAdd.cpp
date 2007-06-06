/****************************************************************************/
/// @file    GUIGlObject_AbstractAdd.cpp
/// @author  Daniel Krajzewicz
/// @date    2004
/// @version $Id$
///
// Base class for additional objects (emitter, detectors etc.)
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "GUIGlObject_AbstractAdd.h"
#include <cassert>
#include <iostream>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// static member definitions
// ===========================================================================
std::map<std::string, GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjects;
std::vector<GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjectList;


// ===========================================================================
// method definitions
// ===========================================================================
GUIGlObject_AbstractAdd::GUIGlObject_AbstractAdd(GUIGlObjectStorage &idStorage,
        std::string fullName,
        GUIGlObjectType type)
        : GUIGlObject(idStorage, fullName), myGlType(type)
{
//!!!    assert(myObjects.find(fullName)==myObjects.end());
    myObjects[fullName] = this;
    myObjectList.push_back(this);
}


GUIGlObject_AbstractAdd::GUIGlObject_AbstractAdd(GUIGlObjectStorage &idStorage,
        std::string fullName,
        size_t glID,
        GUIGlObjectType type)
        : GUIGlObject(idStorage, fullName, glID), myGlType(type)
{
//!!!    assert(myObjects.find(fullName)==myObjects.end());
    myObjects[fullName] = this;
    myObjectList.push_back(this);
}


GUIGlObject_AbstractAdd::~GUIGlObject_AbstractAdd()
{}



GUIGlObjectType
GUIGlObject_AbstractAdd::getType() const
{
    return myGlType;
}


void
GUIGlObject_AbstractAdd::clearDictionary()
{
    std::map<std::string, GUIGlObject_AbstractAdd*>::iterator i;
    for (i=myObjects.begin(); i!=myObjects.end(); i++) {
//!!!        delete (*i).second;
    }
    myObjects.clear();
    myObjectList.clear();
}


GUIGlObject_AbstractAdd *
GUIGlObject_AbstractAdd::get(const std::string &name)
{
    std::map<std::string, GUIGlObject_AbstractAdd*>::iterator i =
        myObjects.find(name);
    if (i==myObjects.end()) {
        return 0;
    }
    return (*i).second;
}


const std::vector<GUIGlObject_AbstractAdd*> &
GUIGlObject_AbstractAdd::getObjectList()
{
    return myObjectList;
}


std::vector<size_t>
GUIGlObject_AbstractAdd::getIDList()
{
    std::vector<size_t> ret;
    for (std::vector<GUIGlObject_AbstractAdd*>::iterator i=myObjectList.begin(); i!=myObjectList.end(); ++i) {
        ret.push_back((*i)->getGlID());
    }
    return ret;
}


Boundary
GUIGlObject_AbstractAdd::getCenteringBoundary() const
{
    Boundary b;
    b.add(getPosition());
    b.grow(10);
    return b;
}



/****************************************************************************/

