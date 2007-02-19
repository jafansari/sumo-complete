/****************************************************************************/
/// @file    MSRouteLoader.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 6 Nov 2002
/// @version $Id$
///
// A class that performs the loading of routes
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
#ifndef MSRouteLoader_h
#define MSRouteLoader_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <string>
#include <utils/common/FileErrorReporter.h>
#include <microsim/MSNet.h>
#include "MSVehicleContainer.h"
#include "MSRouteHandler.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRouteLoader
 */
class MSRouteLoader
{
public:
    /// constructor
    MSRouteLoader(MSNet &net,
                  MSRouteHandler *handler);

    /// destructor
    ~MSRouteLoader();

    /** loads vehicles until a vehicle is read that starts after
        the specified time */
    void loadUntil(SUMOTime time, MSVehicleContainer &into);

    /// resets the reader
    void init();

    /// returns the information whether new data is available
    bool moreAvailable() const;
private:
    /// the used SAX2XMLReader
    SAX2XMLReader* myParser;

    /// the token for saving the current position
    XMLPScanToken  myToken;

    /// information whether more vehicles should be available
    bool _moreAvailable;

    MSRouteHandler *myHandler;

};


#endif

/****************************************************************************/

