/****************************************************************************/
/// @file    NBEdge.h
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// The representation of a single edge during network building
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
#ifndef NBEdge_h
#define NBEdge_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <map>
#include <vector>
#include <string>
#include <set>
#include "NBCont.h"
#include <utils/common/UtilExceptions.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/Bresenham.h>
#include <utils/geom/Position2DVector.h>
#include <utils/geom/Line2D.h>
#include <utils/common/SUMOVehicleClass.h>
#include "NBHelpers.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NBNodeCont;
class NBEdgeCont;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBEdge
 * @brief The representation of a single edge during network building
 */
class NBEdge
{
public:
    /**  @enum LaneSpreadFunction
     * @brief Information how the edge's lateral offset shall be computed
     *
     * In dependence to this value, lanes will be spread to the right side or
     *  to both sides from the given edge geometry (Also used when node
     *  positions are used as edge geometry).
     */
    enum LaneSpreadFunction {
        /// @brief The lanes will be spread to right
        LANESPREAD_RIGHT,
        /// @brief The lanes will be spread to both sides
        LANESPREAD_CENTER
    };


    /** @enum EdgeBuildingStep
     * @brief Current state of the edge within the building process
     *
     * As the network is build in a cascaded way, considering loaded
     *  information, a counter holding the current step is needed. This is done
     *  by using this enumeration.
     */
    enum EdgeBuildingStep {
        /// @brief The edge has been loaded and connections shall not be added
        INIT_REJECT_CONNECTIONS,
        /// @brief The edge has been loaded, nothing is computed yet
        INIT,
        /// @brief The relationships between edges are computed/loaded
        EDGE2EDGES,
        /// @brief Lanes to edges - relationships are computed/loaded
        LANES2EDGES,
        /// @brief Lanes to lanes - relationships are computed; should be recheked
        LANES2LANES_RECHECK,
        /// @brief Lanes to lanes - relationships are computed; no recheck is necessary/wished
        LANES2LANES_DONE,
        /// @brief Lanes to lanes - relationships are loaded; no recheck is necessary/wished
        LANES2LANES_USER
    };


    /** @enum Lane2LaneInfoType
    * @brief Modes of setting connections between lanes
    */
    enum Lane2LaneInfoType {
        /// @brief The connection was computed
        L2L_COMPUTED,
        /// @brief The connection was given by the used
        L2L_USER,
        /// @brief The connection was computed and validated
        L2L_VALIDATED
    };


    /** @struct Lane
     * @brief An (internal) definition of a single lane of an edge
     */
    struct Lane {
        /// @brief The lane's shape
        Position2DVector shape;
        /// @brief The speed allowed on this lane
        SUMOReal speed;
        /// @brief List of vehicle types that are allowed on this lane
        std::vector<SUMOVehicleClass> allowed;
        /// @brief List of vehicle types that are not allowed on this lane
        std::vector<SUMOVehicleClass> notAllowed;
    };


    /** @struct Connection
     * @brief A structure which describes a connection between edges or lanes
     */
    struct Connection {
        /** @brief Constructor
         * @param[in] fromLane_ The lane the connections starts at
         * @param[in] toEdge_ The edge the connections yields in
         * @param[in] toLane_ The lane the connections yields in
         */
        Connection(int fromLane_, NBEdge *toEdge_, int toLane_) throw()
                : fromLane(fromLane_), toEdge(toEdge_), toLane(toLane_) { }


        /// @brief The lane the connections starts at
        int fromLane;
        /// @brief The edge the connections yields in
        NBEdge *toEdge;
        /// @brief The lane the connections yields in
        int toLane;
        /// @brief The id of the traffic light that controls this connection
        std::string tlID;
        /// @brief The index of this connection within the controlling traffic light
        unsigned int tlLinkNo;

    };


public:
    /** @brief Constructor
     *
     * Use this if no edge geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (my be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string &id,
           NBNode *from, NBNode *to, std::string type,
           SUMOReal speed, unsigned int nolanes, int priority,
           LaneSpreadFunction spread=LANESPREAD_RIGHT) throw(ProcessError);


    /** @brief Constructor
     *
     * Use this if the edge's geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (my be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] geom The edge's geomatry
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string &id,
           NBNode *from, NBNode *to, std::string type,
           SUMOReal speed, unsigned int nolanes, int priority,
           Position2DVector geom, bool tryIgnoreNodePositions,
           LaneSpreadFunction spread=LANESPREAD_RIGHT) throw(ProcessError);


    /** @brief Destructor
     */
    ~NBEdge() throw();



