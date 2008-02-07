/****************************************************************************/
/// @file    NLHandler.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The XML-Handler for network loading (parses the elements)
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
#ifndef NLHandler_h
#define NLHandler_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include "NLBuilder.h"
#include "NLDiscreteEventBuilder.h"
#include "NLSucceedingLaneBuilder.h"
#include <microsim/MSLink.h>
#include <microsim/MSRouteHandler.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/traffic_lights/MSActuatedTrafficLightLogic.h>
#include <microsim/MSBitSetLogic.h>
#include <microsim/MSEdgeContinuations.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// xerces 2.2 compatibility
// ===========================================================================
#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace XERCES_CPP_NAMESPACE;
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class NLContainer;
class NLDetectorBuilder;
class NLTriggerBuilder;
class MSTrafficLightLogic;
class NLGeomShapeBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLNetHandler
 * The SAX2-handler responsible for parsing networks and routes to load.
 * This is an extension of the MSRouteHandler as routes and vehicles may also
 * be loaded from network descriptions.
 */
class NLHandler : public MSRouteHandler
{
public:
    /// @brief Definition of a lane vector
    typedef std::vector<MSLane*> LaneVector;

public:
    /** @brief Constructor
     *
     * @param[in] file Name of the parsed file
     * @param[in, out] net The network to fill
     * @param[in] detBuilder The detector builder to use
     * @param[in] triggerBuilder The trigger builder to use
     * @param[in] edgeBuilder The builder of edges to use
     * @param[in] junctionBuilder The builder of junctions to use
     * @param[in] shapeBuilder The geometric shapes builder to use
     * @param[in] incDUABase Base of incremented vehicle loading
     * @param[in] incDUAStage Stage of incremented vehicle loading
     * @todo Describe incDUA; check whether another API would be more appropriate (maybe this should be moved into VehicleControl)
     */
    NLHandler(const std::string &file, MSNet &net,
              NLDetectorBuilder &detBuilder, NLTriggerBuilder &triggerBuilder,
              NLEdgeControlBuilder &edgeBuilder,
              NLJunctionControlBuilder &junctionBuilder,
              NLGeomShapeBuilder &shapeBuilder,
              int incDUABase, int incDUAStage) throw();


    /// @brief Destructor
    virtual ~NLHandler() throw();


    /// @brief sets the data type filter
    /// @todo Remove load filter functionality
    void setWanted(NLLoadFilter filter);


    /** @brief Returns the information about lane continuations
     *
     * @return Edge continuations
     * @todo This should probably be a part of the network
     */
    const MSEdgeContinuations &getContinuations() const;


protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     * @todo Refactor/describe
     */
    virtual void myStartElement(SumoXMLTag element,
                                const Attributes &attrs) throw(ProcessError);


    /** @brief Called when characters occure
     *
     * @param[in] element ID of the last opened element
     * @param[in] chars The read characters (complete)
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myCharacters
     * @todo Refactor/describe
     */
    virtual void myCharacters(SumoXMLTag element,
                              const std::string &chars) throw(ProcessError);


    /** @brief Called when a closing tag occures
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     * @todo Refactor/describe
     */
    virtual void myEndElement(SumoXMLTag element) throw(ProcessError);
    //@}


protected:
    void addParam(const Attributes &attrs);

    /** @brief adds a detector
        Determines the type of the detector first, and then calls the
         appropriate method */
    virtual void addDetector(const Attributes &attrs);

    /// Builds an e1-detector using the given specification
    virtual void addE1Detector(const Attributes &attrs);

    /// Builds an e2-detector using the given specification
    virtual void addE2Detector(const Attributes &attrs);

    /// Starts building of an e3-detector using the given specification
    void beginE3Detector(const Attributes &attrs);

    void addE3Entry(const Attributes &attrs);
    void addE3Exit(const Attributes &attrs);

    /// Builds of an e3-detector using collected values
    virtual void endE3Detector();

    /// Ends the detector building
    void endDetector();


protected:
    /** returns the information whether instances belonging to the
        given class of data shall be extracted during this parsing */
    bool wanted(NLLoadFilter filter) const;

    /// Closes the process of building an edge
    virtual void closeEdge();

    /// Closes the process of building a lane
    virtual void closeLane();

    /// Closes the process of building lanes of an edge
    virtual void closeLanes();

    /// Closes the process of the connections between edges
    virtual void closeAllowedEdge();



protected:
    /// The net to fill (preinitialised)
    MSNet &myNet;

    /// the definition of what to load
    NLLoadFilter myLoadFilter;


private:
    /// add the shape to the Lane
    void addLaneShape(const std::string &chars);

    /// sets the number of edges the network contains
    void setEdgeNumber(const Attributes &attrs);

    /// begins the processing of an edge
    void chooseEdge(const Attributes &attrs);

    /// adds a lane to the previously opened edge
    void addLane(const Attributes &attrs);

