/***************************************************************************
                          main.cpp
              The main procedure for the build of person/vehicle routes
                             -------------------
    project              : SUMO
    subproject           : router
    begin                : Thu, 06 Jun 2002
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
namespace
{
    const char rcsid[] =
        "$Id$";
}
// $Log: duarouter_main.cpp,v $
// Revision 1.16  2006/11/29 07:51:22  dkrajzew
// added the possibility to use the loaded weights outside their boundaries
//
// Revision 1.15  2006/11/20 11:11:33  dkrajzew
// bug [ 1598346 ] (Versioning information in many places) patched - Version number is now read from windows_config.h/config.h
//
// Revision 1.14  2006/11/14 06:53:11  dkrajzew
// readapting changes in the router-API
//
// Revision 1.13  2006/08/01 07:19:56  dkrajzew
// removed build number information
//
// Revision 1.12  2006/02/13 07:35:16  dkrajzew
// made dijkstra-router checking for closures optionally
//
// Revision 1.11  2006/01/26 08:54:44  dkrajzew
// adapted the new router API
//
// Revision 1.10  2006/01/16 13:21:28  dkrajzew
// computation of detector types validated for the 'messstrecke'-scenario
//
// Revision 1.9  2006/01/09 13:33:30  dkrajzew
// debugging error handling
//
// Revision 1.8  2005/11/30 08:56:49  dkrajzew
// final try/catch is now only used in the release version
//
// Revision 1.7  2005/10/17 09:27:46  dkrajzew
// got rid of the old MSVC memory leak checker
//
// Revision 1.6  2005/10/07 11:48:00  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.5  2005/09/23 06:13:19  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.4  2005/09/15 12:27:08  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2005/07/12 12:52:55  dkrajzew
// build number output added
//
// Revision 1.2  2004/11/23 10:43:28  dkrajzew
// debugging
//
// Revision 1.1  2004/08/02 13:03:19  dkrajzew
// applied better names
//
// Revision 1.6  2004/07/02 09:49:36  dkrajzew
// generalised for easier online-router implementation; debugging
//
// Revision 1.5  2004/04/14 13:53:49  roessel
// Changes and additions in order to implement supplementary-weights.
//
// Revision 1.4  2004/04/02 11:30:35  dkrajzew
// moving the vehicle forward if it shall start at a too short edge added;
//  output of the number of loaded, build, and discarded
//
// Revision 1.3  2004/02/06 08:54:28  dkrajzew
// _INC_MALLOC definition removed (does not work on MSVC7.0)
//
// Revision 1.2  2004/01/27 08:46:15  dkrajzew
// shared input possibilities between both routers
//
// Revision 1.1  2004/01/26 07:12:12  dkrajzew
// now two routers are available - the dua- and the jp-router
//
// Revision 1.27  2003/10/28 08:35:01  dkrajzew
// random number specification options added
//
// Revision 1.26  2003/10/17 07:16:44  dkrajzew
// errors patched
//
// Revision 1.25  2003/10/17 06:53:08  dkrajzew
// added the possibility to change the standard-krauss parameter via the
//  command line
//
// Revision 1.24  2003/08/21 13:01:39  dkrajzew
// some bugs patched
//
// Revision 1.23  2003/07/22 15:17:15  dkrajzew
// documentation
//
// Revision 1.22  2003/07/07 08:42:26  dkrajzew
// no configuration is loaded as default any more
//
// Revision 1.21  2003/06/24 14:38:46  dkrajzew
// false instantiation of option "log-file" as Option_String patched into
//  Option_FileName patched
//
// Revision 1.20  2003/06/24 08:50:00  dkrajzew
// some more sophisticated default values for Gawrons dua inserted
//
// Revision 1.19  2003/06/24 08:06:36  dkrajzew
// implemented SystemFrame and applied the changes to all applications
//
// Revision 1.18  2003/06/19 11:03:57  dkrajzew
// debugging
//
// Revision 1.17  2003/06/19 07:07:52  dkrajzew
// false order of calling XML- and Options-subsystems patched
//
// Revision 1.16  2003/06/18 11:26:15  dkrajzew
// new message and error processing: output to user may be a message, warning
//  or an error now; it is reported to a Singleton (MsgHandler);
// this handler puts it further to output instances.
// changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.15  2003/05/20 09:54:45  dkrajzew
// configuration files are no longer set as default
//
// Revision 1.14  2003/04/10 16:13:52  dkrajzew
// recent changes
//
// Revision 1.13  2003/04/09 15:45:30  dkrajzew
// router debugging & extension: no routing over sources, random routes added
//
// Revision 1.11  2003/04/02 11:48:43  dkrajzew
// debug statements removed
//
// Revision 1.10  2003/03/31 06:18:17  dkrajzew
// help screen corrected
//
// Revision 1.9  2003/03/18 13:06:19  dkrajzew
// windows eol removed
//
// Revision 1.8  2003/03/12 16:34:34  dkrajzew
// some style guides applied
//
// Revision 1.7  2003/03/03 15:08:20  dkrajzew
// debugging
//
// Revision 1.6  2003/02/07 10:37:30  dkrajzew
// files updated
//
// Revision 1.4  2002/10/22 10:02:47  dkrajzew
// minor warnings removed
//
// Revision 1.3  2002/10/21 10:01:03  dkrajzew
// routedefs renamed to tripdefs in options
//
// Revision 1.2  2002/10/18 11:35:53  dkrajzew
// postinitialisation of edges for computation of lane-independent value added
//
// Revision 1.1  2002/10/16 14:51:08  dkrajzew
// Moved from ROOT/sumo to ROOT/src; added further help and main files for
//  netconvert, router, od2trips and gui version
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

#include <sax/SAXException.hpp>
#include <sax/SAXParseException.hpp>
#include <utils/common/TplConvert.h>
#include <iostream>
#include <string>
#include <fstream>
#include <limits.h>
#include <ctime>
#include <router/ROLoader.h>
#include <router/ROEdgeVector.h>
#include <router/RONet.h>
#include <router/ROVehicleType_Krauss.h>
#include <utils/helpers/SUMODijkstraRouter.h>
#include <routing_dua/RODUAEdgeBuilder.h>
#include <router/ROFrame.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <routing_dua/RODUAFrame.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * functions
 * ======================================================================= */
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
/**
 * loads the net
 * The net is in this meaning made up by the net itself and the dynamic
 * weights which may be supplied in a separate file
 */
