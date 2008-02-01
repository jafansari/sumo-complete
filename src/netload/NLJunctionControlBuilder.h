/****************************************************************************/
/// @file    NLJunctionControlBuilder.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 9 Jul 2001
/// @version $Id$
///
// Builder of microsim-junctions and tls
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
#ifndef NLJunctionControlBuilder_h
#define NLJunctionControlBuilder_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <vector>
#include <microsim/MSJunction.h>
#include <microsim/MSRightOfWayJunction.h>
#include <microsim/MSJunctionControl.h>
#include <utils/geom/Position2D.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/traffic_lights/MSActuatedTrafficLightLogic.h>
#include <microsim/MSBitSetLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class MSEdgeContinuations;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLJunctionControlBuilder
 * @brief Builder of microsim-junctions and tls
 *
 * NLJunctionControlBuilder is a factory for MSJunction, MSJunctionLogic, WAUT,
 *  and MSTRafficLightLogic-instances.
 *
 * @todo Refactor this class - it's too large
 * @todo Resort method by one of the topics.
 */
class NLJunctionControlBuilder
{
private:
    /// @brief Definition of a lane vector
    typedef std::vector<MSLane*> LaneVector;

public:
    /** @brief Constructor
     *
     * Stores default values for extended tls reading them from the given 
     *  options. Builds a MSTLLogicControl instance for myLogicControl.
     *
     * @param[in] net The network to fill
     * @param[in] oc The options to use
     * @todo Why are options not const?
     */
    NLJunctionControlBuilder(MSNet &net, OptionsCont &oc) throw();


    /** @brief Destructor
     *
     * Deletes previously allocated "myLogicControl" and "myJunctions" if
     *  they were not previously returned (this may happen if an error occured).
     */
    virtual ~NLJunctionControlBuilder() throw();


    /** @brief Preallocates space for the found number of junctions
     * 
     * unused!
     * @param[in] no The number of junctions within the net
     * @todo This method is completely useless; the built structure does not take the parameter, may be built in the constructor
     */
    void prepare(unsigned int no) throw();


    /** @brief Begins the processing of the named junction
     *
     * @param[in] id The ID of the junction
     * @param[in] key unused?!
     * @param[in] type The type of the junction
     * @param[in] x x-position of the junction
     * @param[in] y y-position of the junction
     * @exception InvalidArgument If the junction type is not known
     * @todo Check why "key" is given
     */
    void openJunction(const std::string &id, const std::string &key,
                      const std::string &type, SUMOReal x, SUMOReal y) throw(InvalidArgument);


    /** @brief Adds an incoming lane to the previously chosen junction
     *
     * @param[in] lane The lane to add to the junction as being an incoming lane
     */
    void addIncomingLane(MSLane *lane) throw();


#ifdef HAVE_INTERNAL_LANES
    /** @brief Adds an internal lane to the previously chosen junction
     *
     * @param[in] lane The lane to add to the junction as being an internal lane
     */
    void addInternalLane(MSLane *lane) throw();
#endif


    /** @brief Closes (ends) the processing of the current junction
     *
     * This method throws an InvalidArgument when a junction with the same id 
     *  as the current was already added or if the junction type stored in "myType"
     *  is invalid. It throws a ProcessError if the container to store the
     *  junction in was not built before.
     *
     * @exception InvalidArgument If the current id is already used or the junction type was invalid
     * @exception ProcessError If the container to store the junction in was not built before
     * @todo Throwing ProcessError would get unneeded if the container would be built by default (see prepare)
     * @todo The type of the junctions shoould probably be checked when supprted (in openJunction)
     */
    void closeJunction() throw(InvalidArgument, ProcessError);


    /** @brief Builds the MSJunctionControl which holds all of the simulations junctions 
     *
     * Returns the previously built junction control ("myJunctions"). "myJunctions" is
     *  set to 0, so that it will not be destroyed by the destructor.
     *
     * @return The built junction control
     */
    MSJunctionControl *build() const throw();


    /** @brief Clears the inlanes-container
     *
     * @todo It seems as the container would be used in several ways; Recheck!
     */
    void initIncomingLanes() throw();

    
    /** @brief Initialises a junction logic
     *
     * Resets all internal values that describe a junction logic to allow resetting them from
     *  read values.
     */
    void initJunctionLogic() throw();


    /** @brief Adds a logic item
     *
     * Adds a logic item to the current processed logic. Throws an InvalidArgument
     *  if the current request size (myRequestSize) is 0 or lower.
     *
     * @param[in] request The request (~link) index
     * @param[in] response The response (list of higher priorised links)
     * @param[in] foes List of foes to the request
     * @param[in] cont Whether the request is followed by an internal end
     * @todo Recheck "cont"; is the description correct?
     */
    void addLogicItem(int request, const std::string &response,
                      const std::string &foes, bool cont) throw(InvalidArgument);


