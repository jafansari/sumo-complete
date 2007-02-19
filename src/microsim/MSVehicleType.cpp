/****************************************************************************/
/// @file    MSVehicleType.cpp
/// @author  Christian Roessel
/// @date    Tue, 06 Mar 2001
/// @version $Id$
///
// parameters.
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

#include "MSVehicleType.h"
#include "MSNet.h"
#include <cassert>
#include <utils/bindevice/BinaryInputDevice.h>
#include <utils/common/FileHelpers.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// static member definitions
// ===========================================================================
MSVehicleType::DictType MSVehicleType::myDict;
SUMOReal MSVehicleType::myMinDecel  = 0;
SUMOReal MSVehicleType::myMaxLength = 0;


// ===========================================================================
// method definitions
// ===========================================================================
MSVehicleType::~MSVehicleType()
{}


MSVehicleType::MSVehicleType(const string &id, SUMOReal length,
                             SUMOReal maxSpeed, SUMOReal accel,
                             SUMOReal decel, SUMOReal dawdle,
                             SUMOVehicleClass vclass)
        : myID(id), myLength(length), myMaxSpeed(maxSpeed), myAccel(accel),
        myDecel(decel), myDawdle(dawdle), myTau(1), myVehicleClass(vclass)
{
    assert(myLength > 0);
    assert(myMaxSpeed > 0);
    assert(myAccel > 0);
    assert(myDecel > 0);
    assert(myDawdle >= 0 && myDawdle <= 1);
    if (myMinDecel==0 || myDecel<myMinDecel) {
        myMinDecel = myDecel;
    }
    if (myLength>myMaxLength) {
        myMaxLength = myLength;
    }
    myInverseTwoDecel = SUMOReal(1) / (SUMOReal(2) * myDecel);
}


bool
MSVehicleType::dictionary(string id, MSVehicleType* vehType)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        myDict.insert(DictType::value_type(id, vehType));
        return true;
    }
    return false;
}


MSVehicleType*
MSVehicleType::dictionary(string id)
{
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        return 0;
    }
    return it->second;
}


void
MSVehicleType::clear()
{
    for (DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete(*i).second;
    }
    myDict.clear();
}


MSVehicleType*
MSVehicleType::dict_Random()
{
    assert(myDict.size()!=0);
    size_t r = (size_t)((SUMOReal) rand() / RAND_MAX * myDict.size());
    if (r>=myDict.size()) {
        r = myDict.size() - 1;
    }
    for (DictType::iterator i=myDict.begin(); i!=myDict.end(); i++, r-=1) {
        if (r==0) {
            return (*i).second;
        }
    }
    return myDict.begin()->second;
}


const std::string &
MSVehicleType::getID() const
{
    return myID;
}


void
MSVehicleType::dict_saveState(std::ostream &os, long what)
{
    FileHelpers::writeUInt(os, myDict.size());
    for (DictType::iterator it=myDict.begin(); it!=myDict.end(); ++it) {
        (*it).second->saveState(os, what);
    }
}


void
MSVehicleType::saveState(std::ostream &os, long /*what*/)
{
    FileHelpers::writeString(os, myID);
    FileHelpers::writeFloat(os, myLength);
    FileHelpers::writeFloat(os, myMaxSpeed);
    FileHelpers::writeFloat(os, myAccel);
    FileHelpers::writeFloat(os, myDecel);
    FileHelpers::writeFloat(os, myDawdle);
    FileHelpers::writeInt(os, (int) myVehicleClass);
}


void
MSVehicleType::dict_loadState(BinaryInputDevice &bis, long /*what*/)
{
    unsigned int size;
    bis >> size;
    while (size-->0) {
        string id;
        SUMOReal length, maxSpeed, accel, decel, dawdle;
        int vclass;
        bis >> id;
        bis >> length;
        bis >> maxSpeed;
        bis >> accel;
        bis >> decel;
        bis >> dawdle;
        bis >> vclass;
        MSVehicleType *t = new MSVehicleType(id, length, maxSpeed, accel, decel, dawdle, (SUMOVehicleClass) vclass);
        dictionary(id, t);
    }
}



/****************************************************************************/

