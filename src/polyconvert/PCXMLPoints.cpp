/****************************************************************************/
/// @file    PCXMLPoints.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 02.11.2006
/// @version $Id$
///
// A reader of pois stored in XML-format
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

#include <string>
#include <map>
#include <fstream>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/importio/LineReader.h>
#include <utils/common/StdDefs.h>
#include <utils/gfx/GfxConvHelper.h>
#include <polyconvert/PCPolyContainer.h>
#include "PCXMLPoints.h"
#include <utils/gfx/RGBColor.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position2D.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/xml/XMLSubSys.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method defintions
// ===========================================================================
PCXMLPoints::PCXMLPoints(PCPolyContainer &toFill,
                         PCTypeMap &tm, OptionsCont &oc)
        : SUMOSAXHandler("xml-poi-definition"),
        myCont(toFill), myTypeMap(tm), myOptions(oc)
{}


PCXMLPoints::~PCXMLPoints() throw()
{}


void
PCXMLPoints::load(OptionsCont &oc)
{
    if(!XMLSubSys::runParser(*this, oc.getString("xml-points"))) {
        throw ProcessError();
    }
}


void
PCXMLPoints::myStartElement(SumoXMLTag element, const std::string &name,
                            const Attributes &attrs) throw()
{
    if (name!="poi") {
        return;
    }
    string id = getStringSecure(attrs, SUMO_ATTR_ID, "");
    string type = getStringSecure(attrs, SUMO_ATTR_TYPE, "");
    SUMOReal x = getFloatSecure(attrs, SUMO_ATTR_X, -1);
    SUMOReal y = getFloatSecure(attrs, SUMO_ATTR_Y, -1);
    x *= 100000.0;
    y *= 100000.0;
    Position2D pos(y, x); // !!! reverse!
    GeoConvHelper::x2cartesian(pos);
    // check the poi
    if (id=="") {
        MsgHandler::getErrorInstance()->inform("The name of a poi is missing.");
        return;
    }
    // patch the values
    bool discard = false;
    int layer = myOptions.getInt("layer");
    RGBColor color;
    if (myTypeMap.has(type)) {
        const PCTypeMap::TypeDef &def = myTypeMap.get(type);
        id = def.prefix + id;
        type = def.id;
        color = GfxConvHelper::parseColor(def.color);
        discard = def.discard;
        layer = def.layer;
    } else {
        id = myOptions.getString("prefix") + id;
        type = myOptions.getString("type");
        color = GfxConvHelper::parseColor(myOptions.getString("color"));
    }
    if (!discard) {
        bool ignorePrunning = false;
        if (OptionsSubSys::helper_CSVOptionMatches("prune.ignore", id)) {
            ignorePrunning = true;
        }
        PointOfInterest *poi = new PointOfInterest(id, type, pos, color);
        myCont.insert(id, poi, layer, ignorePrunning);
    }
}


void
PCXMLPoints::myCharacters(SumoXMLTag , const std::string &,
                          const std::string &) throw()
{}


void
PCXMLPoints::myEndElement(SumoXMLTag , const std::string &) throw()
{}



/****************************************************************************/