    /// @name Atomar getter methods
    //@{

    /** @brief Returns the id of the edge
     * @return This edge's id
     */
    const std::string &getID() const throw() {
        return myID;
    }


    /** @brief Returns the number of lanes
     * @returns This edge's number of lanes
     */
    unsigned int getNoLanes() const throw() {
        return (unsigned int) myLanes.size();
    }


    /** @brief Returns the priority of the edge
     * @return This edge's priority
     */
    int getPriority() const throw() {
        return myPriority;
    }


    /** @brief Returns the origin node of the edge
     * @return The node this edge starts at
     */
    NBNode * const getFromNode() const throw() {
        return myFrom;
    }


    /** @brief Returns the destination node of the edge
     * @return The node this edge ends at
     */
    NBNode * const getToNode() const throw() {
        return myTo;
    }


    /** @brief Returns the angle of the edge
     *
     * The angle is computed within the constructor using NBHelpers::angle
     *
     * @return This edge's angle
     * @see NBHelpers::angle
     */
    SUMOReal getAngle() const throw() {
        return myAngle;
    }


    /** @brief Returns the computed length of the edge
     * @return The edge's computed length
     */
    SUMOReal getLength() const throw() {
        return myLength;
    }


    /** @brief Returns the speed allowed on this edge
     * @return The maximum speed allowed on this edge
     */
    SUMOReal getSpeed() const throw() {
        return mySpeed;
    }


    /** @brief The building step of this edge
     * @return The current building step for this edge
     * @todo Recheck usage!
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep getStep() const throw() {
        return myStep;
    }
    //@}



    /// @name Edge geometry access and computation
    //@{

    /** @brief Returns the geometry of the edge
     * @return The edge's geometry
     */
    const Position2DVector &getGeometry() const throw() {
        return myGeom;
    }


    /** @brief (Re)sets the edge's geometry
     *
     * Replaces the edge's prior geometry by the given. Then, computes
     *  the geometries of all lanes using computeLaneShapes.
     * Definitely not the best way to have it accessable from outside...
     * @param[in] g The edge's new geometry
     * @todo Recheck usage, disallow access
     * @see computeLaneShapes
     */
    void setGeometry(const Position2DVector &g) throw();


    /** @brief Adds a further geometry point
     *
     * Some importer do not know an edge's geometry when it is initialised.
     *  This method allows to insert further geometry points after the edge
     *  has been built.
     *
     * @param[in] index The position at which the point shall be added
     * @param[in] p The point to add
     */
    void addGeometryPoint(int index, const Position2D &p) throw();


    /** @brief Computes the shape of the edge (regarding the nodes' shapes
     *
     * Because an edge's shape should start/end at the boundaries of the nodes
     *  the edge starts/ends at, we have to recompute the edge's shape after
     *  we know the ones of the nodes. This is done within this method.
     * @todo Describe what is done here
     */
    void computeEdgeShape() throw();


    /** @brief Returns the shape of the nth lane
     * @return The shape of the lane given by its index (counter from right)
     */
    const Position2DVector &getLaneShape(unsigned int i) const throw();


    /** @brief (Re)sets how the lanes lateral offset shall be computed
     * @param[in] spread The type of lateral offset to apply
     * @see LaneSpreadFunction
     */
    void setLaneSpreadFunction(LaneSpreadFunction spread) throw();


    /** @brief Returns how this edge's lanes' lateral offset is computed
     * @return The type of lateral offset that is applied on this edge
     * @see LaneSpreadFunction
     */
    LaneSpreadFunction getLaneSpreadFunction() const throw() {
        return myLaneSpreadFunction;
    }


    /** @brief Resets this edge's position, so that the network starts at (0,0)
     *
     * GeoConvHelper::getOffset() is applied to both the edge's and all lanes'
     *  geometries.
     * @see GeoConvHelper::getOffset()
     */
    void normalisePosition() throw();


    /** @brief Applies an additional offset and rotation on the network
     * @param[in] xoff The x-offset to apply
     * @param[in] yoff The y-offset to apply
     * @param[in] rot The rotation to apply
     */
    void reshiftPosition(SUMOReal xoff, SUMOReal yoff, SUMOReal rot) throw();
    //@}



