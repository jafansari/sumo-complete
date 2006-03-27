#ifndef NBType_h
#define NBType_h
/***************************************************************************
                          NBType.h
			  A single edge type
                             -------------------
    project              : SUMO
    subproject           : netbuilder / netconverter
    begin                : Tue, 20 Nov 2001
    copyright            : (C) 2001 by DLR http://ivf.dlr.de/
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
// Revision 1.8  2006/03/27 07:27:13  dkrajzew
// edge types may now store the edge function
//
// Revision 1.7  2005/10/07 11:38:19  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.6  2005/09/23 06:01:06  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.5  2005/09/15 12:02:45  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.4  2005/04/27 11:48:26  dkrajzew
// level3 warnings removed; made containers non-static
//
// Revision 1.3  2003/09/05 15:16:57  dkrajzew
// umlaute conversion; node geometry computation; internal links computation
//
// Revision 1.2  2003/02/07 10:43:44  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:48:13  dkrajzew
// initial commit for net building classes
//
// Revision 1.4  2002/06/11 16:00:40  dkrajzew
// windows eol removed; template class definition inclusion depends now on the EXTERNAL_TEMPLATE_DEFINITION-definition
//
// Revision 1.3  2002/05/14 04:42:56  dkrajzew
// new computation flow
//
// Revision 1.2  2002/04/26 10:07:12  dkrajzew
// Windows eol removed; minor SUMOReal to int conversions removed;
//
// Revision 1.1.1.1  2002/04/09 14:18:27  dkrajzew
// new version-free project name (try2)
//
// Revision 1.1.1.1  2002/04/09 13:22:00  dkrajzew
// new version-free project name
//
// Revision 1.1.1.1  2002/02/19 15:33:04  traffic
// Initial import as a separate application.
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

#include <string>
#include "NBEdge.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
/**
 * NBType
 * This class holds all informations about a single type of the sections/edges
 */
class NBType {
public:
    /** parameterised constructor */
    NBType(const std::string &name, int noLanes, SUMOReal speed, int priority,
        NBEdge::EdgeBasicFunction function);

    /** destructor */
    ~NBType();

    /** the container may access the private members */
    friend class NBTypeCont;

private:
    /** the name of the type */
    std::string myName;

    /** the number of lanes of this type */
    int myNoLanes;

    /** the speed on a section/edge of this type */
    SUMOReal mySpeed;

    /** the priority of the edge/section of this type */
    int myPriority;

    /// The street's function
    NBEdge::EdgeBasicFunction myFunction;

};

/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
