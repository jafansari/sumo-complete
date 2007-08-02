/****************************************************************************/
/// @file    OutputDevice_COUT.cpp
/// @author  Daniel Krajzewicz
/// @date    2004
/// @version $Id$
///
// An output device
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

#include <iostream>
#include "OutputDevice_COUT.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_COUT::OutputDevice_COUT()
{}


OutputDevice_COUT::~OutputDevice_COUT()
{}


std::ostream &
OutputDevice_COUT::getOStream()
{
    return cout;
}


void
OutputDevice_COUT::closeInfo()
{
    cout << endl;
}

/****************************************************************************/
