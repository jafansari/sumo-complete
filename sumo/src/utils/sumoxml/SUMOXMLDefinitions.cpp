//---------------------------------------------------------------------------//
//                        SUMOXMLDefinitions.cpp -
//  Definitions of SUMO-tags
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
// Revision 1.5  2003/03/03 15:27:00  dkrajzew
// period attribute for router added
//
// Revision 1.4  2003/02/07 10:53:23  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <utils/xml/GenericSAX2Handler.h>
#include <utils/xml/AttributesHandler.h>
#include "SUMOXMLDefinitions.h"

/* =========================================================================
 * definitions
 * ======================================================================= */
size_t noSumoTags = 38;

GenericSAX2Handler::Tag sumotags[38] =
{
    /* 00 */  { "simulation",       SUMO_TAG_SIMULATION },
    /* 01 */  { "edge",             SUMO_TAG_EDGE },
    /* 02 */  { "lane",             SUMO_TAG_LANE },
    /* 03 */  { "lanes",            SUMO_TAG_LANES },
    /* 04 */  { "cedge",            SUMO_TAG_CEDGE },
    /* 05 */  { "junction",         SUMO_TAG_JUNCTION },
    /* 06 */  { "inlanes",          SUMO_TAG_INLANES },
    /* 07 */  { "detector",         SUMO_TAG_DETECTOR },
    /* 08 */  { "vehicle",          SUMO_TAG_VEHICLE },
    /* 09 */  { "vtype",            SUMO_TAG_VTYPE },
    /* 10 */  { "route",            SUMO_TAG_ROUTE },
    /* 11 */  { "succ",             SUMO_TAG_SUCC },
    /* 12 */  { "succlane",         SUMO_TAG_SUCCLANE },
    /* 13 */  { "key",              SUMO_TAG_KEY },
    /* 14 */  { "junctionlogic",    SUMO_TAG_JUNCTIONLOGIC },
    /* 15 */  { "requestsize",      SUMO_TAG_REQUESTSIZE },
    /* 16 */  { "responsesize",     SUMO_TAG_RESPONSESIZE },
    /* 17 */  { "lanenumber",       SUMO_TAG_LANENUMBER },
    /* 18 */  { "logicitem",        SUMO_TAG_LOGICITEM },
    /* 19 */  { "trafoitem",        SUMO_TAG_TRAFOITEM },
    /* 20 */  { "row-logic",        SUMO_TAG_ROWLOGIC },
    /* 21 */  { "source",           SUMO_TAG_SOURCE },
    /* 22 */  { "district",         SUMO_TAG_DISTRICT },
    /* 23 */  { "dsource",          SUMO_TAG_DSOURCE },
    /* 24 */  { "dsink",            SUMO_TAG_DSINK },
    /* 25 */  { "edges",            SUMO_TAG_EDGES },
    /* 26 */  { "node_count",       SUMO_TAG_NODECOUNT },
    /* 27 */  { "tl-logic",         SUMO_TAG_TLLOGIC },
    /* 28 */  { "offset",           SUMO_TAG_OFFSET },
    /* 29 */  { "initstep",         SUMO_TAG_INITSTEP },
    /* 30 */  { "phaseno",          SUMO_TAG_PHASENO },
    /* 31 */  { "phase",            SUMO_TAG_PHASE },
    /* 32 */  { "edgepos",          SUMO_TAG_EDGEPOS },
    /* 33 */  { "routealt",         SUMO_TAG_ROUTEALT },
    /* 34 */  { "tripdef",          SUMO_TAG_TRIPDEF },
    /* 36 */  { "logicno",          SUMO_TAG_LOGICNO },
    /* 37 */  { "trigger",          SUMO_TAG_TRIGGER },
    /* 38 */  { "step",             SUMO_TAG_STEP }
};

size_t noSumoAttrs = 53;

