#ifndef MSLaneSpeedTrigger_h
#define MSLaneSpeedTrigger_h
//---------------------------------------------------------------------------//
//                        MSLaneSpeedTrigger.h -
//  Class that realises the setting of a lane's maximum speed triggered by
//      values read from a file
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
// $Log$
// Revision 1.7  2006/03/17 08:58:36  dkrajzew
// changed the Event-interface (execute now gets the current simulation time, event handlers are non-static)
//
// Revision 1.6  2006/01/31 10:57:12  dkrajzew
// debugging
//
// Revision 1.5  2006/01/17 14:10:56  dkrajzew
// debugging
//
// Revision 1.4  2005/11/09 06:37:52  dkrajzew
// trigger reworked
//
// Revision 1.3  2005/10/07 11:37:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.2  2005/09/22 13:45:52  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.1  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.3  2005/04/26 08:11:49  dksumo
// level3 warnings patched; debugging
//
// Revision 1.2.2.1  2005/04/15 09:48:19  dksumo
// using a single SUMOTime type for time values; level3 warnings removed
//
// Revision 1.2  2005/01/06 10:48:06  dksumo
// 0.8.2.1 patches
//
// Revision 1.1  2004/10/22 12:49:29  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.4  2004/07/02 09:56:40  dkrajzew
// debugging while implementing the vss visualisation
//
// Revision 1.3  2003/09/23 14:18:15  dkrajzew
// hierarchy refactored; user-friendly implementation
//
// Revision 1.2  2003/02/07 10:41:51  dkrajzew
// updated
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <string>
#include <vector>
#include <utils/helpers/Command.h>
#include <utils/sumoxml/SUMOSAXHandler.h>
#include "MSTrigger.h"


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSNet;
class MSLane;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * MSLaneSpeedTrigger
 * This trigger reads the next maximum velocity of the lane he is
 * responsible for from a file and sets it.
 * Lanes with variable speeds are so possible.
 */
class MSLaneSpeedTrigger : public MSTrigger, public SUMOSAXHandler {
public:
    /** constructor */
    MSLaneSpeedTrigger(const std::string &id,
        MSNet &net, const std::vector<MSLane*> &destLanes,
        const std::string &aXMLFilename);

    /** destructor */
    virtual ~MSLaneSpeedTrigger();

    SUMOTime execute(SUMOTime currentTime);

    SUMOTime processCommand(bool move2next);

    SUMOReal getDefaultSpeed() const;

    void setOverriding(bool val);

    void setOverridingValue(SUMOReal val);

    SUMOReal getLoadedSpeed();

    /// Returns the current speed
    SUMOReal getCurrentSpeed() const;

protected:
    /** the implementation of the SAX-handler interface for reading
        element begins */
    virtual void myStartElement(int element, const std::string &name,
        const Attributes &attrs);

    /** the implementation of the SAX-handler interface for reading
        characters */
    void myCharacters(int element, const std::string &name,
        const std::string &chars);

    /** the implementation of the SAX-handler interface for reading
        element ends */
    void myEndElement(int element, const std::string &name);


protected:
    /// Define the container for those lanes that shall be manipulated
    typedef std::vector<MSLane*> LaneCont;

    /** the lane the trigger is responsible for */
    LaneCont myDestLanes;

    /** the speed that will be set on the next call */
    SUMOReal myCurrentSpeed;

    /// The original speed allowed on the lanes
    SUMOReal myDefaultSpeed;

    /// The information whether the read speed shall be overridden
    bool myAmOverriding;

    /// The speed to use if overriding the read speed
    SUMOReal mySpeedOverrideValue;

    std::vector<std::pair<SUMOTime, SUMOReal> > myLoadedSpeeds;
    std::vector<std::pair<SUMOTime, SUMOReal> >::iterator myCurrentEntry;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

