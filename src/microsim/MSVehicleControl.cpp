/***************************************************************************
                          MSVehicleControl.cpp  -
    The class responsible for building and deletion of vehicles
                             -------------------
    begin                : Wed, 10. Dec 2003
    copyright            : (C) 2002 by DLR http://ivf.dlr.de
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.14  2006/08/02 11:58:23  dkrajzew
// first try to make junctions tls-aware
//
// Revision 1.13  2006/07/06 06:06:30  dkrajzew
// made MSVehicleControl completely responsible for vehicle handling - MSVehicle has no longer a static dictionary
//
// Revision 1.12  2006/05/15 05:54:11  dkrajzew
// debugging saving/loading of states
//
// Revision 1.12  2006/05/08 11:06:59  dkrajzew
// debugging loading/saving of states
//
// Revision 1.11  2006/04/05 05:27:34  dkrajzew
// retrieval of microsim ids is now also done using getID() instead of id()
//
// Revision 1.10  2005/12/01 07:37:35  dkrajzew
// introducing bus stops: eased building vehicles; vehicles may now have nested elements
//
// Revision 1.9  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/05/04 08:35:40  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.5  2005/02/01 10:10:42  dkrajzew
// got rid of MSNet::Time
//
// Revision 1.4  2004/11/23 10:20:11  dkrajzew
// new detectors and tls usage applied; debugging
//
// Revision 1.3  2004/07/02 09:56:40  dkrajzew
// debugging while implementing the vss visualisation
//
// Revision 1.2  2004/04/02 11:36:28  dkrajzew
// "compute or not"-structure added; added two further simulation-wide output
//  (emission-stats and single vehicle trip-infos)
//
// Revision 1.1  2003/12/11 06:31:45  dkrajzew
// implemented MSVehicleControl as the instance responsible for vehicles
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

#include "MSCORN.h"
#include "MSVehicleControl.h"
#include "MSVehicle.h"
#include "MSSaveState.h"
#include "MSGlobals.h"
#include <utils/common/FileHelpers.h>
#include <utils/bindevice/BinaryInputDevice.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * some definitions (debugging only)
 * ======================================================================= */
#define DEBUG_OUT cout


/* =========================================================================
 * member method definitions
 * ======================================================================= */
MSVehicleControl::MSVehicleControl()
    : myLoadedVehNo(0), myEmittedVehNo(0), myRunningVehNo(0), myEndedVehNo(0),
    myAbsVehWaitingTime(0), myAbsVehTravelTime(0)
{
}


MSVehicleControl::~MSVehicleControl()
{
    for(VehicleDictType::iterator i=myVehicleDict.begin(); i!=myVehicleDict.end(); i++) {
        delete (*i).second;
    }
    myVehicleDict.clear();
}


MSVehicle *
MSVehicleControl::buildVehicle(std::string id, MSRoute* route,
                               SUMOTime departTime,
                               const MSVehicleType* type,
                               int repNo, int repOffset)
{
    myLoadedVehNo++;
    route->incReferenceCnt();
    return new MSVehicle(id, route, departTime, type,
        MSNet::getInstance()->getNDumpIntervalls(), repNo, repOffset);
}


void
MSVehicleControl::scheduleVehicleRemoval(MSVehicle *v)
{
    assert(myRunningVehNo>0);
    // check whether to generate the information about the vehicle's trip
    if(MSCORN::wished(MSCORN::CORN_OUT_TRIPDURATIONS)) {
        MSCORN::compute_TripDurationsOutput(v);
    }
    if(MSCORN::wished(MSCORN::CORN_OUT_VEHROUTES)) {
        MSCORN::compute_VehicleRouteOutput(v);
    }
    // check whether to save information about the vehicle's trip
    if(MSCORN::wished(MSCORN::CORN_MEAN_VEH_TRAVELTIME)) {
        myAbsVehTravelTime +=
            (MSNet::getInstance()->getCurrentTimeStep()
            -
            (long) v->getCORNDoubleValue(MSCORN::CORN_VEH_REALDEPART));
    }
    myRunningVehNo--;
    myEndedVehNo++;
    deleteVehicle(v);
}


void
MSVehicleControl::newUnbuildVehicleLoaded()
{
    myLoadedVehNo++;
}


void
MSVehicleControl::newUnbuildVehicleBuild()
{
    myLoadedVehNo--;
}



size_t
MSVehicleControl::getLoadedVehicleNo() const
{
    return myLoadedVehNo;
}


size_t
MSVehicleControl::getEndedVehicleNo() const
{
    return myEndedVehNo;
}


size_t
MSVehicleControl::getRunningVehicleNo() const
{
    return myRunningVehNo;
}


size_t
MSVehicleControl::getEmittedVehicleNo() const
{
    return myEmittedVehNo;
}


size_t
MSVehicleControl::getWaitingVehicleNo() const
{
    return myLoadedVehNo - myEmittedVehNo;
}


SUMOReal
MSVehicleControl::getMeanWaitingTime() const
{
    if(myEmittedVehNo==0) {
        return -1;
    }
    return (SUMOReal) myAbsVehWaitingTime / (SUMOReal) myEmittedVehNo;
}


SUMOReal
MSVehicleControl::getMeanTravelTime() const
{
    if(myEndedVehNo==0) {
        return -1;
    }
    return (SUMOReal) myAbsVehTravelTime / (SUMOReal) myEndedVehNo;
}


void
MSVehicleControl::vehiclesEmitted(size_t no)
{
    myEmittedVehNo += no;
    myRunningVehNo += no;
}


