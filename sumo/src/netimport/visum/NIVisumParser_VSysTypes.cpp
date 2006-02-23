/***************************************************************************
                          NIVisumParser_VSysTypes.cpp
			  Parser for visum-modality types
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
// Revision 1.5  2006/02/23 11:23:53  dkrajzew
// VISION import added
//
// Revision 1.4  2005/10/07 11:41:01  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 12:03:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/04/27 12:24:42  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
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

#include <utils/common/UtilExceptions.h>
#include "NIVisumLoader.h"
#include "NIVisumParser_VSysTypes.h"

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
NIVisumParser_VSysTypes::NIVisumParser_VSysTypes(NIVisumLoader &parent,
        const std::string &dataName,
        NIVisumLoader::VSysTypeNames &vsystypes)
    : NIVisumLoader::NIVisumSingleDataTypeParser(parent, dataName),
    usedVSysTypes(vsystypes)
{
}


NIVisumParser_VSysTypes::~NIVisumParser_VSysTypes()
{
}


void
NIVisumParser_VSysTypes::myDependentReport()
{
    try {
        string name =
            myLineParser.know("VSysCode")
            ? myLineParser.get("VSysCode").c_str()
            : myLineParser.get("CODE").c_str();
        string type =
            myLineParser.know("VSysMode")
            ? myLineParser.get("VSysMode").c_str()
            : myLineParser.get("Typ").c_str();
        usedVSysTypes[name] = type;
    } catch (OutOfBoundsException) {
        addError2("VSYS", "", "OutOfBounds");
    } catch (NumberFormatException) {
        addError2("VSYS", "", "NumberFormat");
    } catch (UnknownElement) {
        addError2("VSYS", "", "UnknownElement");
    }
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


