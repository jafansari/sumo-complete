/****************************************************************************/
/// @file    MSTriggeredXMLReader.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: $
///
// The basic class for classes that read XML-triggers
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
#ifndef MSTriggeredXMLReader_h
#define MSTriggeredXMLReader_h
// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <utils/sumoxml/SUMOSAXHandler.h>
#include "MSTriggeredReader.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * class MSTriggeredXMLReader
 * Base class for triggered file readers which work with XML-files
 */
class MSTriggeredXMLReader : public MSTriggeredReader,
            public SUMOSAXHandler
{
public:
    /// Destructor
    virtual ~MSTriggeredXMLReader();

protected:
    /// Constructor (for derived classes)
    MSTriggeredXMLReader(MSNet &net, const std::string &filename);

    /// reads from the XML-file (parses from file)
    bool readNextTriggered();

    virtual bool nextRead() = 0;

protected:
    void myInit();

protected:
    /// The used SAX-parser
    SAX2XMLReader* myParser;

    /// Position within the XML-file
    XMLPScanToken  myToken;

    bool myHaveMore;

private:
    /// invalidated default constructor
    MSTriggeredXMLReader();

    /// invalidated copy constructor
    MSTriggeredXMLReader(const MSTriggeredXMLReader &s);

    /// invalidated assignment operator
    const MSTriggeredXMLReader &operator=(const MSTriggeredXMLReader &s);

};


#endif

/****************************************************************************/