    /** @brief Begins the reading of a traffic lights logic
     *
     * Resets all internal values that describe a traffic lights logic to allow
     *  resetting them from read values.
     *
     * @param[in] type The type of the tls 
     * @param[in] absDuration The absolute duration (cycle time)
     * @param[in] requestSize The size of the tls request 
     * @param[in] detectorOffset The offset of the detectors to build
     * @todo Why is the type not verified?
     * @todo Recheck, describe usage of absDuration (where does the information come from?)
     * @todo Recheck, describe usage of requestSize (where does the information come from?)
     * @todo Recheck, describe usage of detectorOffset (where does the information come from?)
     * @todo detectorOffset is used only by one junction type. Is it not possible, to remove this from the call?
     */
    void initTrafficLightLogic(const std::string &type,
                               size_t absDuration, int requestSize, 
                               SUMOReal detectorOffset) throw();


    /** @brief Adds a phase to the currently built traffic lights logic
     *
     * @param[in] duration The duration of the phase
     * @param[in] phase The green lights definition of the phase
     * @param[in] prios The priority (halting) definition of the phase
     * @param[in] yellow The yellow lights definition of the phase
     * @param[in] min The minimum duration of the phase
     * @param[in] max The maximum duration of the phase
     * @todo min/max is used only by one junction type. Recheck
     * @todo min/max: maybe only one type of a phase definition should be built
     * @todo Check why the duration is not SUMOTime
     */
    void addPhase(size_t duration, const std::bitset<64> &phase,
                  const std::bitset<64> &prios, const std::bitset<64> &yellow,
                  int min, int max) throw();


    /** @brief Sets the size of the request
     *
     * @param[in] size The size of the request
     * @todo Where does this information come from?
     */
    void setRequestSize(int size) throw();


    /** @brief Sets the lane number the parsed logic will be responsible for
     *
     * @param[in] size The number of lanes controlled by the junction
     * @todo This information is not used anymore!
     */
    void setLaneNumber(int size) throw();


    /** @brief Set the key of the logic
     *
     * @param[in] key The key of the logic
     */
    void setKey(const std::string &key) throw();


    /** @brief Set the subkey of the logic
     *
     * @param[in] subkey The subkey of the logic
     */
    void setSubKey(const std::string &subkey) throw();


    /** @brief Set the offset with which the logic shall start
     *
     * @param[in] val The offset with which the logic shall start
     * @todo Check why the offset is not SUMOTime
     */
    void setOffset(int val) throw();


    /** @brief Returns a previously build tls logic
     *
     * @param[in] id The ID of the tls logic to return
     * @return The named logic
     * @exception InvalidArgument If the named tls logic was not built before
     */
    MSTLLogicControl::TLSLogicVariants &getTLLogic(const std::string &id) 
        const throw(InvalidArgument);


    /** @brief Returns the built tls-logic control
     *
     * Returns the previously built junction control ("myLogicControl"). "myLogicControl" is
     *  set to 0, so that it will not be destroyed by the destructor.
     *
     * @return The built tls-logic control
     */
    MSTLLogicControl *buildTLLogics() const throw();


    /** @brief Ends the building of a traffic lights logic
     *
     * Builds the correct type of a MSTrafficLightLogic using the stored information.
     *  Tries to add it to the used tls control. Throws an InvalidArgument if 
     *  this is not possible (another tls logic with the same name exists).
     * 
     * @exception InvalidArgument If another tls logic with the same name as the currently built was loaded before
     */
    virtual void closeTrafficLightLogic() throw(InvalidArgument);


    /** @brief Ends the building of a junction logic (row-logic)
     *
     * Rechecks values for the request and builds a MSJunctionLogic using these values.
     *  Throws and InvalidArgument if the values are invalid (error message is
     *  included). 
     * Tries to add the built logic to the internal container "myLogics". If another
     *  logic with the same id exists, an InvalidArgument is thrown.
     *
     * @exception InvalidArgument If the logic's values are false or another logic with the same id was built before
     */
    void closeJunctionLogic() throw(InvalidArgument);


    /** @brief Closes the building of the junction control
     *
     * Sets loaded junction parameter into all tls logics.
     *
     * @param[in] db The detector builder to use (unused!)
     * @param[in] edgeContinuations Edge continuations to use (unused!)
     * @todo Really nasty! Both parameter are not used herein.
     * @todo Where do extended tl build their detectors, what value are set herein?
     * @todo May the called method throw something?
     */
    void closeJunctions(NLDetectorBuilder &db,
                        const MSEdgeContinuations &edgeContinuations) throw();


