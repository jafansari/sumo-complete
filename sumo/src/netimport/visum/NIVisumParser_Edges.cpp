/***************************************************************************
                          NIVisumParser_Edges.cpp
              Parser for visum-edges
                             -------------------
    project              : SUMO
    begin                : Thu, 14 Nov 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.12  2006/02/23 11:23:53  dkrajzew
// VISION import added
//
// Revision 1.11  2005/11/09 06:42:07  dkrajzew
// complete geometry building rework (unfinished)
//
// Revision 1.10  2005/10/07 11:41:01  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.9  2005/09/23 06:03:50  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.8  2005/09/15 12:03:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.7  2005/04/27 12:24:41  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
//
// Revision 1.6  2004/11/23 10:23:51  dkrajzew
// debugging
//
// Revision 1.5  2004/07/02 09:36:35  dkrajzew
// error on edges with no lanes patched
//
// Revision 1.4  2004/01/12 15:36:08  dkrajzew
// node-building classes are now lying in an own folder
//
// Revision 1.3  2003/11/11 08:19:46  dkrajzew
// made the code a little bit more pretty
//
// Revision 1.2  2003/05/20 09:39:14  dkrajzew
// Visum traffic light import added (by Markus Hartinger)
//
// Revision 1.1  2003/02/07 11:14:54  dkrajzew
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

#include <utils/common/TplConvertSec.h>
#include <netbuild/NBHelpers.h>
#include <netbuild/nodes/NBNodeCont.h>
#include <netbuild/NBTypeCont.h>
#include <utils/geom/GeomHelper.h>
#include "NIVisumLoader.h"
#include "NIVisumParser_Edges.h"

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
NIVisumParser_Edges::NIVisumParser_Edges(NIVisumLoader &parent,
        NBNodeCont &nc, NBEdgeCont &ec, NBTypeCont &tc,
		const std::string &dataName)
    : NIVisumLoader::NIVisumSingleDataTypeParser(parent, dataName),
    myNodeCont(nc), myEdgeCont(ec), myTypeCont(tc)
{
}


NIVisumParser_Edges::~NIVisumParser_Edges()
{
}


void
NIVisumParser_Edges::myDependentReport()
{
    string id;
    try {
        // get the id
        id = NBHelpers::normalIDRepresentation(myLineParser.get("Nr"));
        // get the from- & to-node and validate them
        string from_id =
            myLineParser.know("VonKnot")
            ? myLineParser.get("VonKnot")
            : myLineParser.get("VonKnotNr");
        NBNode *from = myNodeCont.retrieve(NBHelpers::normalIDRepresentation(from_id));
        string to_id =
            myLineParser.know("NachKnot")
            ? myLineParser.get("NachKnot")
            : myLineParser.get("NachKnotNr");
        NBNode *to = myNodeCont.retrieve(NBHelpers::normalIDRepresentation(to_id));
        if(!checkNodes(from, to)) {
            return;
        }
        // get the type
        string type =
            myLineParser.know("Typ")
            ? myLineParser.get("Typ")
            : myLineParser.get("TypNr");
        // get the street length
        SUMOReal length = getLength(from, to);
        // get the speed
        SUMOReal speed = getSpeed(type);
        // get the information whether the edge is a one-way
        bool oneway =
            myLineParser.know("Einbahn")
            ? TplConvert<char>::_2bool(myLineParser.get("Einbahn").c_str())
            : true;
        // get the number of lanes
        int nolanes = getNoLanes(type);
        // check whether the id is already used
        //  (should be the opposite direction)
        bool oneway_checked = oneway;
        NBEdge *previous = myEdgeCont.retrieve(id);
        if(previous!=0) {
            id = '-' + id;
            previous->setLaneSpreadFunction(NBEdge::LANESPREAD_RIGHT);
            oneway_checked = false;
        }
        if(find(myTouchedEdges.begin(), myTouchedEdges.end(), id)!=myTouchedEdges.end()) {
            oneway_checked = false;
        }
        string tmpid = '-' + id;
        if(find(myTouchedEdges.begin(), myTouchedEdges.end(), tmpid)!=myTouchedEdges.end()) {
            previous = myEdgeCont.retrieve(tmpid);
            if(previous!=0) {
                previous->setLaneSpreadFunction(NBEdge::LANESPREAD_RIGHT);
            }
            oneway_checked = false;
        }
        // add the edge
        int prio = myTypeCont.getPriority(type);
        if(nolanes!=0) {
            insertEdge(id, from, to, type, speed, nolanes, length, prio, oneway_checked);
        }
        myTouchedEdges.push_back(id);
        // nothing more to do, when the edge is a one-way street
        if(oneway) {
            return;
        }
        // add the opposite edge
        id = '-' + id;
        if(nolanes!=0) {
            insertEdge(id, to, from, type, speed, nolanes, length, prio, false);
        }
        myTouchedEdges.push_back(id);
    } catch (OutOfBoundsException) {
        addError2("STRECKE", id, "OutOfBounds");
    } catch (NumberFormatException) {
        addError2("STRECKE", id, "NumberFormat");
    } catch (UnknownElement) {
        addError2("STRECKE", id, "UnknownElement");
    }
}


bool
NIVisumParser_Edges::checkNodes(NBNode *from, NBNode *to) const
{
    if(from==0) {
        addError(" The from-node was not found within the net");
    }
    if(to==0) {
        addError(" The to-node was not found within the net");
    }
    if(from==to) {
        addError(" Both nodes are the same");
    }
    return from!=0&&to!=0&&from!=to;
}


SUMOReal
NIVisumParser_Edges::getLength(NBNode *from, NBNode *to) const
{
    SUMOReal length = 0;
    try {
        length = TplConvertSec<char>::_2SUMORealSec(
            myLineParser.get("Laenge").c_str(), 0);
    } catch (OutOfBoundsException) {
    }
    // compute when the street's length is not available
    if(length==0) {
        length = GeomHelper::distance(from->getPosition(), to->getPosition());
    }
    return length;
}


SUMOReal
NIVisumParser_Edges::getSpeed(const std::string &type) const
{
    SUMOReal speed = 0;
    try {
        speed = myLineParser.know("v0-IV")
            ? TplConvertSec<char>::_2SUMORealSec(myLineParser.get("v0-IV").c_str(), -1)
            : TplConvertSec<char>::_2SUMORealSec(myLineParser.get("V0IV").c_str(), -1);
    } catch (OutOfBoundsException) {
    }
    if(speed<=0) {
        speed = myTypeCont.getSpeed(type);
    } else {
        speed = speed / (SUMOReal) 3.6;
    }
    return speed;
}


int
NIVisumParser_Edges::getNoLanes(const std::string &type) const
{
    int nolanes = 0;
    try {
        nolanes = myLineParser.know("Fahrstreifen")
            ? TplConvertSec<char>::_2intSec(myLineParser.get("Fahrstreifen").c_str(), 0)
            : TplConvertSec<char>::_2intSec(myLineParser.get("ANZFAHRSTREIFEN").c_str(), 0);
    } catch (UnknownElement) {
        nolanes = myTypeCont.getNoLanes(type);
    }
    return nolanes;
}


void
NIVisumParser_Edges::insertEdge(const std::string &id,
                                NBNode *from, NBNode *to,
                                const std::string &type,
                                SUMOReal speed, int nolanes, SUMOReal length,
                                int prio, bool oneway) const
{
    NBEdge::LaneSpreadFunction lsf = oneway
        ? NBEdge::LANESPREAD_CENTER
        : NBEdge::LANESPREAD_RIGHT;
    NBEdge *e = new NBEdge(id, id, from, to, type, speed, nolanes, length, prio, lsf);
    if( !myEdgeCont.insert(e)) {
        delete e;
        addError(" Duplicate edge occured ('" + id + "').");
    }
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
