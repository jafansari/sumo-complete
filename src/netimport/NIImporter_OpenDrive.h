/****************************************************************************/
/// @file    NIImporter_OpenDrive.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 14.04.2008
/// @version $Id: NIImporter_OpenDrive.h 7440 2009-07-15 09:04:03Z dkrajzew $
///
// Importer for networks stored in openDrive format
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef NIImporter_OpenDrive_h
#define NIImporter_OpenDrive_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/geom/Position2DVector.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBNetBuilder;
class NBEdge;
class OptionsCont;
class NBNode;
class NBNodeCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_OpenDrive
 * @brief Importer for networks stored in openDrive format
 *
 */
class NIImporter_OpenDrive : public SUMOSAXHandler {
public:
    /** @brief Loads content of the optionally given SUMO file
     *
     * If the option "opendrive" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "opendrive" is not set, this method simply returns.
     *
     * The loading is done by parsing the network definition as an XML file
     *  using the SAXinterface and handling the incoming data via this class'
     *  methods.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont &oc, NBNetBuilder &nb);


protected:
    enum LinkType {
        OPENDRIVE_LT_SUCCESSOR,
        OPENDRIVE_LT_PREDECESSOR
    };

    enum ElementType {
        OPENDRIVE_ET_UNKNOWN,
        OPENDRIVE_ET_ROAD,
        OPENDRIVE_ET_JUNCTION
    };

    enum ContactPoint {
        OPENDRIVE_CP_UNKNOWN,
        OPENDRIVE_CP_START,
        OPENDRIVE_CP_END
    };

    enum GeometryType {
        OPENDRIVE_GT_UNKNOWN,
        OPENDRIVE_GT_LINE,
        OPENDRIVE_GT_SPIRAL,
        OPENDRIVE_GT_ARC,
        OPENDRIVE_GT_POLY3
    };

    /**
     * @struct OpenDriveLink
     * @brief Representation of an openDrive connection
     */
    struct OpenDriveLink {
        OpenDriveLink(LinkType linkTypeArg, const std::string &elementIDArg)
                : linkType(linkTypeArg), elementID(elementIDArg),
                elementType(OPENDRIVE_ET_UNKNOWN), contactPoint(OPENDRIVE_CP_UNKNOWN) { }

        LinkType linkType;
        std::string elementID;
        ElementType elementType;
        ContactPoint contactPoint;
    };


    /**
     * @struct OpenDriveGeometry
     * @brief Representation of an openDrive geometry part
     */
    struct OpenDriveGeometry {
        OpenDriveGeometry(SUMOReal lengthArg, SUMOReal sArg, SUMOReal xArg, SUMOReal yArg, SUMOReal hdgArg)
                : length(lengthArg), s(sArg), x(xArg), y(yArg), hdg(hdgArg),
                type(OPENDRIVE_GT_UNKNOWN) { }

        SUMOReal length;
        SUMOReal s;
        SUMOReal x;
        SUMOReal y;
        SUMOReal hdg;
        GeometryType type;
        std::vector<SUMOReal> params;
    };


    /**
     * @struct OpenDriveEdge
     * @brief Representation of an openDrive road
     */
    struct OpenDriveEdge {
        OpenDriveEdge(const std::string &idArg, const std::string &junctionArg, SUMOReal lengthArg)
                : id(idArg), junction(junctionArg), length(lengthArg),
                from(0), to(0) { }

        /// @brief The id of the edge
        std::string id;
        /// @brief The id of the junction the edge belongs to
        std::string junction;
        /// @brief The length of the edge
        SUMOReal length;
        std::vector<OpenDriveLink> links;
        std::vector<OpenDriveGeometry> geometries;
        NBNode *from;
        NBNode *to;
        std::vector<Position2D> geom;
    };


protected:
    /** @brief Constructor
     * @param[in] nc The node control to fill
     */
    NIImporter_OpenDrive(NBNodeCont &nc, std::vector<OpenDriveEdge> &edgeCont);


    /// @brief Destructor
    ~NIImporter_OpenDrive() throw();



    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * In dependence to the obtained type, an appropriate parsing
     *  method is called ("addEdge" if an edge encounters, f.e.).
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(SumoXMLTag element,
                        const SUMOSAXAttributes &attrs) throw(ProcessError);


    /** @brief Called when characters occure
     *
     * @param[in] element ID of the last opened element
     * @param[in] chars The read characters (complete)
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myCharacters
     */
    void myCharacters(SumoXMLTag element,
                      const std::string &chars) throw(ProcessError);


    /** @brief Called when a closing tag occures
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(SumoXMLTag element) throw(ProcessError);
    //@}



private:
    void addLink(LinkType lt, const std::string &elementType, const std::string &elementID,
                 const std::string &contactPoint) throw(ProcessError);

    void addGeometryShape(GeometryType type, const std::vector<SUMOReal> &vals) throw(ProcessError);



    OpenDriveEdge myCurrentEdge;

    std::vector<OpenDriveEdge> &myEdges;
    std::vector<SumoXMLTag> myElementStack;


protected:
    /** @brief Builds a node or returns the already built
     *
     * If the node is already known, it is returned. Otherwise, the
     *  node is built. If the newly built node can not be added to 
     *  the container, a ProcessError is thrown. 
     *  Otherwise this node is returned.
     *
     * @param[in] id The id of the node to build/get
     * @param[in, changed] pos The position of the node to build/get
     * @param[filled] nc The node container to retrieve/add the node to
     * @return The retrieved/built node
     * @exception ProcessError If the node could not be built/retrieved
     */
    static NBNode *getOrBuildNode(const std::string &id, Position2D &pos, NBNodeCont &nc) throw(ProcessError);


    static std::vector<Position2D> geomFromLine(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw();
    static std::vector<Position2D> geomFromSpiral(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw();
    static std::vector<Position2D> geomFromArc(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw();
    static std::vector<Position2D> geomFromPoly(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw();
    static Position2D calculateStraightEndPoint(double hdg, double length, const Position2D &start) throw();
    static void calculateClothoidProperties(SUMOReal *x, SUMOReal *y, SUMOReal *hdg, SUMOReal curve, SUMOReal length, SUMOReal dist, bool direction) throw();
    static void calculateFirstClothoidPoint(SUMOReal* ad_X, SUMOReal* ad_Y, SUMOReal* ad_hdg, SUMOReal ad_curvature, SUMOReal ad_lengthE) throw();
    static void calculateCurveCenter(SUMOReal *ad_x, SUMOReal *ad_y, SUMOReal ad_radius, SUMOReal ad_hdg) throw();

};


#endif

/****************************************************************************/