    /** @brief Adds a parameter
     *
     * @param[in] key The key of the parameter
     * @param[in] value The value of the parameter
     * @todo Where are these parameter used? Describe!
     * @todo Can a parameter be overwritten?
     */
    void addParam(const std::string &key, const std::string &value) throw();


    /** @brief Adds a WAUT definition
     *
     * Passes the values directly to the used tls control. This throws an InvalidArgument
     *  if the given id is already in use.
     *
     * @param[in] refTime The reference time of the WAUT
     * @param[in] id The ID of the WAUT
     * @param[in] startProg The begin program of the WAUT
     * @exception InvalidArgument If the id is already used by another WAUT
     */
    void addWAUT(SUMOTime refTime, const std::string &id, 
        const std::string &startProg) throw(InvalidArgument);


    /** @brief Adds a WAUT switch step to a previously built WAUT
     *
     * Passes the values directly to the used tls control. This throws an InvalidArgument
     *  if the given WAUT id is not known.
     *
     * @param[in] wautid The ID of the WAUT
     * @param[in] when The switch procedure begin
     * @param[in] to The program the WAUT shall start to switch to at the given time
     * @exception InvalidArgument If the named WAUT is not known
     */
    void addWAUTSwitch(const std::string &wautid, SUMOTime when, 
        const std::string &to) throw(InvalidArgument);


    /** @brief Adds a tls to the list of tls to be switched by the named WAUT
     *
     * Passes the values directly to the used tls control. This throws an InvalidArgument
     *  if the given WAUT id or the given junction id is not known.
     *
     * @param[in] wautid The ID of the WAUT
     * @param[in] tls The id of the tls to be switched
     * @param[in] proc The switching procedure to use
     * @param[in] synchron Whether the switching shall be done in synchron mode
     * @exception InvalidArgument If the named WAUT or the named tls are not known
     */
    void addWAUTJunction(const std::string &wautid, const std::string &tls,
                         const std::string &proc, bool synchron) throw(InvalidArgument);


    /** @brief Returns the active name
     * @return The active id
     */
    const std::string &getActiveID() const throw();


    /** @brief Returns the active key
     * @return The active key
     */
    const std::string &getActiveKey() const throw();


    /** @brief Returns the active sub key
     * @return The active sub key
     */
    const std::string &getActiveSubKey() const throw();


protected:
    /** @brief Adds an information about the initialisation of a tls
     *
     * The initialisation is done during the closing of junctions
     *
     * @param[in] key The tls logic to add a new initialisation info for
     * @todo Why is this a separate method? It's only used in one place
     * @todo No checks/exceptions?
     */
    void addJunctionInitInfo(MSTrafficLightLogic *key) throw();


    /** @brief Returns the current junction logic
     *
     * "Current" means the one with "myActiveID". If it is not built yet 
     *  (not within "myLogics") an InvalidArgument is thrown.
     * 
     * @return The current tls logic
     * @exception InvalidArgument If the logic was not built before
     * @todo Where is this used?
     */
    MSJunctionLogic *getJunctionLogicSecure() throw(InvalidArgument);


    /** @brief Computes the initial step of a tls-logic from the stored offset and duration
     *
     * @return The step the current tls has to begin with given current values (including simulation begin time)
     */
    SUMOTime computeInitTLSStep() const throw();


    /** @brief Computes the time offset the tls shall for the first time
     *
     * @return The time to first switch of the current tls, given current values (including simulation begin time)
     */
    SUMOTime computeInitTLSEventOffset() const throw();


    /** @brief Returns the used tls control
     *
     * This may be either the internal one ("myLogicControl"), or, if the network has
     *  been already loaded and we are loading additional tls programs, the net's logic control.
     *
     * @return The tls control to use
     */
    MSTLLogicControl &getTLLogicControlToUse() const throw();


protected:
    /// @name Factory methods, virtual so that other versions of the structures can be built
    /// @{

    /** @brief Builds a junction that does not use a logic
     * 
     * Builds a MSNoLogicJunction
     * 
     * @return The built junction
     */
    virtual MSJunction *buildNoLogicJunction() throw();


    /** @brief Builds a junction with a logic
     * 
     * Builds a MSRightOfWayJunction. Throws an exception if the logic was not built
     *  (see getJunctionLogicSecure).
     * 
     * @return The built junction
     * @exception InvalidArgument If the logic of the junction was not built before
     */
    virtual MSJunction *buildLogicJunction() throw(InvalidArgument);


#ifdef HAVE_INTERNAL_LANES
    /** @brief Builds an internal junction
     * 
     * Builds a MSInternalJunction
     * 
     * @return The built junction
     */
    virtual MSJunction *buildInternalJunction() throw();
#endif
    /// @}


protected:
    /// @brief The net to use
    MSNet &myNet;

