#ifndef help_h
#define help_h
//---------------------------------------------------------------------------//
//                        sumo_help.h -
//  Help-screen of the simulation module
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
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
// Revision 1.23  2006/05/19 12:34:19  awegener
// --remote-port added
//
// Revision 1.22  2006/05/18 13:09:14  awegener
// *** empty log message ***
//
// Revision 1.21  2006/01/31 11:04:10  dkrajzew
// debugging
//
// Revision 1.20  2006/01/16 13:38:22  dkrajzew
// help and error handling patched
//
// Revision 1.19  2006/01/11 12:05:58  dkrajzew
// using the same text in man-pages and help
//
// Revision 1.18  2006/01/09 13:33:30  dkrajzew
// debugging error handling
//
// Revision 1.17  2005/10/07 11:48:01  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.16  2005/09/15 12:27:08  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.15  2005/02/17 10:33:29  dkrajzew
// code beautifying;
// Linux building patched;
// warnings removed;
// new configuration usage within guisim
//
// Revision 1.14  2004/12/20 15:00:37  dkrajzew
// version patched
//
// Revision 1.13  2004/12/20 14:05:16  dkrajzew
// version patched
//
// Revision 1.12  2004/12/20 13:15:58  dkrajzew
// options output corrected
//
// Revision 1.11  2004/12/20 10:48:35  dkrajzew
// net-files changed to net-file
//
// Revision 1.10  2004/07/02 09:51:38  dkrajzew
// emissions and trip-file included in the options list
//
// Revision 1.9  2004/01/26 07:13:51  dkrajzew
// added the possibility to place lsa-detectors at a default position/using a default length
//
// Revision 1.8  2003/12/12 12:32:13  dkrajzew
// continuing on accidents is now meant to be the default behaviour
//
// Revision 1.7  2003/10/28 08:35:01  dkrajzew
// random number specification options added
//
// Revision 1.6  2003/07/07 08:40:09  dkrajzew
// included new options into the help-screens
//
// Revision 1.5  2003/06/18 11:26:15  dkrajzew
// new message and error processing: output to user may be a message,
//  warning or an error now; it is reported to a Singleton (MsgHandler);
//  this handler puts it further to output instances.
//  changes: no verbose-parameter needed; messages are exported to singleton
//
// Revision 1.4  2003/03/31 06:18:18  dkrajzew
// help screen corrected
//
// Revision 1.3  2003/02/07 10:37:30  dkrajzew
// files updated
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
 * definitions
 * ======================================================================= */
/**
 * The list of help strings for the sumo (simulation) module
 */
char *help[] = {
    " A microscopic road traffic simulation.",
    " ",
    "Usage: sumo(.exe) [OPTION]*",
    " ",
    " ",
    "Examples: ",
    "  sumo -b 0 -e 1000 -n net.xml -r routes.xml -C",
    "  sumo -c munich_config.cfg",
    "  sumo --version",
    "  sumo --help",
    " ",
    "Options:",
    " Input files:",
    "   -n, --net-file FILE[;FILE]*     FILE is a network file",
    "   -r, --route-files FILE[;FILE]*  FILE is a route file",
    "   -a, --additional-files FILE...  FILE is a detector file",
    "   -c, --configuration FILE        FILE will be used as configuration",
    "                                    1. Default: ./sumo.cfg",
    " ",
    " Output files:",
    "   --emissions FILE                Writes statistics about vehicle emissions",
    "                                    to FILE",
    "   --tripinfo FILE                 Writes trip information for each vehicle",
    "                                    to FILE",
    "   --vehroutes FILE                Writes route information for each vehicle",
    "                                    to FILE",
    " ",
    "   --dump-basename PATH            PATH is the name and path prefix that",
    "                                    of edge-based net loads destination",
    "                                   To use in conjunction with --dump-intervals",
    "   --dump-intervals UINT[;UINT]*   UINT is a positive integer time interval",
    "                                    of edge-based net loads",
    "                                   To use in conjunction with --dump-basename",
    " ",
    "   --lanedump-basename PATH        PATH is the name and path prefix that",
    "                                    of lane-based net loads destination",
    "                                   To use in c. with --lanedump-intervals",
    "   --lanedump-intervals UINT...    UINT is a positive integer time interval",
    "                                    of lane-based net loads",
    "                                   To use in c. with --lanedump-basename",
    " ",
    "   --dump-begin UINT[;UINT]*       List of begin time steps dump data will",
    "                                    be written",
    "   --dump-end UINT[;UINT]*         List of end time steps dump data will",
    "                                    be written",
    " ",
    "   --netstate-dump  FILE           FILE is the raw output destination file",
    "   --dump-empty-edges              Forces SUMO to write edges in dump even",
    "                                    if they are empty.",
    " ",
    " Simulation timing:",
    "   -b, --begin INT                 First time step of the simulation",
    "   -e, --end INT                   Last time step of the simulation",
    "   -s, --route-steps INT           The number of steps to read routes in",
    "                                   forward (default=0: read all routes)",
    " Simulation options:",
    "   --quit-on-accident              Quits when an accident occures",
    "   --time-to-teleport              Specifies maximum waiting time before",
    "                                    a vehicle is being teleported",
    " ",
    " Traffic Lights Options:",
    "   --actuated-tl.detector-pos <FLOAT> The position of the detectors in front",
    "                                    of an actuated tls",
    "   --actuated-tl.max-gap <FLOAT>   The max gap an actuated tls shall use",
    "   --actuated-tl.detector-gap <FLOAT> unknown meaning",
    "   --actuated-tl.passing-time <FLOAT> The passing time used",
    " ",
    "   --agent-tl.detector-len <FLOAT> The length of the e2-detectors that ",
    "                                    compute the jam used by a agent based",
    "                                    traffic light logic in m",
    "   --agent-tl.learn-horizon <FLOAT> The learning horizon in s",
    "   --agent-tl.decision-horizon <FLOAT> The decision horizon",
    "   --agent-tl.min-diff <FLOAT>     The minimum difference in jam lengths",
    "                                   to make the tls react in m",
    "   --agent-tl.tcycle <FLOAT>       The cycle length of a tls in s",
    " ",
    " ",
    " Random Number Options:",
    "   --srand INT                 Initialises the random number generator",
    "                                 with the given value",
    "   --abs-rand                  Set this when the current time shall be",
    "                                 used for random number initialisation",
    " Report Options:",
    "   -v, --verbose                   SUMO will report what it does",
    "   -W, --suppress-warnings         No warnings will be printed",
    "   -l, --log-file FILE             Writes all messages to the file",
    "   -p, --print-options             Prints option values before processing",
    "   -?, --help                      This screen",
    "   --version                       Prints the program version",
    " ",
    " ",
    " RemoteServer Options:",
    "   --remote-port INT               Server port for remote controlled simulation runs",
    
    0
};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
//
