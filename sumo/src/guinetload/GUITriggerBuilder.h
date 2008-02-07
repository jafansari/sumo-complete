/****************************************************************************/
/// @file    GUITriggerBuilder.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 26.04.2004
/// @version $Id$
///
// Builds trigger objects for guisim
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
#ifndef GUITriggerBuilder_h
#define GUITriggerBuilder_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <netload/NLTriggerBuilder.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSTrigger;
class MSNet;
class MSLaneSpeedTrigger;
class MSTriggerControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUITriggerBuilder
 * @brief Builds trigger objects for guisim
 */
class GUITriggerBuilder : public NLTriggerBuilder
{
public:
    /// @brief Constructor
    GUITriggerBuilder() throw();


    /// @brief Destructor
    ~GUITriggerBuilder() throw();


protected:
    /// @name building methods
    ///
    /// Override NLTriggerBuilder-methods for building guisim-objects
    /// @see NLTriggerBuilder
    //@{

    /** @brief Builds a lane speed trigger
     *
     * Simply calls the GUILaneSpeedTrigger constructor.
     *
     * @param[in] net The net the lane speed trigger belongs to
     * @param[in] id The id of the lane speed trigger
     * @param[in] destLanes List of lanes affected by this speed trigger
     * @param[in] file Name of the file to read the speeds to set from
     * @return The built lane speed trigger
     * @see MSLaneSpeedTrigger
     * @exception ProcessError If the XML definition file is errornous
     */
    virtual MSLaneSpeedTrigger *buildLaneSpeedTrigger(MSNet &net,
            const std::string &id, const std::vector<MSLane*> &destLanes,
            const std::string &file) throw(ProcessError);


    /** @brief Builds an emitter
     *
     * Simply calls the GUIEmitter constructor.
     *
     * @param[in] net The net the emitter belongs to
     * @param[in] id The id of the emitter
     * @param[in] destLane The lane the emitter is placed on
     * @param[in] pos Position of the emitter on the given lane
     * @param[in] file Name of the file to read the emission definitions from
     * @return The built emitter
     */
    virtual MSEmitter *buildLaneEmitTrigger(MSNet &net,
                                            const std::string &id, MSLane *destLane, SUMOReal pos,
                                            const std::string &file) throw();


    /** @brief builds an rerouter
     *
     * Simply calls the GUITriggeredRerouter constructor.
     *
     * @param[in] net The net the rerouter belongs to
     * @param[in] id The id of the rerouter
     * @param[in] edges The edges the rerouter is placed at
     * @param[in] prob The probability the rerouter reoutes vehicles with
     * @param[in] file The file to read the reroute definitions from
     * @return The built rerouter
     */
    virtual MSTriggeredRerouter *buildRerouter(MSNet &net,
            const std::string &id, std::vector<MSEdge*> &edges,
            SUMOReal prob, const std::string &file) throw();


    /** @brief builds a vehicle actor
     *
     * Simply calls the GUIE1VehicleActor constructor.
     *
     * @param[in] net The net the actor belongs to
     * @param[in] id The id of the actor
     * @param[in] edges The edges the rerouter is placed at
     * @param[in] prob The probability the rerouter reoutes vehicles with
     * @param[in] file The file to read the reroute definitions from
     * @return The built vehicle actor
     * @todo Recheck usage of TOL-actors
     */
    virtual MSE1VehicleActor *buildVehicleActor(MSNet &, const std::string &id,
            MSLane *lane, SUMOReal pos, unsigned int la,
            unsigned int cell, unsigned int type) throw();


    /** @brief Builds a bus stop
     *
     * Simply calls the GUIBusStop constructor.
     *
     * @param[in] net The net the bus stop belongs to
     * @param[in] id The id of the bus stop
     * @param[in] lines Names of the bus lines that halt on this bus stop
     * @param[in] lane The lane the bus stop is placed on
     * @param[in] frompos Begin position of the bus stop on the lane
     * @param[in] topos End position of the bus stop on the lane
     * @return The built bus stop
     */
    virtual MSBusStop* buildBusStop(MSNet &net, const std::string &id,
                                    const std::vector<std::string> &lines, MSLane *lane,
                                    SUMOReal frompos, SUMOReal topos) throw();
    /// @}

};


#endif

/****************************************************************************/