AttributesHandler::Attr sumoattrs[53] =
{
    /* 00 */ { "id",            SUMO_ATTR_ID },
    /* 01 */ { "name",          SUMO_ATTR_NAME },
    /* 02 */ { "type",          SUMO_ATTR_TYPE },
    /* 03 */ { "priority",      SUMO_ATTR_PRIORITY },
    /* 04 */ { "nolanes",       SUMO_ATTR_NOLANES },
    /* 05 */ { "speed",         SUMO_ATTR_SPEED },
    /* 06 */ { "length",        SUMO_ATTR_LENGTH },
    /* 07 */ { "fromnode",      SUMO_ATTR_FROMNODE },
    /* 08 */ { "tonode",        SUMO_ATTR_TONODE },
    /* 09 */ { "xfrom",         SUMO_ATTR_XFROM },
    /* 10 */ { "yfrom",         SUMO_ATTR_YFROM },
    /* 11 */ { "xto",           SUMO_ATTR_XTO },
    /* 12 */ { "yto",           SUMO_ATTR_YTO },
    /* 13 */ { "x",             SUMO_ATTR_X },
    /* 14 */ { "y",             SUMO_ATTR_Y },
    /* 15 */ { "key",           SUMO_ATTR_KEY },
    /* 16 */ { "weight",        SUMO_ATTR_WEIGHT },
    /* 17 */ { "depart",        SUMO_ATTR_DEPART },
    /* 18 */ { "route",         SUMO_ATTR_ROUTE },
    /* 19 */ { "maxspeed",      SUMO_ATTR_MAXSPEED },
    /* 20 */ { "accel",         SUMO_ATTR_ACCEL },
    /* 21 */ { "decel",         SUMO_ATTR_DECEL },
    /* 22 */ { "sigma",         SUMO_ATTR_SIGMA },
    /* 23 */ { "last",          SUMO_ATTR_LAST },
    /* 24 */ { "cost",          SUMO_ATTR_COST },
    /* 25 */ { "propability",   SUMO_ATTR_PROP },
    /* 26 */ { "pos",           SUMO_ATTR_POS },
    /* 27 */ { "lane",          SUMO_ATTR_LANE },
    /* 28 */ { "from",          SUMO_ATTR_FROM },
    /* 29 */ { "to",            SUMO_ATTR_TO },
    /* 30 */ { "function",      SUMO_ATTR_FUNC },
    /* 31 */ { "changeurge",    SUMO_ATTR_CHANGEURGE },
    /* 32 */ { "request",       SUMO_ATTR_REQUEST },
    /* 33 */ { "response",      SUMO_ATTR_RESPONSE },
    /* 34 */ { "pos",           SUMO_ATTR_POSITION },
    /* 35 */ { "freq",          SUMO_ATTR_SPLINTERVAL },
    /* 36 */ { "style",         SUMO_ATTR_STYLE },
    /* 37 */ { "file",          SUMO_ATTR_FILE },
    /* 38 */ { "junction",      SUMO_ATTR_JUNCTION },
    /* 39 */ { "yield",         SUMO_ATTR_YIELD },
    /* 40 */ { "no",            SUMO_ATTR_NO },
    /* 41 */ { "phase",         SUMO_ATTR_PHASE },
    /* 42 */ { "brake",         SUMO_ATTR_BRAKE },
    /* 43 */ { "duration",      SUMO_ATTR_DURATION },
    /* 44 */ { "objecttype",    SUMO_ATTR_OBJECTTYPE },
    /* 45 */ { "attr",          SUMO_ATTR_ATTR },
    /* 46 */ { "objectid",      SUMO_ATTR_OBJECTID },
    /* 47 */ { "time",          SUMO_ATTR_TIME },
    /* 48 */ { "multi_ref",     SUMO_ATTR_MULTIR },
    /* 49 */ { "traveltime",    SUMO_ATTR_VALUE },
    /* 50 */ { "begin",         SUMO_ATTR_BEGIN },
    /* 51 */ { "end",           SUMO_ATTR_END },
    /* 51 */ { "period",        SUMO_ATTR_PERIOD }
};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "SUMOXMLDefinitions.icc"
//#endif

// Local Variables:
// mode:C++
// End:


