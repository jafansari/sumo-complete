/***************************************************************************
    NIVissimSingleTypeParser_Netzobjektdefinition.cpp

                             -------------------
    begin                : Wed, 30 Apr 2003
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.3  2005/09/23 06:02:58  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.2  2005/04/27 12:24:38  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
//
// Revision 1.1  2003/05/20 09:42:37  dkrajzew
// all data types implemented
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <iostream>
#include <utils/common/TplConvert.h>
#include "../NIVissimLoader.h"
#include "../tempstructs/NIVissimSource.h"
#include "NIVissimSingleTypeParser_Netzobjektdefinition.h"

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
NIVissimSingleTypeParser_Netzobjektdefinition::NIVissimSingleTypeParser_Netzobjektdefinition(NIVissimLoader &parent)
	: NIVissimLoader::VissimSingleTypeParser(parent)
{
}


NIVissimSingleTypeParser_Netzobjektdefinition::~NIVissimSingleTypeParser_Netzobjektdefinition()
{
}


bool
NIVissimSingleTypeParser_Netzobjektdefinition::parse(std::istream &from)
{
    return true;
}
/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