    /// adds a polygon
    void addPOI(const Attributes &attrs);

    /// adds a polygon
    void addPoly(const Attributes &attrs);

    /// add the position to the Polygon
    void addPolyPosition(const std::string &chars);

    /// opens the list of next edges for processing
    void openAllowedEdge(const Attributes &attrs);

    /// adds a logic item to the current logic
    void addLogicItem(const Attributes &attrs);

    /// begins the reading of a traffic lights logic
    void initTrafficLightLogic(const Attributes &attrs);

    /// adds a phase to the traffic lights logic currently build
    void addPhase(const Attributes &attrs);


    /// opens a junction for processing
    virtual void openJunction(const Attributes &attrs);

    /// adds a source
    virtual void addSource(const Attributes &attrs);

    /// adds a trigger
    void addTrigger(const Attributes &attrs);

    /// opens the container of succeeding lanes for processing
    void openSucc(const Attributes &attrs);

    /// adds a succeeding lane
    void addSuccLane(const Attributes &attrs);


    /// adds the incoming lanes
    void addIncomingLanes(const std::string &chars);

    /// adds the incoming Polygon's Positions
    void addIncomingPolyPosititon(const std::string &chars);

#ifdef HAVE_INTERNAL_LANES
    /// adds the internal lanes
    void addInternalLanes(const std::string &chars);
#endif


    virtual void openWAUT(const Attributes &attrs);
    void addWAUTSwitch(const Attributes &attrs);
    void addWAUTJunction(const Attributes &attrs);


    /// allocates edges from the initial list
    void allocateEdges(const std::string &chars);

    /// sets the number of nodes (allocates storage when needed)
    void setNodeNumber(const std::string &chars);


    /// adds the list of allowed following edges
    void addAllowedEdges(const std::string &chars);

    /// adds the route elements
    void addRouteElements(const std::string &name, const std::string &chars);

    /// sets the request size of the current junction logic
    void setRequestSize(const std::string &chars);

    /// sets the lane number of the current junction logic
    void setLaneNumber(const std::string &chars);

    /// sets the key of the current junction logic
    void setKey(const std::string &chars);

    /// sets the subkey of the current junction logic
    void setSubKey(const std::string &chars);

    /// Sets the offset a tl-logic shall be fired the first time after
    void setOffset(const std::string &chars);


    void setNetOffset(const std::string &chars);
    void setNetConv(const std::string &chars);
    void setNetOrig(const std::string &chars);



    /// ends the loading of a junction logic
    void closeJunctionLogic();

    /// ends the processing of a junction
    virtual void closeJunction();

    void closeWAUT();

    /// closes the processing of a lane
    void closeSuccLane();

    /// Parses the given character into an enumeration typed link direction
    MSLink::LinkDirection parseLinkDir(char dir);

    /// Parses the given character into an enumeration typed link state
    MSLink::LinkState parseLinkState(char state);


protected:
    /// A builder for object actions
    NLDiscreteEventBuilder myActionBuilder;

    /// Information whether the currently parsed edge is internal and not wished, here
    bool myCurrentIsInternalToSkip;

    /// The detector builder to use
    NLDetectorBuilder &myDetectorBuilder;

    /// The type of the last detector
    std::string myCurrentDetectorType;

    /// The trigger builder to use
    NLTriggerBuilder &myTriggerBuilder;

    /** storage for edges during building */
    NLEdgeControlBuilder &myEdgeControlBuilder;

    /** storage for junctions during building */
    NLJunctionControlBuilder &myJunctionControlBuilder;

    NLGeomShapeBuilder &myShapeBuilder;

    /// storage for building succeeding lanes
    NLSucceedingLaneBuilder mySucceedingLaneBuilder;


    /// @name Information about a lane
    //@{

    /// The id of the current lane
    std::string myCurrentLaneID;

    /// The information whether the current lane is a depart lane
    bool myLaneIsDepart;

    /// The maximum speed allowed on the current lane
    SUMOReal myCurrentMaxSpeed;

    /// The length of the current lane
    SUMOReal myCurrentLength;

    /// Vehicle classes allowed/disallowed on the current lane
    std::string myVehicleClasses;

    /// The shape of the current lane
    Position2DVector myShape;
    //@}

    /// Edge continuations
    MSEdgeContinuations myContinuations;

    /// internal information whether a tls-logic is currently read
    bool myAmInTLLogicMode;

    /// The id of the currently processed WAUT
    std::string myCurrentWAUTID;

    /// The network offset
    Position2D myNetworkOffset;

    /// The network's boundaries
    Boundary myOrigBoundary, myConvBoundary;

    bool myCurrentIsBroken;

private:
    /** invalid copy constructor */
    NLHandler(const NLHandler &s);

    /** invalid assignment operator */
    NLHandler &operator=(const NLHandler &s);

};


#endif

/****************************************************************************/