    /// @name Setting and getting connections
    /// @{

    /** @brief Returns connections from a given lane
     *
     * @param[in] lane The lane which connections shall be returned
     * @return The connections from the given lane
     * @see NBEdge::Connection
     */
    std::vector<Connection> getConnectionsFromLane(unsigned int lane) const throw();

    /// @}


    void setAsMacroscopicConnector() throw() {
        myAmMacroscopicConnector = true;
    }

    bool isMacroscopicConnector() throw() {
        return myAmMacroscopicConnector;
    }


    /// computes which edge shall be the turn-around one, if any
    void computeTurningDirections();


    /// sets the junction priority of the edge
    void setJunctionPriority(NBNode *node, int prio);

    /// returns the junction priority (normalised for the node currently build)
    int getJunctionPriority(NBNode *node);


    /** writes the edge definition with lanes and connected edges
        into the given stream */
    void writeXMLStep1(OutputDevice &into);

    /** writes the succeeding lane information */
    void writeXMLStep2(OutputDevice &into, bool includeInternal);

    bool hasRestrictions() const;
    void writeLanesPlain(OutputDevice &into);
    void setLoadedLength(SUMOReal val);
    void dismissVehicleClassInformation();


    /** adds a connection to another edge;
        instead of adding connections one by one, they may be computed
        using "computeEdge2Edges"
        returns false when an connection already existed; otherwise true */
    bool addEdge2EdgeConnection(NBEdge *dest);

    /** adds a connection between the specified this edge's lane and an approached one */
    bool addLane2LaneConnection(size_t fromLane, NBEdge *dest,
                                size_t toLane, Lane2LaneInfoType type,
                                bool mayUseSameDestination=false);

    /** builds no connections starting at the given lanes */
    bool addLane2LaneConnections(size_t fromLane,
                                 NBEdge *dest, size_t toLane, size_t no,
                                 Lane2LaneInfoType type, bool invalidatePrevious=false);

    /// computes the edge (step1: computation of approached edges)
    bool computeEdge2Edges();

    /// computes the edge, step2: computation of which lanes approach the edges)
    bool computeLanes2Edges();

    /// sorts the connections of outgoing lanes (!!! Kaskade beschreiben)
    void sortOutgoingLanesConnections();

    /** recheck whether all lanes within the edge are all right and
        optimises the connections once again */
    bool recheckLanes();

    /// appends turnarounds
    void appendTurnaround();

    /// returns the list of lanes that may be used to reach the given edge
    std::vector<int> getConnectionLanes(NBEdge *currentOutgoing) const;

    /// adds a connection to a certain lane of a certain edge
    void setConnection(size_t lane, NBEdge *destEdge,
                       size_t destLane,
                       Lane2LaneInfoType type,
                       bool mayUseSameDestination=false);

    /** returns the information whether the given edge is the opposite
        direction to this edge */
    bool isTurningDirectionAt(const NBNode *n, NBEdge *edge) const;

    bool isAlmostSameDirectionAt(const NBNode *n, NBEdge *edge) const;


    /** @brief Returns the node at the given edges length (using an epsilon)
        When no node is existing at the given position, 0 is returned
        The epsilon is a static member of NBEdge, should be setable via program options */
    NBNode *tryGetNodeAtPosition(SUMOReal pos, SUMOReal tolerance=5.0) const;

    void replaceInConnections(NBEdge *which, NBEdge *by, size_t laneOff);

    SUMOReal getMaxLaneOffset();

    Position2D getMinLaneOffsetPositionAt(NBNode *node, SUMOReal width) const;
    Position2D getMaxLaneOffsetPositionAt(NBNode *node, SUMOReal width) const;
    const std::vector<Connection> &getConnections() const {
        return myConnections;
    }
    std::vector<Connection> &getConnections() {
        return myConnections;
    }

    /// Returns the information whethe a connection to the given edge has been added (or computed)
    bool isConnectedTo(NBEdge *e);

    /** returns the list of outgoing edges without the turnaround
        sorted in clockwise direction */
    const std::vector<NBEdge*> *getConnectedSorted();

    /** returns the list of outgoing edges unsorted */
    std::vector<NBEdge*> getConnectedEdges() const throw();

    /** @brief Remaps the connection in a way tha allows the removal of it
        This edges (which is a "dummy" edge, in fact) connections are spread over the incoming non-dummy edges */
    void remapConnections(const EdgeVector &incoming);

