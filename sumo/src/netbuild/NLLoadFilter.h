#ifndef NLLoadFilter_h
#define NLLoadFilter_h
/***************************************************************************
                          NLDetectorBuilder.h
                          A building helper for the detectors
                             -------------------
    begin                : Mon, 15 Apr 2002
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
// Revision 1.4  2005/04/27 11:48:27  dkrajzew
// level3 warnings removed; made containers non-static
//
// Revision 1.3  2003/07/18 12:35:05  dkrajzew
// removed some warnings
//
// Revision 1.2  2003/02/07 10:43:44  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:48:13  dkrajzew
// initial commit for net building classes
//
// Revision 1.6  2002/07/22 12:44:32  dkrajzew
// Source loading structures added
//
// Revision 1.5  2002/06/11 14:39:25  dkrajzew
// windows eol removed
//
// Revision 1.4  2002/06/11 13:44:34  dkrajzew
// Windows eol removed
//
// Revision 1.3  2002/04/17 11:18:47  dkrajzew
// windows-newlines removed
//
// Revision 1.2  2002/04/16 06:50:20  dkrajzew
// documentation added; coding standard attachements added
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


/* =========================================================================
 * definitions
 * ======================================================================= */
/**
 * LoadFilter
 * The load filter is an information given to the handler to describe which
 * types of data shall be loaded.
 */
enum LoadFilter {
    /** load all known data */
    LOADFILTER_ALL = 255,
    /** load only the junction logics */
    LOADFILTER_LOGICS = 1,
    /** load detectors only */
    LOADFILTER_DETECTORS = 2,
    /** load only vehicles, their routes and theri types */
    LOADFILTER_DYNAMIC = 4,
    /** load the sources */
    LOADFILTER_SOURCES = 8,
    /** load static net elements only */
    LOADFILTER_NET = 1 | 2 | 8
};

/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
//