RONet *
loadNet(ROLoader &loader, OptionsCont &oc)
{
    // load the net
    RODUAEdgeBuilder builder(oc.getBool("expand-weights"));
    RONet *net = loader.loadNet(builder);
    if(net==0) {
        throw ProcessError();
    }
    // load the weights when wished/available
    if(oc.isSet("w")) {
        loader.loadWeights(*net, oc.getString("w"), false);
    }
    if(oc.isSet("lane-weights")) {
        loader.loadWeights(*net, oc.getString("lane-weights"), true);
    }
    if ( oc.isSet( "S" ) ) {
        loader.loadSupplementaryWeights( *net );
    }
    return net;
}



/**
 * Computes the routes saving them
 */
void
startComputation(RONet &net, ROLoader &loader, OptionsCont &oc)
{
    SUMOAbstractRouter<ROEdge, ROVehicle> *router;
    if(net.hasRestrictions()) {
        router = new SUMODijkstraRouter<ROEdge, ROVehicle, prohibited_withRestrictions<ROEdge, ROVehicle>, ROEdge>(
            net.getEdgeNo(), oc.getBool("continue-on-unbuild"), &ROEdge::getEffort);
    } else {
        router = new SUMODijkstraRouter<ROEdge, ROVehicle, prohibited_noRestrictions<ROEdge, ROVehicle>, ROEdge>(
		    net.getEdgeNo(), oc.getBool("continue-on-unbuild"), &ROEdge::getEffort);
    }
    // build the router
    // initialise the loader
    size_t noLoaders =
        loader.openRoutes(net, oc.getFloat("gBeta"), oc.getFloat("gA"));
    if(noLoaders==0) {
        MsgHandler::getErrorInstance()->inform("No route input specified.");
        throw ProcessError();
    }
    // prepare the output
    net.openOutput(oc.getString("output"), true);
    // the routes are sorted - process stepwise
    if(!oc.getBool("unsorted")) {
        loader.processRoutesStepWise(oc.getInt("b"), oc.getInt("e"), net, *router);
    }
    // the routes are not sorted: load all and process
    else {
        loader.processAllRoutes(oc.getInt("b"), oc.getInt("e"), net, *router);
    }
    // end the processing
    loader.closeReading();
    net.closeOutput();
    delete router;
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    int ret = 0;
    RONet *net = 0;
    ROLoader *loader = 0;
#ifndef _DEBUG
    try {
#endif
        // initialise the application system (messaging, xml, options)
        int init_ret =
            SystemFrame::init(false, argc, argv, RODUAFrame::fillOptions);
        if(init_ret<0) {
            cout << "SUMO duarouter" << endl;
            cout << " (c) DLR/ZAIK 2000-2007; http://sumo.sourceforge.net" << endl;
            cout << " Version " << VERSION << endl;
            switch(init_ret) {
            case -2:
                OptionsSubSys::getOptions().printHelp(cout);
                break;
            default:
                cout << " Use --help to get the list of options." << endl;
                SystemFrame::close();
                return -1;
            }
            SystemFrame::close();
            return 0;
        } else if(init_ret!=0||!RODUAFrame::checkOptions(OptionsSubSys::getOptions())) {
            throw ProcessError();
        }
        // retrieve the options
        OptionsCont &oc = OptionsSubSys::getOptions();
		ROFrame::setDefaults(oc);
        // load data
        ROVehicleBuilder vb;
        loader = new ROLoader(oc, vb, false);
        net = loadNet(*loader, oc);
        if(net!=0) {
            // build routes
            try {
                startComputation(*net, *loader, oc);
            } catch (SAXParseException &e) {
                MsgHandler::getErrorInstance()->inform(
                    toString<int>(e.getLineNumber()));
                ret = 1;
            } catch (SAXException &e) {
                MsgHandler::getErrorInstance()->inform(
                    TplConvert<XMLCh>::_2str(e.getMessage()));
                ret = 1;
            }
        } else {
            ret = 1;
        }
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    }
#endif
    delete net;
    delete loader;
    SystemFrame::close();
    if(ret==0) {
        cout << "Success." << endl;
    }
    return ret;
}

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

