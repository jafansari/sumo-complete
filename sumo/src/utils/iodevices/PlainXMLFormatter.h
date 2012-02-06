/****************************************************************************/
/// @file    PlainXMLFormatter.h
/// @author  Michael Behrisch
/// @date    2012
/// @version $Id$
///
// Output formatter for plain XML output
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef PlainXMLFormatter_h
#define PlainXMLFormatter_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "OutputFormatter.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PlainXMLFormatter
 * @brief Output formatter for plain XML output
 *
 * PlainXMLFormatter format XML like output into the output stream.
 */
class PlainXMLFormatter : public OutputFormatter {
public:
    /// @brief Constructor
    PlainXMLFormatter(const unsigned int defaultIndentation=0);


    /// @brief Destructor
    virtual ~PlainXMLFormatter() { }


    /** @brief Writes an XML header with optional configuration
     *
     * If something has been written (myXMLStack is not empty), nothing
     *  is written and false returned.
     *
     * @param[in] rootElement The root element to use
     * @param[in] xmlParams Additional parameters (such as encoding) to include in the <?xml> declaration
     * @param[in] attrs Additional attributes to save within the rootElement
     * @param[in] comment Additional comment (saved in front the rootElement)
     * @todo Check which parameter is used herein
     * @todo Describe what is saved
     */
    bool writeXMLHeader(std::ostream& into, const std::string& rootElement,
                        const std::string xmlParams = "",
                        const std::string& attrs = "",
                        const std::string& comment = "");


    /** @brief Opens an XML tag
     *
     * An indentation, depending on the current xml-element-stack size, is written followed
     *  by the given xml element ("<" + xmlElement)
     * The xml element is added to the stack, then.
     *
     * @param[in] xmlElement Name of element to open
     * @returns The OutputDevice for further processing
     */
    void openTag(std::ostream& into, const std::string& xmlElement);


    /** @brief Opens an XML tag
     *
     * Helper method which finds the correct string before calling openTag.
     *
     * @param[in] xmlElement Id of the element to open
     */
    void openTag(std::ostream& into, const SumoXMLTag& xmlElement);


    /** @brief Ends the most recently opened element start.
     *
     * Writes more or less nothing but ">" and a line feed.
     */
    void closeOpener(std::ostream& into);


    /** @brief Closes the most recently opened tag
     *
     * @param[in] name whether abbreviated closing is performed
     * @returns Whether a further element existed in the stack and could be closed
     * @todo it is not verified that the topmost element was closed
     */
    bool closeTag(std::ostream& into, bool abbreviated=false);


    /** @brief writes an arbitrary attribute
     *
     * @param[in] attr The attribute (name)
     * @param[in] val The attribute value
     */
    void writeAttr(std::ostream& into, const std::string& attr, const std::string& val);


	/** @brief writes a named attribute
     *
     * @param[in] attr The attribute (name)
     * @param[in] val The attribute value
     */
    template <class T>
    static void writeAttr(std::ostream& into, const SumoXMLAttr attr, const T& val) {
		into << " " << toString(attr) << "=\"" << toString(val, into.precision()) << "\"";
	}


private:
    /// @brief The stack of begun xml elements
    std::vector<std::string> myXMLStack;

    /// @brief The initial indentation level
    unsigned int myDefaultIndentation;


};


#endif

/****************************************************************************/