    /// @brief The switch offset within the tls
    SUMOTime myOffset;

    /// @brief The current logic type
    std::string myLogicType;

    /// @brief The right-of-way-logic of the currently chosen bitset-logic
    MSBitsetLogic::Logic *myActiveLogic;

    /// @brief The description about which lanes disallow other passing the junction simultaneously
    MSBitsetLogic::Foes *myActiveFoes;

    /// @brief The description about which lanes have an internal follower
    std::bitset<64> myActiveConts;

    /// @brief The current phase definitions for a simple traffic light
    MSSimpleTrafficLightLogic::Phases myActivePhases;

    /// @brief The size of the request
    int myRequestSize;

    /// @brief The number of lanes
    int myLaneNumber;

    /// @brief Counter for the inserted items
    int myRequestItemNumber;

    /// @brief The junctions controls
    mutable MSJunctionControl *myJunctions;

    /// @brief The list of the incoming lanes of the currently chosen junction
    LaneVector myActiveIncomingLanes;

#ifdef HAVE_INTERNAL_LANES
    /// @brief The list of the internal lanes of the currently chosen junction
    LaneVector myActiveInternalLanes;
#endif

    /// @brief The id of the currently chosen junction
    std::string myActiveID;

    /// @brief The key of the currently chosen junction
    std::string myActiveKey, myActiveSubKey;

    /// @brief The type of the currently chosen junction
    int myType;

    /// @brief The position of the junction
    Position2D myPosition;

    /// @brief The absolute duration of a tls-control loop
    size_t myAbsDuration;

    /// @brief A definition of junction initialisation
    struct TLInitInfo {
        /// @brief The logic to initialise
        MSTrafficLightLogic *logic;
        /// @brief The loaded logic's parameter
        std::map<std::string, std::string> params;
    };

    /// @brief The container for information which junctions shall be initialised using which values
    std::vector<TLInitInfo> myJunctions2PostLoadInit;

    /// @name Default parameter for extended tls
    /// @todo Recheck, move to somewhere else
    /// @{

    /// @brief Default detector offset
    SUMOReal myDetectorOffset;

    /// @brief Default detector positions
    SUMOReal myStdDetectorPositions;

    /// @brief Default detector lengths (agentbased)
    SUMOReal myStdDetectorLengths;

    /// @brief Default learning horizon (agentbased)
    int myStdLearnHorizon;

    /// @brief Default decision horizon (agentbased)
    int myStdDecisionHorizon;

    /// @brief Default difference minimum (agentbased)
    SUMOReal myStdDeltaLimit;

    /// @brief The loaded default cycle time
    int myStdTCycle;

    // @brief Default maximum gap  (actuated)
    SUMOReal myStdActuatedMaxGap;

    // @brief Default passing time (actuated)
    SUMOReal myStdActuatedPassingTime;

    // @brief Default maximum gap (actuated)
    SUMOReal myStdActuatedDetectorGap;
    /// @}


    /// @brief The tls control to use (0 if net's tls control shall be used)
    mutable MSTLLogicControl *myLogicControl;


    /// @brief Definition of a parameter map (key->value)
    typedef std::map<std::string, std::string> StringParameterMap;

    /// @brief Parameter map (key->value)
    StringParameterMap myAdditionalParameter;


    /// @brief Map of loaded junction logics
    std::map<std::string, MSJunctionLogic*> myLogics;


protected:
    /// @name numerical representations of junction types
    /// @todo What? Use an enum!
    /// @{ 

    /// @brief numerical representation for a junction with no purpose
    static const int TYPE_NOJUNCTION;

    /** @brief numerical representation for a junction where vehicles coming from the right side may drive as first */
    static const int TYPE_RIGHT_BEFORE_LEFT;

    /** @brief numerical representation of a junction where a street has a higher priority */
    static const int TYPE_PRIORITY_JUNCTION;

    /** @brief a dead end (all roads end here) */
    static const int TYPE_DEAD_END;

    /** @brief an internal junction */
    static const int TYPE_INTERNAL;
    /// @}


private:
    /** @brief invalid copy operator */
    NLJunctionControlBuilder(const NLJunctionControlBuilder &s);

    /** @brief invalid assignment operator */
    NLJunctionControlBuilder &operator=(const NLJunctionControlBuilder &s);

};


#endif

/****************************************************************************/