bool
MSVehicleControl::haveAllVehiclesQuit() const
{
    return myLoadedVehNo==myEndedVehNo;
}


void
MSVehicleControl::vehicleEmitted(MSVehicle *v)
{
    if(MSCORN::wished(MSCORN::CORN_MEAN_VEH_WAITINGTIME)) {
        myAbsVehWaitingTime +=
            ((long) v->getCORNDoubleValue(MSCORN::CORN_VEH_REALDEPART)
            -
            v->desiredDepart());
    }
}


void
MSVehicleControl::vehicleMoves(MSVehicle *v)
{
}


void
MSVehicleControl::saveState(std::ostream &os, long what)
{
    FileHelpers::writeUInt(os, myLoadedVehNo);
    FileHelpers::writeUInt(os, myEmittedVehNo);
    FileHelpers::writeUInt(os, myRunningVehNo);
    FileHelpers::writeUInt(os, myEndedVehNo);

    FileHelpers::writeInt(os, myAbsVehWaitingTime);
    FileHelpers::writeInt(os, myAbsVehTravelTime);
    MSVehicleType::dict_saveState(os, what);
    MSRoute::dict_saveState(os, what);
    //MSVehicle::dict_saveState(os, what);
    {
        FileHelpers::writeUInt(os, myVehicleDict.size());
        for(VehicleDictType::iterator it = myVehicleDict.begin(); it!=myVehicleDict.end(); ++it) {
            if((*it).second->hasCORNDoubleValue(MSCORN::CORN_VEH_REALDEPART)) {
                (*it).second->saveState(os, what);
            }
        }
        FileHelpers::writeString(os, "-----------------end---------------");
    }
}

void
MSVehicleControl::loadState(BinaryInputDevice &bis, long what)
{
    bis >> myLoadedVehNo;
    bis >> myEmittedVehNo;
    bis >> myRunningVehNo;
    bis >> myEndedVehNo;

    bis >> myAbsVehWaitingTime;
    bis >> myAbsVehTravelTime;

//    long t;
    //os >> t;
    MSVehicleType::dict_loadState(bis, what);
    MSRoute::dict_loadState(bis, what);
    {
        // load vehicles
        unsigned int size;
        bis >> size;
        string id;
        do {
            bis >> id;
            if(id!="-----------------end---------------") {
                SUMOTime lastLaneChangeOffset;
                bis >> lastLaneChangeOffset; // !!! check type= FileHelpers::readInt(os);
                unsigned int waitingTime;
                bis >> waitingTime;
                int repetitionNumber;
                bis >> repetitionNumber;
                int period;
                bis >> period;
                string routeID;
                bis >> routeID;
                MSRoute* route;
                unsigned int desiredDepart;
                bis >> desiredDepart;
                string typeID;
                bis >> typeID;
                const MSVehicleType* type;
                unsigned int routeOffset;
                bis >> routeOffset;
                unsigned int wasEmitted;
                bis >> wasEmitted;

                // !!! several things may be missing
                unsigned int segIndex;
                bis >> segIndex;
                SUMOReal tEvent;
                bis >> tEvent;
                SUMOReal tLastEntry;
                bis >> tLastEntry;

                //
                route = MSRoute::dictionary(routeID);
                route->incReferenceCnt();
                assert(route!=0);
                type = MSVehicleType::dictionary(typeID);
                assert(type!=0);
                if(getVehicle(id)!=0) {
                    DEBUG_OUT << "Error: vehicle was already added" << endl;
                    continue;
                }

                MSVehicle *v = MSNet::getInstance()->getVehicleControl().buildVehicle(id,
                    route, desiredDepart, type, repetitionNumber, period);
                v->myDoubleCORNMap[MSCORN::CORN_VEH_REALDEPART] = (SUMOReal) wasEmitted;
                while(routeOffset>0) {
                    v->myCurrEdge++;
                    routeOffset--;
                }
                if(!addVehicle(id, v)) {
                    cout << "Could not build vehicle!!!" << endl;
                    throw 1;
                }
                size--;
            }
        } while(id!="-----------------end---------------");
        DEBUG_OUT << myVehicleDict.size() << " vehicles loaded."; // !!! verbose
    }
//    MSVehicle::dict_loadState(bis, what);
    MSRoute::clearLoadedState();
}


bool
MSVehicleControl::addVehicle(const std::string &id, MSVehicle *v)
{
    VehicleDictType::iterator it = myVehicleDict.find(id);
    if (it == myVehicleDict.end()) {
        // id not in myVehicleDict.
        myVehicleDict[id] = v;//.insert(VehicleDictType::value_type(id, ptr));
        return true;
    }
    return false;
}


MSVehicle *
MSVehicleControl::getVehicle(const std::string &id)
{
    VehicleDictType::iterator it = myVehicleDict.find(id);
    if (it == myVehicleDict.end()) {
        // id not in myVehicleDict.
        return 0;
    }
    return it->second;
}


void
MSVehicleControl::deleteVehicle(const std::string &id)
{
    VehicleDictType::iterator i = myVehicleDict.find(id);
    MSVehicle *veh = (*i).second;
    myVehicleDict.erase(id);
    delete veh;
}


void
MSVehicleControl::deleteVehicle(MSVehicle *veh)
{
    deleteVehicle(veh->getID());
}


MSVehicle *
MSVehicleControl::detachVehicle(const std::string &id)
{
    VehicleDictType::iterator i = myVehicleDict.find(id);
    if(i==myVehicleDict.end()) {
        return 0;
    }
    MSVehicle *ret = (*i).second;
    myVehicleDict.erase(id);
    return ret;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

