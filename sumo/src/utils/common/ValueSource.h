/****************************************************************************/
/// @file    ValueSource.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 29.04.2005
/// @version $Id:ValueSource.h 4699 2007-11-09 14:05:13Z dkrajzew $
///
//
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
#ifndef ValueSource_h
#define ValueSource_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
template<typename T>
class ValueSource
{
public:
    ValueSource() { }
    virtual ~ValueSource() { }
    virtual T getValue() const = 0;
    virtual ValueSource *copy() const = 0;
    virtual ValueSource<SUMOReal> *makeSUMORealReturningCopy() const = 0;

};


#endif

/****************************************************************************/

