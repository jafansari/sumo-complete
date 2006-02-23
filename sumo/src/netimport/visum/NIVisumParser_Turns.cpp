/***************************************************************************
                          NIVisumParser_Turns.cpp
              Parser for visum-turn definitions
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
// Revision 1.7  2006/02/23 11:23:53  dkrajzew
// VISION import added
//
// Revision 1.6  2005/10/07 11:41:01  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.5  2005/09/15 12:03:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/04/27 12:24:42  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
//
// Revision 1.3  2004/11/23 10:23:51  dkrajzew
// debugging
//
// Revision 1.2  2004/01/12 15:36:08  dkrajzew
// node-building classes are now lying in an own folder
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

#include <netbuild/nodes/NBNode.h>
#include <netbuild/nodes/NBNodeCont.h>
#include <utils/common/UtilExceptions.h>
#include "NIVisumLoader.h"
#include "NIVisumParser_Turns.h"

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
NIVisumParser_Turns::NIVisumParser_Turns(NIVisumLoader &parent,
        NBNodeCont &nc,
        const std::string &dataName, NIVisumLoader::VSysTypeNames &vsystypes)
    : NIVisumLoader::NIVisumSingleDataTypeParser(parent, dataName),
    usedVSysTypes(vsystypes), myNodeCont(nc)
{
}


NIVisumParser_Turns::~NIVisumParser_Turns()
{
}


void
NIVisumParser_Turns::myDependentReport()
{
    try {
        // retrieve the nodes
        string from_node_row = myLineParser.know("VonKnot") ? "VonKnot" : "VonKnotNr";
        NBNode *from =
            myNodeCont.retrieve(NBHelpers::normalIDRepresentation(myLineParser.get(from_node_row)));

        string via_node_row = myLineParser.know("UeberKnot") ? "UeberKnot" : "UeberKnotNr";
        NBNode *via =
            myNodeCont.retrieve(NBHelpers::normalIDRepresentation(myLineParser.get(via_node_row)));

        string to_node_row = myLineParser.know("NachKnot") ? "NachKnot" : "NachKnotNr";
        NBNode *to =
            myNodeCont.retrieve(NBHelpers::normalIDRepresentation(myLineParser.get(to_node_row)));

        // check the nodes
        bool ok = checkNode(from, "from", from_node_row) &&
            checkNode(via, "via", via_node_row) &&
            checkNode(to, "to", to_node_row);
        if(!ok) {
            return;
        }
        // all nodes are known
        if(isVehicleTurning()) {
            // try to set the turning definition
            via->setTurningDefinition(from, to);
        }
    } catch (OutOfBoundsException) {
        addError2("ABBIEGEBEZIEHUNG", "", "OutOfBounds");
    } catch (NumberFormatException) {
        addError2("ABBIEGEBEZIEHUNG", "", "NumberFormat");
    } catch (UnknownElement) {
        addError2("ABBIEGEBEZIEHUNG", "", "UnknownElement");
    }
}



bool
NIVisumParser_Turns::checkNode(NBNode *node, const std::string &type,
                               const std::string &nodeTypeName)
{
    if(node==0) {
        addError("The " + type + "-node '" + NBHelpers::normalIDRepresentation(myLineParser.get(nodeTypeName)) + "' is not known inside the net.");
        return false;
    }
    return true;
}


bool
NIVisumParser_Turns::isVehicleTurning() {
    string type =
        myLineParser.know("VSysCode")
        ? myLineParser.get("VSysCode")
        : myLineParser.get("VSYSSET");
    return usedVSysTypes.find(type)!=usedVSysTypes.end() &&
        usedVSysTypes.find(type)->second=="IV";
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


