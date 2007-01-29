#ifndef MSRouteLoader_h
#define MSRouteLoader_h
/***************************************************************************
                          MSRouteLoader.h
              A class that performs the loading of routes
                             -------------------
    project              : SUMO
    begin                : Wed, 6 Nov 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
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
// $Log$
// Revision 1.9  2006/10/12 10:14:27  dkrajzew
// synchronized with internal CVS (mainly the documentation has changed)
//
// Revision 1.8  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/02/01 10:10:42  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.5  2004/07/02 09:26:24  dkrajzew
// classes prepared to be derived
//
// Revision 1.4  2004/01/26 07:47:55  dkrajzew
// just some small stlye changes
//
// Revision 1.3  2003/06/18 11:12:51  dkrajzew
// new message and error processing: output to user may be a message, warning or an error now; it is reported to a Singleton (MsgHandler); this handler puts it further to output instances. changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.2  2003/05/20 09:31:46  dkrajzew
// emission debugged; movement model reimplemented (seems ok); detector output debugged; setting and retrieval of some parameter added
//
// Revision 1.1  2003/02/07 10:41:51  dkrajzew
// updated
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


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

#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <string>
#include <utils/common/FileErrorReporter.h>
#include <microsim/MSNet.h>
#include "MSVehicleContainer.h"
#include "MSRouteHandler.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
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


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:

