#ifndef help_h
#define help_h
//---------------------------------------------------------------------------//
//                        netgen_help.h -
//  Help-screen of the gui version
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Wed, 16 Jul 2003
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
// Revision 1.7  2005/09/15 12:27:08  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2004/12/20 15:00:37  dkrajzew
// version patched
//
// Revision 1.5  2004/12/20 14:05:16  dkrajzew
// version patched
//
// Revision 1.4  2004/12/20 13:15:58  dkrajzew
// options output corrected
//
// Revision 1.3  2003/12/04 12:51:26  dkrajzew
// documentation added; possibility to use actuated and agentbased junctions
//  added; usage of street types patched
//
// Revision 1.2  2003/10/28 08:35:01  dkrajzew
// random number specification options added
//
// Revision 1.1  2003/07/16 15:41:17  dkrajzew
// network generator added
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


/* =========================================================================
 * definitions
 * ======================================================================= */
char *help[] =
{
    "Simulation of Urban MObility v0.8.2 - netgen application",
    "  Generates abstract networks.",
    "  http://sumo.sourceforge.net",
    "Usage: sumo-netgen [OPTION]*",
    " ",
    "Examples: ",
    "  netgen --grid-net [grid-network options] -o <OUTPUTFILE>",
    "  netgen --spider-net [spider-network opts] -o <OUTPUTFILE>",
    "  netgen --random-net [random-network opts] -o <OUTPUTFILE>",
    "  netgen -c <CONFIGURATION>",
    " ",
    " Either \"--grid-net\", \"--spider-net\" or \"--random-net\" must be",
    "  supplied. In dependance to these switches other options are used",
    " ",
    " Grid network options:",
    "  --grid-number INT              The number of junctions in both dirs",
    "  --grid-length FLOAT            The length of streets in both dirs",
    "  --grid-x-number INT            The number of junctions in x-dir",
    "                                  Overrides --grid-number",
    "  --grid-y-number INT            The number of junctions in y-dir",
    "                                  Overrides --grid-number",
    "  --grid-x-length FLOAT          The length of vertical streets",
    "                                  Overrides --grid-length",
    "  --grid-y-length FLOAT          The length of vertical streets",
    "                                  Overrides --grid-length",
    " ",
    " Spider network options:",
    "  --spider-arm-number INT        The number of axes within the net",
    "  --spider-circle-number INT     The number of circles of the net",
    "  --spider-space-rad FLOAT       The distances between the circles",
    " ",
    " Random network options:",
    "  --rand-max-distance FLOAT ",
    "  --rand-min-distance FLOAT",
    "  --rand-min-angle FLOAT",
    "  --rand-num-tries FLOAT",
    "  --rand-connectivity FLOAT",
    "  --rand-neighbor-dist1 FLOAT",
    "  --rand-neighbor-dist2 FLOAT",
    "  --rand-neighbor-dist3 FLOAT",
    "  --rand-neighbor-dist4 FLOAT",
    "  --rand-neighbor-dist5 FLOAT",
    "  --rand-neighbor-dist6 FLOAT",
    "  --rand-iterations INT",
    " ",
    " Output files:",
    "  -o, --output-file FILE         The generated net will be written to FILE",
    "  --map-output FILE              File to write joined edges information to",
    "  --plain-output FILE            Prefix of files to write node and edge",
    " ",
    " Setting Defaults:",
    "   -T, --type NAME                The default name for an edges type",
    "   -L, --lanenumber INT           The default number of lanes in an edge",
    "   -S, --speed DOUBLE             The default speed on an edge (in m/s)",
    "   -P, --priority INT             The default priority of an edge",
    "   -j, --default-junction-type [traffic_light|priority|actuated|agentbased]",
    "                                    Determines the type of the build junctions",
    " Processing Options:",
    "  -N, --capacity-norm FLOAT      The factor for flow to no. lanes conv.",
    "  -D, --min-decel FLOAT          The least vehicle deceleration value used",
    "  --flip-y                       Flips the y-coordinate along zero",
    "  --speed-in-km                  vmax is parsed as given in km/h; XML only",
    "  --use-laneno-as-priority       Uses the number of lanes priority hint",
    "  --omit-corrupt-edges           Continues parsing although a corrupt edge",
    "                                    occured (warning is printed)",
    "  --vissim-offset DOUBLE         Specifies the structure join offset within",
    "                                   vissim-import",
    "  --keep-small-tyellow           Given yellow times are kept even if too ",
    "                                   they are small",
    "  --traffic-light-green INT      Overide the default (20s) of a green phase",
    "                                   duration ",
    "  --edges-min-speed FLOAT        Specifies the minimum speed of edges in order",
    "                                  to be important",
    "  --guess-tls                    Turn on tls guessing",
    "  --guess-tls.min-incoming-speed FLOAT",
    "  --guess-tls.max-incoming-speed FLOAT",
    "  --guess-tls.min-outgoing-speed FLOAT",
    "  --guess-tls.max-outgoing-speed FLOAT",
    "                                 Min/max speeds that incoming/outgoing edges",
    "                                  must allowed in order to make their junction",
    "                                  tls-controlled.",
    "  --tls-guess.no-incoming-min INT",
    "  --tls-guess.no-incoming-max INT",
    "  --tls-guess.no-outgoing-min INT",
    "  --tls-guess.no-outgoing-max INT",
    "                                 Min/max of incoming/outgoing edges a junction",
    "                                  may have in order to be tls-controlled.",
    "  --explicite-tls JID[;JID]*     Specifies which junctions named by their ids",
    "                                  shall be tls-controlled."
    "  --explicite-no-tls JID[;JID]*  Specifies which junctions named by their ids",
    "                                  shall not be tls-controlled."
    " ",
    " ",
    " Random Number Options:",
    "  --srand INT                     Initialises the random number generator",
    "                                   with the given value",
    "  --abs-rand                      Set this when the current time shall be",
    "                                   used for random number initialisation",
    " ",
    " Report options:",
    "  -v, --verbose                   SUMO-NETGEN will report what it does",
    "  -W, --suppress-warnings         No warnings will be printed",
    "  -l, --log-file FILE             Writes all messages to the file",
    "  -p, --print-options             Prints option values before processing",
    "  -?, --help                      This screen",
    0
};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
//