    void removeFromConnections(NBEdge *which, int lane=-1);

    void invalidateConnections(bool reallowSetting=false);

    bool lanesWereAssigned() const;

    /// Returns if the link could be set as to be controlled
    bool setControllingTLInformation(int fromLane, NBEdge *toEdge, int toLane,
                                     const std::string &tlID, size_t tlPos);

    void addCrossingPointsAsIncomingWithGivenOutgoing(NBEdge *o,
            Position2DVector &into);

    SUMOReal width() const;

    Position2DVector getCWBoundaryLine(const NBNode &n, SUMOReal offset) const;
    Position2DVector getCCWBoundaryLine(const NBNode &n, SUMOReal offset) const;

    bool expandableBy(NBEdge *possContinuation) const;
    void append(NBEdge *continuation);
    SUMOReal getNormedAngle() const;

    bool hasSignalisedConnectionTo(NBEdge *e) const;


    /** friend class used for the computation of connections to
        following edges */
    friend class NBEdgeSuccessorBuilder;

    friend class NBEdgeCont;

    void moveOutgoingConnectionsFrom(NBEdge *e, size_t laneOff);

    NBEdge *getTurnDestination() const;

    std::string getLaneID(size_t lane);

    void setLaneSpeed(unsigned int lane, SUMOReal speed);

    SUMOReal getLaneSpeed(unsigned int lane) const;

    bool isNearEnough2BeJoined2(NBEdge *e);

    SUMOReal getAngle(const NBNode &atNode) const;

    SUMOReal getNormedAngle(const NBNode &atNode) const;


    void incLaneNo(unsigned int by);

    void decLaneNo(unsigned int by, int dir=0);

    void copyConnectionsFrom(NBEdge *src);

    void markAsInLane2LaneState();

    void allowVehicleClass(int lane, SUMOVehicleClass vclass);
    void disallowVehicleClass(int lane, SUMOVehicleClass vclass);
    std::vector<SUMOVehicleClass> getAllowedVehicleClasses() const;
    void setVehicleClasses(const std::vector<SUMOVehicleClass> &allowed, const std::vector<SUMOVehicleClass> &disallowed, int lane=-1);

    void disableConnection4TLS(int fromLane, NBEdge *toEdge, int toLane);

    void recheckEdgeGeomForDoublePositions();

    int getMinConnectedLane(NBEdge *of) const;
    int getMaxConnectedLane(NBEdge *of) const;

    void setTurningDestination(NBEdge *e);

    void setIsInnerEdge() {
        myAmInnerEdge = true;
    }

    bool isInnerEdge() const {
        return myAmInnerEdge;
    }

private:
    /**
     * ToEdgeConnectionsAdder
     * A class that being a bresenham-callback assigns the incoming
     * lanes to the edges
     */
class ToEdgeConnectionsAdder : public Bresenham::BresenhamCallBack
    {
    private:
        /// map of edges to this edge's lanes that reach them
        std::map<NBEdge*, std::vector<size_t> > myConnections;

        /// the transition from the virtual lane to the edge it belongs to
        const std::vector<NBEdge*> &myTransitions;

    public:
        /// constructor
        ToEdgeConnectionsAdder(const std::vector<NBEdge*> &transitions) throw()
                : myTransitions(transitions) { }

        /// destructor
        ~ToEdgeConnectionsAdder() throw() { }

        /// executes a bresenham - step
        void execute(SUMOReal lane, SUMOReal virtEdge) throw();

        const std::map<NBEdge*, std::vector<size_t> > &getBuiltConnections() const throw() {
            return myConnections;
        }

    private:
        /// @brief Invalidated copy constructor.
        ToEdgeConnectionsAdder(const ToEdgeConnectionsAdder&);

        /// @brief Invalidated assignment operator.
        ToEdgeConnectionsAdder& operator=(const ToEdgeConnectionsAdder&);

    };


    /**
     * MainDirections
     * Holds (- relative to the edge it is build from -!!!) the list of
     * main directions a vehicle that drives on this street may take on
     * the junction the edge ends in
     * The back direction is not regarded
     */
    class MainDirections
    {
    public:
        /// enum of possible directions
        enum Direction { DIR_RIGHTMOST, DIR_LEFTMOST, DIR_FORWARD };

