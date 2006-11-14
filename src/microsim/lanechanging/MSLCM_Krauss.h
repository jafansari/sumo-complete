#ifndef MSLCM_Krauss_h
#define MSLCM_Krauss_h
//---------------------------------------------------------------------------//
//                        MSLCM_Krauss.h -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Fri, 29.04.2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
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
// Revision 1.6  2006/11/14 13:02:36  dkrajzew
// warnings removed
//
// Revision 1.5  2005/10/07 11:37:47  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.4  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
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

#include <microsim/MSAbstractLaneChangeModel.h>



class MSLCM_Krauss : public MSAbstractLaneChangeModel {
public:
    MSLCM_Krauss(MSVehicle &v);

    virtual ~MSLCM_Krauss();

    /** @brief Called to examine whether the vehicle wants to change to right
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToRight(
        MSAbstractLaneChangeModel::MSLCMessager &msgPass, int blocked,
        const MSVehicle * const leader,
        const MSVehicle * const neighLead,
        const MSVehicle * const neighFollow,
        const MSLane &neighLane,
        int bestLaneOffset, SUMOReal bestDist,
        SUMOReal currentDist);

    /** @brief Called to examine whether the vehicle wants to change to left
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToLeft(
        MSAbstractLaneChangeModel::MSLCMessager &msgPass, int blocked,
        const MSVehicle * const leader,
        const MSVehicle * const neighLead,
        const MSVehicle * const neighFollow,
        const MSLane &neighLane,
//        bool congested, bool predInteraction,
        int bestLaneOffset, SUMOReal bestDist,
        SUMOReal currentDist);

    virtual void inform(LaneChangeAction , MSVehicle * /*sender*/) { }


};
/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
