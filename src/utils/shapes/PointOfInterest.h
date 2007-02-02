/****************************************************************************/
/// @file    PointOfInterest.h
/// @author  Daniel Krajzewicz
/// @date    2005-09-15
/// @version $Id$
///
// missing_desc
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
#ifndef PointOfInterest_h
#define PointOfInterest_h
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
#include <utils/gfx/RGBColor.h>
#include <utils/geom/Position2D.h>
#include <utils/common/Named.h>

class PointOfInterest :
            public RGBColor, public Position2D, public Named
{
public:
    PointOfInterest(const std::string &id, const std::string &type,
                    const Position2D &p, const RGBColor &c)
            : RGBColor(c), Position2D(p), Named(id), myType(type)
    { }
    virtual ~PointOfInterest()
    { }

    const std::string &getType() const
    {
        return myType;
    }

protected:
    std::string myType;

};


#endif

/****************************************************************************/

