/***************************************************************************
    NIVissimSingleTypeParser_Fahrtverlaufdateien.cpp

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
// Revision 1.2  2003/07/22 15:11:25  dkrajzew
// removed warnings
//
// Revision 1.1  2003/05/20 09:42:37  dkrajzew
// all data types implemented
//
//

/* =========================================================================
 * included modules
 * ======================================================================= */
#include <iostream>
#include <utils/convert/TplConvert.h>
#include "../NIVissimLoader.h"
#include "../tempstructs/NIVissimSource.h"
#include "NIVissimSingleTypeParser_Fahrtverlaufdateien.h"


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
NIVissimSingleTypeParser_Fahrtverlaufdateien::NIVissimSingleTypeParser_Fahrtverlaufdateien(NIVissimLoader &parent)
	: NIVissimLoader::VissimSingleTypeParser(parent)
{
}


NIVissimSingleTypeParser_Fahrtverlaufdateien::~NIVissimSingleTypeParser_Fahrtverlaufdateien()
{
}


bool
NIVissimSingleTypeParser_Fahrtverlaufdateien::parse(std::istream &)
{
    return true;
}

