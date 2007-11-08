/****************************************************************************/
/// @file    ODDistrict.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A district (origin/destination)
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

#include <vector>
#include <string>
#include <utility>
#include <utils/common/UtilExceptions.h>
#include <utils/common/Named.h>
#include <utils/common/MsgHandler.h>
#include "ODDistrict.h"

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
ODDistrict::ODDistrict(const std::string &id) throw()
        : Named(id)
{}


ODDistrict::~ODDistrict() throw()
{}


void
ODDistrict::addSource(const std::string &id, SUMOReal weight) throw()
{
    mySources.add(weight, id);
}


void
ODDistrict::addSink(const std::string &id, SUMOReal weight) throw()
{
    mySinks.add(weight, id);
}


std::string
ODDistrict::getRandomSource() const throw(OutOfBoundsException)
{
    return mySources.get();
}


std::string
ODDistrict::getRandomSink() const throw(OutOfBoundsException)
{
    return mySinks.get();
}


unsigned int
ODDistrict::sinkNumber() const
{
    return mySinks.getVals().size();
}


unsigned int
ODDistrict::sourceNumber() const
{
    return mySources.getVals().size();
}


/****************************************************************************/

