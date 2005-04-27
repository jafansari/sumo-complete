#ifndef NIVissimSingleTypeParser_Fahrzeugtypdefinition_h
#define NIVissimSingleTypeParser_Fahrzeugtypdefinition_h
/***************************************************************************
                          NIVissimSingleTypeParser_Fahrzeugtypdefinition.h

                             -------------------
    begin                : Wed, 18 Dec 2002
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
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
// Revision 1.2  2005/04/27 12:24:38  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
//
// Revision 1.1  2003/02/07 11:08:42  dkrajzew
// Vissim import added (preview)
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#include <iostream>
#include "../NIVissimLoader.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class NIVissimSingleTypeParser_Fahrzeugtypdefinition
 *
 */
class NIVissimSingleTypeParser_Fahrzeugtypdefinition :
        public NIVissimLoader::VissimSingleTypeParser {
public:
    /// Constructor
    NIVissimSingleTypeParser_Fahrzeugtypdefinition(NIVissimLoader &parent,
        NIVissimLoader::ColorMap &colorMap);

    /// Destructor
    ~NIVissimSingleTypeParser_Fahrzeugtypdefinition();

    /// Parses the data type from the given stream
    bool parse(std::istream &from);

private:
    /// The color map to use
    NIVissimLoader::ColorMap &myColorMap;
};

/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
