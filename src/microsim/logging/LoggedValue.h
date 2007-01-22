#ifndef LoggedValue_h
#define LoggedValue_h
//---------------------------------------------------------------------------//
//                        LoggedValue.h -
//  An abstract (virtual) class for value logging
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
// Revision 1.5  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.4  2005/05/04 08:09:23  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.3  2004/03/01 10:49:51  roessel
// Reintroduced formerly removed files.
//
// Revision 1.1  2003/11/11 08:07:37  dkrajzew
// logging (value passing) moved from utils to microsim
//
// Revision 1.4  2003/06/05 14:29:39  dkrajzew
// class templates applied; documentation added
//
// Revision 1.3  2003/04/09 15:36:17  dkrajzew
// debugging of emitters: forgotten release of vehicles (gui) debugged; forgotten initialisation of logger-members debuggt; error managament corrected
//
// Revision 1.2  2003/03/18 13:16:57  dkrajzew
// windows eol removed
//
// Revision 1.1  2003/03/06 17:22:14  dkrajzew
// initial commit for value logging cleasses
//
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


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class LoggedValue
 * As detectors may store values over a SUMORealing or a fixed interval and
 * a sample interval of one does not require any kind of remembering
 * previous values, we implemented this class to make a common access to
 * all those mentioned cases.
 */
template<typename _T>
class LoggedValue {
public:
    /** @brief Constructor
        The dummy value is used to ensure, the derived class must
        remind the sample interval */
    LoggedValue(size_t dummy)
        : myCurrentValue(0) { }

    /// Destructor
    virtual ~LoggedValue() { }

    /// Adds a new value
    virtual void add(_T value) = 0;

    /** returns the average of previously set values
        (for and over the given sample interval) */
    virtual _T getAvg() const = 0;

    /** returns the sum of previously set values
        (for the given sample interval) */
    virtual _T getAbs() const = 0;

protected:
    /// The current value
    _T      myCurrentValue;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