        /** list of the main direction within the following junction relative
            to the edge */
        std::vector<Direction> myDirs;

    public:
        /// constructor
        MainDirections(const std::vector<NBEdge*> &outgoing,
                       NBEdge *parent, NBNode *to);

        /// destructor
        ~MainDirections();

        /** returns the information whether no following street has a higher
            priority */
        bool empty() const;

        /** returns the information whether the street in the given direction
            has a higher priority */
        bool includes(Direction d) const;

    private:
        /// @brief Invalidated copy constructor.
        MainDirections(const MainDirections&);

        /// @brief Invalidated assignment operator.
        MainDirections& operator=(const MainDirections&);

    };

    /// Computes the shape for the given lane
    Position2DVector computeLaneShape(size_t lane) throw(InvalidArgument);

    /// Computes the offset from the edge shape on the current segment
    std::pair<SUMOReal, SUMOReal> laneOffset(const Position2D &from,
            const Position2D &to, SUMOReal lanewidth, size_t lane) throw(InvalidArgument);

    void computeLaneShapes() throw(InvalidArgument);

    bool splitGeometry(NBEdgeCont &ec, NBNodeCont &nc);
protected:
    bool acceptBeingTurning(NBEdge *e);


private:
    /** @brief Initialization routines common to all constructors
     *
     * Checks whether the number of lanes>0, whether the junction's from-
     *  and to-nodes are given (!=0) and whether they are distict. Throws
     *  a ProcessError if any of these checks fails.
     *
     * Adds the nodes positions to geometry if it shall not be ignored or
     *  if the geometry is empty.
     *
     * Computes the angle and length, and adds this edge to its node as
     *  outgoing/incoming. Builds lane informations.
     *
     * @param[in] noLanes The number of lanes this edge has
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     */
    void init(unsigned int noLanes, bool tryIgnoreNodePositions) throw(ProcessError);


    /** divides the lanes on the outgoing edges */
    void divideOnEdges(const std::vector<NBEdge*> *outgoing);

    /** recomputes the priorities and manipulates them for a distribution
        of lanes on edges which is more like in real-life */
    std::vector<size_t> *preparePriorities(
        const std::vector<NBEdge*> *outgoing);

    /** computes teh sum of the given list's entries (sic!) */
    size_t computePrioritySum(std::vector<size_t> *priorities);

    /** moves a connection one place to the left;
        Attention! no checking for field validity */
    void moveConnectionToLeft(size_t lane);

    /** moves a connection one place to the right;
        Attention! no checking for field validity */
    void moveConnectionToRight(size_t lane);


    /** writes information about the described lane into the given stream */
    void writeLane(OutputDevice &into, NBEdge::Lane &lane, unsigned int index) const;

    // !!! describe
    void writeSucceeding(OutputDevice &into, size_t lane,
                         bool includeInternal);





    // !!! describe
    void writeSingleSucceeding(OutputDevice &into,
                               const NBEdge::Connection &c, bool includeInternal);




private:
    /** @brief The building step
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep myStep;

    /// @brief The id of the edge
    std::string myID;

    /// @brief The type of the edge
    std::string myType;

    /// @brief The source and the destination node
    NBNode *myFrom, *myTo;

    /// @brief The length of the edge
    SUMOReal myLength;

    /// @brief The angle of the edge
    SUMOReal myAngle;

    /// @brief The priority of the edge
    int myPriority;

    /// @brief The maximal speed
    SUMOReal mySpeed;

    /** @brief List of connections to following edges
     * @see Connection
     */
    std::vector<Connection> myConnections;

    /// @brief The turn destination edge
    NBEdge *myTurnDestination;

    /// @brief The priority normalised for the node the edge is outgoing of
    int myFromJunctionPriority;

    /// @brief The priority normalised for the node the edge is incoming in
    int myToJunctionPriority;

    /// @brief The geometry for the edge
    Position2DVector myGeom;

    /// @brief The information about how to spread the lanes
    LaneSpreadFunction myLaneSpreadFunction;

    /** @brief Lane information
     * @see Lane
     */
    std::vector<Lane> myLanes;

    /// @brief An optional length to use (-1 if not valid)
    SUMOReal myLoadedLength;


    /// @name Temporary variables for turning edge computation
    /// @{

    /// @brief Was assigned as a turn with this angle
    SUMOReal myAmTurningWithAngle;
    /// @brief Was assigned as a turning edge of this one
    NBEdge *myAmTurningOf;
    /// @}

