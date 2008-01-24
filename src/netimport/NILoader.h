/****************************************************************************/
/// @file    NILoader.h
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Perfoms network import
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
#ifndef NILoader_h
#define NILoader_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <netbuild/NLLoadFilter.h>
#include <xercesc/sax2/SAX2XMLReader.hpp>


// ===========================================================================
// xerces 2.2 compatibility
// ===========================================================================
#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace XERCES_CPP_NAMESPACE;
#endif


// ===========================================================================
// class definitions
// ===========================================================================
class OptionsCont;
class SUMOSAXHandler;
class LineReader;
class LineHandler;
class NBNetBuilder;


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class NILoader
 * @brief Perfoms network import
 *
 * This class loads the specified data using a loading method that accords
 * to the data type specified by the user.
 * Subclasses may be used for the loading process, too, as it is done while
 * loading the XML-descriptions.
 */
class NILoader
{
public:
    NILoader(NBNetBuilder &nb);
    ~NILoader();

    /** loads data from the files specified in the given option container */
    void load(OptionsCont &oc);

private:
    /** loads data from sumo-files */
    void loadSUMO(OptionsCont &oc);

    /** loads net or logics */
    void loadSUMOFiles(OptionsCont &oc, LoadFilter what,
                       const std::string &files, const std::string &type);

    /** loads data from XML-files */
    void loadXML(OptionsCont &oc);

    /** loads data from the list of xml-files of certain type */
    void loadXMLType(SUMOSAXHandler *handler,
                     const std::vector<std::string> &files, const std::string &type);

    /** reads using a file reader */
    bool useLineReader(LineReader &lr, const std::string &file,
                       LineHandler &lh);

    /** loads data from visum-input-file */
    void loadVisum(OptionsCont &oc);

    /** loads data from arcview-files */
    void loadArcView(OptionsCont &oc);

    /** loads data from vissim-input-file */
    void loadVissim(OptionsCont &oc);

    /// loads Elmar's data parsed from GDF
    void loadElmar(OptionsCont &oc);

    void loadTiger(OptionsCont &oc);

    void loadOSM(OptionsCont &oc);

private:
    NBNetBuilder &myNetBuilder;

private:
    /// @brief Invalidated copy constructor.
    NILoader(const NILoader&);

    /// @brief Invalidated assignment operator.
    NILoader& operator=(const NILoader&);

};


#endif

/****************************************************************************/

