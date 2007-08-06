/****************************************************************************/
/// @file    RODFDetectorHandler.h
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// missing_desc
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
#ifndef RODFDetectorHandler_h
#define RODFDetectorHandler_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/options/OptionsCont.h>
#include "RODFDetector.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFDetectorHandler
 * @brief SAX2-Handler for loading DFROUTER-detector definitions
 */
class RODFDetectorHandler : public SUMOSAXHandler
{
public:
    /// Constructor
    RODFDetectorHandler(OptionsCont &oc, RODFDetectorCon &con,
                      const std::string &file);

    /// Destrcutor
    virtual ~RODFDetectorHandler() throw();

protected:
    /// @name inherited from GenericSAXHandler
    //@{
    /** the user-impemlented handler method for an opening tag */
    void myStartElement(SumoXMLTag element,
                        const Attributes &attrs) throw(ProcessError);
    //@}

protected:
    /// the options (program settings)
    OptionsCont &myOptions;

    RODFDetectorCon &myContainer;

private:
    /// invalidated copy constructor
    RODFDetectorHandler(const RODFDetectorHandler &src);

    /// invalidated assignment operator
    RODFDetectorHandler &operator=(const RODFDetectorHandler &src);

};


#endif

/****************************************************************************/

