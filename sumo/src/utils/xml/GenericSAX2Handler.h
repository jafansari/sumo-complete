/****************************************************************************/
/// @file    GenericSAX2Handler.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 15 Apr 2002
/// @version $Id: $
///
// A class extending the SAX-parser functionality
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
#ifndef GenericSAX2Handler_h
#define GenericSAX2Handler_h
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
#include <stack>
#include <map>
#include <sstream>
#include <vector>
#include <sax2/Attributes.hpp>
#include <sax2/DefaultHandler.hpp>


// ===========================================================================
// xerces 2.2 compatibility
// ===========================================================================
#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace XERCES_CPP_NAMESPACE;
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GenericSAX2Handler
 * GenericSAX2Handler is an extended SAX2-DefaultHandler which provides a faster access
 * to a enumeration of tags through a map
 * This handler has no parsing functionality which is only implemented in the
 * derived classes, where each class solves a single step of the parsing
 */
class GenericSAX2Handler : public DefaultHandler
{
public:
    /** a tag name with its numerical representation */
    struct Tag
    {
        /// The xml-element-name (latin1)
        const char *name;
        /// The numerical representation of the attribute
        int value;
    };

public:
    /** constructor */
    GenericSAX2Handler();

    /** constructor */
    GenericSAX2Handler(const Tag *tags, int tagNo);

    /** destructor */
    ~GenericSAX2Handler();

    /** adds a known tag to the list */
    void addTag(const std::string &name, int id);

    /** returns the information whether an error occured during the parsing */
    bool errorOccured() const;

    /** returns the information whether an unknown tag occured */
    bool unknownOccured() const;

    /** @brief The inherited method called when a new tag opens
        This method calls the user-implemented methof myStartElement */
    void startElement(const XMLCh* const uri, const XMLCh* const localname,
                      const XMLCh* const qname, const Attributes& attrs);

    /** @brief The inherited method called when characters occured
        The characters are appended into a private buffer and given to
        myCharacters when the according tag is being closed */
    void characters(const XMLCh* const chars, const unsigned int length);

    /** @brief The inherited method called when a tag is being closed
        This method calls the user-implemented methods myCharacters and
        and myEndElement */
    void endElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname);

    /** called when ignorable whitespaces occure */
    void ignorableWhitespace(const XMLCh* const chars,
                             const unsigned int length);

    /** called when the document shall be resetted */
    virtual void resetDocument();

    /** called when a XML-warning occures */
    virtual void warning(const SAXParseException& exception);

    /** called when a XML-error occures */
    virtual void error(const SAXParseException& exception);

    /** called when a XML-fatal error occures */
    virtual void fatalError(const SAXParseException& exception);

protected:
    /** @brief handler method for an opening tag to implement by derived classes
        This method is only called when the tag name was supplied by the user */
    virtual void myStartElement(int element, const std::string &name,
                                const Attributes &attrs) = 0;

    /** @brief handler method for characters to implement by derived classes
        This method is only called when tha tag name was supplied by the user */
    virtual void myCharacters(int element, const std::string &name,
                              const std::string &chars) = 0;

    /** @brief handler method for a closing tag to implement by derived classes
        This tag is only called when tha tag name was supplied by the user */
    virtual void myEndElement(int element, const std::string &name) = 0;

    /** a dump-methods that may be used to avoid "unused attribute"-warnings */
    void myStartElementDump(int element, const std::string &name,
                            const Attributes &attrs);

    /** a dump-methods that may be used to avoid "unused attribute"-warnings */
    void myCharactersDump(int element, const std::string &name,
                          const std::string &chars);

    /** a dump-methods that may be used to avoid "unused attribute"-warnings */
    void myEndElementDump(int element, const std::string &name);

    /** build an error description */
    std::string buildErrorMessage(const std::string &file,
                                  const std::string &type,
                                  const SAXParseException& exception);

protected:
    /** the type of the map the maps tag names to ints */
    typedef std::map<std::string, int> TagMap;

    /** the information whether an error occured during the parsing */
    bool _errorOccured;

    /** the information whether an unknown tag occured */
    bool _unknownOccured;

    /** the map of tag names to their internal numerical representation */
    TagMap _tagMap;

    /** the current position in the xml-tree as a stack */
    std::stack<int> _tagTree;

    /// A list of characters string obtained so far to build the complete characters string at the end
    std::vector<std::string> myCharactersVector;

private:
    /** converts a tag from its string into its numerical representation */
    int convertTag(const std::string &tag) const;
};


#endif

/****************************************************************************/

