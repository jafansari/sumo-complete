/****************************************************************************/
/// @file    PCTypeDefHandler.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id: $
///
// A handler for loading polygon type maps
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
#ifndef PCTypeDefHandler_h
#define PCTypeDefHandler_h
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

#include <string>
#include <utils/sumoxml/SUMOSAXHandler.h>
#include <utils/options/OptionsCont.h>
#include "PCTypeDefHandler.h"


// ===========================================================================
// class declarations
// ===========================================================================
class PCTypeToDef;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PCTypeDefHandler
 * @brief A handler for loading polygon type maps
 */
class PCTypeDefHandler : public SUMOSAXHandler
{
public:
    /// Constructor
    PCTypeDefHandler(OptionsCont &oc, PCTypeMap &con);

    /// Destructor
    virtual ~PCTypeDefHandler();


protected:
    /** the user-impemlented handler method for an opening tag */
    void myStartElement(int element, const std::string &name,
                        const Attributes &attrs);

    /** the user-implemented handler method for characters */
    void myCharacters(int element, const std::string &name,
                      const std::string &chars);

    /** the user-implemented handler method for a closing tag */
    void myEndElement(int element, const std::string &name);


protected:
    /// the options (program settings)
    OptionsCont &myOptions;

    /// The type map to fill
    PCTypeMap &myContainer;


private:
    /// invalidated copy constructor
    PCTypeDefHandler(const PCTypeDefHandler &src);

    /// invalidated assignment operator
    PCTypeDefHandler &operator=(const PCTypeDefHandler &src);

};


#endif

/****************************************************************************/

