#ifndef LoggedValue_Single_h
#define LoggedValue_Single_h
//---------------------------------------------------------------------------//
//                        LoggedValue_Single.h -
//  Logger over a single time step
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Thu, 20 Feb 2003
//  copyright            : (C) 2003 by Daniel Krajzewicz
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
// $Log$
// Revision 1.8  2006/11/02 13:15:35  behrisch
// Template inheritance needs explicit member reference (this->)
//
// Revision 1.7  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/15 11:07:54  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/05/04 08:09:23  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.4  2004/03/19 13:06:44  dkrajzew
// some work on the style
//
// Revision 1.3  2004/03/01 10:49:51  roessel
// Reintroduced formerly removed files.
//
// Revision 1.1  2003/11/11 08:07:37  dkrajzew
// logging (value passing) moved from utils to microsim
//
// Revision 1.3  2003/04/09 15:36:18  dkrajzew
// debugging of emitters: forgotten release of vehicles (gui) debugged; forgotten initialisation of logger-members debuggt; error managament corrected
//
// Revision 1.2  2003/03/18 13:16:57  dkrajzew
// windows eol removed
//
// Revision 1.1  2003/03/06 17:22:15  dkrajzew
// initial commit for value logging cleasses
//
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

#include "LoggedValue.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class LoggedValue_Single
 * This class simply stores a value and allows its retrieval
 */
template<typename _T>
class LoggedValue_Single
    : public LoggedValue<_T> {
public:
    /// Constructor
    LoggedValue_Single(size_t dummy)
        : LoggedValue<_T>(dummy)
    {
    }

    /// Destructor
    ~LoggedValue_Single() { }


    /** @brief Adds a new value
        Simply sets the current value */
    void add(_T value)
    {
        this->myCurrentValue = value;
    }


    /** returns the average of previously set values
        (for and over the given sample interval) */
    _T getAvg() const
    {
        return this->myCurrentValue;
    }


    /** returns the sum of previously set values
        (for the given sample interval) */
    _T getAbs() const
    {
        return this->myCurrentValue;
    }


};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

