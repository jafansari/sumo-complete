/****************************************************************************/
/// @file    GUIGlObject_AbstractAdd.cpp
/// @author  Daniel Krajzewicz
/// @date    2004
/// @version $Id$
///
// Base class for additional objects (detectors etc.)
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
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
#include <utils/gui/div/GLHelper.h>
#include <foreign/polyfonts/polyfonts.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// static member definitions
// ===========================================================================
std::map<std::string, GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjects;
std::vector<GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjectList;


// ===========================================================================
// method definitions
// ===========================================================================
GUIGlObject_AbstractAdd::GUIGlObject_AbstractAdd(const std::string& prefix, GUIGlObjectType type, const std::string& id) : 
    GUIGlObject(prefix, type, id) 
{
    myObjects[getFullName()] = this;
    myObjectList.push_back(this);
}


GUIGlObject_AbstractAdd::~GUIGlObject_AbstractAdd() throw() {}


void
GUIGlObject_AbstractAdd::clearDictionary() {
    std::map<std::string, GUIGlObject_AbstractAdd*>::iterator i;
    for (i=myObjects.begin(); i!=myObjects.end(); i++) {
//!!!        delete (*i).second;
    }
    myObjects.clear();
    myObjectList.clear();
}


GUIGlObject_AbstractAdd *
GUIGlObject_AbstractAdd::get(const std::string &name) {
    std::map<std::string, GUIGlObject_AbstractAdd*>::iterator i = myObjects.find(name);
    if (i==myObjects.end()) {
        return 0;
    }
    return (*i).second;
}


const std::vector<GUIGlObject_AbstractAdd*> &
GUIGlObject_AbstractAdd::getObjectList() {
    return myObjectList;
}


std::vector<GLuint>
GUIGlObject_AbstractAdd::getIDList() {
    std::vector<GLuint> ret;
    for (std::vector<GUIGlObject_AbstractAdd*>::iterator i=myObjectList.begin(); i!=myObjectList.end(); ++i) {
        ret.push_back((*i)->getGlID());
    }
    return ret;
}


void
GUIGlObject_AbstractAdd::drawGLName(const Position2D &p, const std::string &id, SUMOReal nameScale) const {
    glPolygonOffset(0, -6);
    glPushMatrix();
    glTranslated(p.x(), p.y(), 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    pfSetPosition(0, 0);
    pfSetScale(nameScale);
    SUMOReal w = pfdkGetStringWidth(id.c_str());
    glRotated(180, 1, 0, 0);
    glTranslated(-w/2., 0.4, 0);
    pfDrawString(id.c_str());
    glPopMatrix();
}


/****************************************************************************/

