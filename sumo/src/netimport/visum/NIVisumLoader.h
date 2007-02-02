/****************************************************************************/
/// @file    NIVisumLoader.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 19 Jul 2002
/// @version $Id: $
///
// A loader visum-files
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
#ifndef NIVisumLoader_h
#define NIVisumLoader_h
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
#include <map>
#include <vector>
#include <netbuild/NBCapacity2Lanes.h>
#include <utils/importio/LineHandler.h>
#include <utils/importio/LineReader.h>
#include <utils/importio/NamedColumnsParser.h>
#include <utils/common/FileErrorReporter.h>
#include "NIVisumTL.h"


// ===========================================================================
// class declaration
// ===========================================================================
class OptionsCont;
class NBNetBuilder;
class NBNodeCont;
class NBEdgeCont;
class NBNode;
class NBEdge;


// ===========================================================================
// class declaration
// ===========================================================================
/**
 * @class NIVisumLoader
 * @brief This class parses the given visum file.
 *
 * When the edge definition is before the node and the type definitions, it
 * will be parsed using a second step; otherwise this class parses the file
 * using a single step.
 * Types are loaded optionally.
 */
class NIVisumLoader :
            public FileErrorReporter
{
public:
    typedef std::map<std::string, std::string> VSysTypeNames;
    typedef std::map<std::string, NIVisumTL*> NIVisumTL_Map;
public:
    /// constructor
    NIVisumLoader(NBNetBuilder &nb, const std::string &file,
                  NBCapacity2Lanes capacity2Lanes, bool useVisumPrio);

    /// destructor
    ~NIVisumLoader();

    /// starts the parsing
    void load(OptionsCont &options);

public:
    /**
     * @class NIVisumSingleDataTypeParser
     * @brief The class that parses entries of a certain data type, like edges, edge
     * connections etc.
     */
class NIVisumSingleDataTypeParser :
                public FileErrorReporter::Child,
                public LineHandler
    {
    public:
        /// constructor
        NIVisumSingleDataTypeParser(NIVisumLoader &parent,
                                    const std::string &dataName);

        /// Destructor
        virtual ~NIVisumSingleDataTypeParser();

        /** @brief Sets the position of the data-type within the visum-file
         *
         * The position is saved as a byte-offset within the file */
        void setStreamPosition(long pos);

        /** @brief Returns the information whether the data type was found within the visum-file
         *
         * This method returns valid values only after the first step - the scanning process */
        bool positionKnown() const;

        /// Reads the data-type from the visum-file using the given reader
        bool readUsing(LineReader &reader);

        /// Returns the name of the data type
        const std::string &getDataName() const;

        /** @brief LineHandler-interface
         *
         * Returns values from the visum-file; Checks whether the data type is over */
        bool report(const std::string &line);

        /// Initialises the line parser
        void initLineParser(const std::string &pattern);

    protected:
        /** @brief builds structures from read data
         *
         * When this method which must be implemented by derived classes, each
         * loading a certain type of visum-data, the line parser contains the
         * values of the next data line.
         * This method is called only for data of a single data type, without
         * the head and the tail */
        virtual void myDependentReport() = 0;

        /** @brief Builds and reports an error
         *
         * We had to name it this way, as otherwise it may be ambigous with the
         * method from FileErrorReporter */
        void addError2(const std::string &type, const std::string &id,
                       const std::string &exception);

        /** @brief tries to get a SUMOReal which is possibly assigned to a certain modality
         *
         * When the SUMOReal cannot be extracted using the given name, "IV" is
         * appended to the begin of the name. Remark that this function does not
         * yet support public traffic. */
        SUMOReal getWeightedFloat(const std::string &name);

        /** @brief tries to get a bool which is possibly assigned to a certain modality
         *
         * When the bool cannot be extracted using the given name, "IV" is
         * appended to the begin of the name. Remark that this function does not
         * yet support public traffic. */
        bool getWeightedBool(const std::string &name);

        /** @brief Tries to get the node which name is stored in the given field
         *
         * If either the "fieldName" does not occure within the currently loaded
         *  data line or the node was not loaded before, an error is generated and
         *  0 is returned.
         * The "dataName" is used to report errors.
         */
        NBNode *getNamedNode(NBNodeCont &nc, const std::string &dataName,
                             const std::string &fieldName);

        /** @brief The same, but two different names for the field are allowed */
        NBNode *getNamedNode(NBNodeCont &nc, const std::string &dataName,
                             const std::string &fieldName1, const std::string &fieldName2);


        /** @brief Tries to get the edge which name is stored in the given field
         *
         * If either the "fieldName" does not occure within the currently loaded
         *  data line or the edge was not loaded before, an error is generated and
         *  0 is returned.
         * The "dataName" is used to report errors.
         */
        NBEdge *getNamedEdge(NBEdgeCont &nc, const std::string &dataName,
                             const std::string &fieldName);

        /** @brief The same, but two different names for the field are allowed */
        NBEdge *getNamedEdge(NBEdgeCont &nc, const std::string &dataName,
                             const std::string &fieldName1, const std::string &fieldName2);


        /** @brief Tries to get the edge which name is stored in the given field
         * continuating the search for a subedge that ends at the given node
         *
         * If either the "fieldName" does not occure within the currently loaded
         *  data line or the edge was not loaded before, an error is generated and
         *  0 is returned.
         * The "dataName" is used to report errors.
         */
        NBEdge *getNamedEdgeContinuating(NBEdgeCont &nc, const std::string &dataName,
                                         const std::string &fieldName, NBNode *node);

        /** @brief The same, but two different names for the field are allowed */
        NBEdge *getNamedEdgeContinuating(NBEdgeCont &nc, const std::string &dataName,
                                         const std::string &fieldName1, const std::string &fieldName2,
                                         NBNode *node);

        /** @brief The same, but the search is started at a named edge */
        NBEdge *getNamedEdgeContinuating(NBEdgeCont &nc, const std::string &name,
                                         NBNode *node);


        /** @brief Returns the named value as a float
         */
        SUMOReal getNamedFloat(const std::string &fieldName);
        SUMOReal getNamedFloat(const std::string &fieldName, SUMOReal defaultValue);

        /** @brief The same, but two different names for the field are allowed */
        SUMOReal getNamedFloat(const std::string &fieldName1,
                               const std::string &fieldName2);
        SUMOReal getNamedFloat(const std::string &fieldName1,
                               const std::string &fieldName2, SUMOReal defaultValue);


        /** @brief Returns the named value as a string
         */
        std::string getNamedString(const std::string &fieldName);

        /** @brief The same, but two different names for the field are allowed */
        std::string getNamedString(const std::string &fieldName1,
                                   const std::string &fieldName2);

        /** @brief Returns the opposite direction of the given edge
         * till its begin
         */
        NBEdge *getReversedContinuating(NBEdgeCont &nc, NBEdge *edge,
                                        NBNode *node);

    private:
        /// Continues the search for a matching edge continuation
        NBEdge *getNamedEdgeContinuating(NBEdge *begin, NBNode *node);


    protected:
        /// The line parser to use
        NamedColumnsParser myLineParser;

        /// The name of the parsed data
        std::string myDataName;

    private:
        /// Sets the stream position to the
        void rewind(std::istream &strm) const;

        /// Returns the information whether the current line is an end-of-data line
        bool dataTypeEnded(const std::string &msg);

    private:
        /// Position the certain data type starts at within the stream
        long myPosition;

    };

private:
    /// the line reader to read from the file
    LineReader myLineReader;

    /** @brief the parser to parse the information from the data lines
     *
     * the visum format seems to vary, so a named parser is needed */
    NamedColumnsParser _lineParser;

    /// the converter to compute the lane number of edges from their capacity
    NBCapacity2Lanes _capacity2Lanes;

    /// the used vsystypes
    VSysTypeNames myVSysTypes;

    /** @brief definition of the list of known parsers
        (each reads a certain visum-type) */
    typedef std::vector<NIVisumSingleDataTypeParser*> ParserVector;

    /// list of known parsers
    ParserVector mySingleDataParsers;

    // list of visum traffic lights
    NIVisumTL_Map myNIVisumTLs;
private:
    /**
     * @class PositionSetter
     * @brief Used within the scanning step for setting the positions of the data
     */
class PositionSetter : public LineHandler
    {
    public:
        /// Constructor
        PositionSetter(NIVisumLoader &parent);

        /// Destructor
        ~PositionSetter();

        /** @brief Interface of line handler - receives all lines, individually */
        bool report(const std::string &result);

    private:
        /** @brief The loader to inform about the occurence of a new data type begin
         *
         * The positions are not set within the PositionSetter itself.
         * Rather, the Loader performs this. */
        NIVisumLoader &myParent;

    };

public:
    /** @brief The PositionSetter is an internal class for retrieving data positions
     *
     * It shall have access to "checkForPosition" */
    friend class PositionSetter;

private:
    /** @brief Sets the begin of known data
     *
     * Checks whether the current line is the begin of one of the known data
     * and saves the current position within the stream into the data type loader
     * if so */
    bool checkForPosition(const std::string &line);

    NBTrafficLightLogicCont &myTLLogicCont;
    NBEdgeCont &myEdgeCont;

};


#endif

/****************************************************************************/

