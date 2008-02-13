/****************************************************************************/
/// @file    MSTriggeredReader.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The basic class for classes that read triggers
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
#ifndef MSTriggeredReader_h
#define MSTriggeredReader_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <microsim/MSNet.h>
#include <utils/common/Command.h>
#include <utils/common/FileErrorReporter.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSTriggeredReader
 * Superclass for structures that read from a file where the times the next
 * reading is performed are triggered by events,
 */
class MSTriggeredReader
{
public:
    /// Destructor
    virtual ~MSTriggeredReader();

    void init();
    bool isInitialised() const;
    SUMOTime wrappedExecute(SUMOTime current) throw(ProcessError);

protected:
    /// Constructor (for an abstract class)
    MSTriggeredReader(MSNet &net);

    /** @brief Processes the next event
        Returns true if the next element shall be read (a new timestemp is then available also)
        Returns false if the action could not be accomplished; this method is then called in the next step again */
    virtual bool processNextEntryReaderTriggered() = 0;

    /// Reads from the file
    virtual bool readNextTriggered() = 0;

    virtual void myInit() = 0;

    virtual void inputEndReached() = 0;

    SUMOTime warppedExecute(SUMOTime currentTime) throw(ProcessError);

protected:
    /// The offset to the next event
    SUMOTime myOffset;
    bool myWasInitialised;

private:
    /// invalidated default constructor
    MSTriggeredReader();

    /// invalidated copy constructor
    MSTriggeredReader(const MSTriggeredReader &s);

    /// invalidated assignment operator
    const MSTriggeredReader &operator=(const MSTriggeredReader &s);

};


#endif

/****************************************************************************/