    /// @brief Information whether this is a junction-inner edge
    bool myAmInnerEdge;

    /// @brief Information whether this edge is a (macroscopic) connector
    bool myAmMacroscopicConnector;

    struct TLSDisabledConnection {
        int fromLane;
        NBEdge *to;
        int toLane;
    };

    std::vector<TLSDisabledConnection> myTLSDisabledConnections;

    /**
     * @class tls_disable_finder
     */
    class tls_disable_finder
    {
    public:
        /// constructor
        tls_disable_finder(const TLSDisabledConnection &tpl) : myDefinition(tpl) { }

        bool operator()(const TLSDisabledConnection &e) const {
            if (e.to!=myDefinition.to) {
                return false;
            }
            if (e.fromLane!=myDefinition.fromLane) {
                return false;
            }
            if (e.toLane!=myDefinition.toLane) {
                return false;
            }
            return true;
        }

    private:
        TLSDisabledConnection myDefinition;

    };



    /**
     * @class connections_toedge_finder
     */
    class connections_toedge_finder
    {
    public:
        /// constructor
        connections_toedge_finder(NBEdge * const edge2find) : myEdge2Find(edge2find) { }

        bool operator()(const Connection &c) const {
            return c.toEdge == myEdge2Find;
        }

    private:
        NBEdge * const myEdge2Find;

    };

    /**
     * @class connections_toedgelane_finder
     */
    class connections_toedgelane_finder
    {
    public:
        /// constructor
        connections_toedgelane_finder(NBEdge * const edge2find, int lane2find) : myEdge2Find(edge2find), myLane2Find(lane2find) { }

        bool operator()(const Connection &c) const {
            return c.toEdge == myEdge2Find && c.toLane == myLane2Find;
        }

    private:
        NBEdge * const myEdge2Find;
        int myLane2Find;

    };
    /**
     * @class connections_finder
     */
    class connections_finder
    {
    public:
        /// constructor
        connections_finder(int fromLane, NBEdge * const edge2find, int lane2find) : myFromLane(fromLane), myEdge2Find(edge2find), myLane2Find(lane2find) { }

        bool operator()(const Connection &c) const {
            return c.fromLane==myFromLane && c.toEdge == myEdge2Find && c.toLane == myLane2Find;
        }

    private:
        int myFromLane;
        NBEdge * const myEdge2Find;
        int myLane2Find;

    };

    /**
     * @class connections_fromlane_finder
     */
    class connections_fromlane_finder
    {
    public:
        /// constructor
        connections_fromlane_finder(int lane2find) : myLane2Find(lane2find) { }

        bool operator()(const Connection &c) const {
            return c.fromLane == myLane2Find;
        }

    private:
        int myLane2Find;

    };

    /**
     * @class connections_sorter
     */
    class connections_sorter
    {
    public:
        /// @brief constructor
        explicit connections_sorter() { }

        int operator()(const Connection &c1, const Connection &c2) const {
            if (c1.fromLane!=c2.fromLane) {
                return c1.fromLane<c2.fromLane;
            }
            return c1.toLane<c2.toLane;
        }

    };

    /**
     * connections_relative_edgelane_sorter
     * Class to sort edges by their angle
     */
    class connections_relative_edgelane_sorter
    {
    public:
        /// constructor
        explicit connections_relative_edgelane_sorter(NBEdge *e, NBNode *n)
                : myEdge(e), myNode(n) {}

    public:
        /// comparing operation
        int operator()(const Connection &c1, const Connection &c2) const {
            if (c1.toEdge!=c2.toEdge) {
                SUMOReal relAngle1 = NBHelpers::normRelAngle(
                                         myEdge->getAngle(), c1.toEdge->getAngle());
                SUMOReal relAngle2 = NBHelpers::normRelAngle(
                                         myEdge->getAngle(), c2.toEdge->getAngle());
                return relAngle1 > relAngle2;
            }
            return c1.toLane<c2.toLane;
        }

    private:
        /// the edge to compute the relative angle of
        NBEdge *myEdge;

        /// the node to use
        NBNode *myNode;

    };

private:
    /// @brief invalidated copy constructor
    NBEdge(const NBEdge &s);

    /// @brief invalidated assignment operator
    NBEdge &operator=(const NBEdge &s);


};


#endif

/****************************************************************************/

