/***************************************************************************
                          MSVehicleType.cpp  -  Base Class for Vehicle
                          parameters.
                             -------------------
    begin                : Tue, 06 Mar 2001
    copyright            : (C) 2001 by ZAIK http://www.zaik.uni-koeln.de/AFS
    author               : Christian Roessel
    email                : roessel@zpr.uni-koeln.de
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
// Revision 1.17  2006/12/18 14:42:00  dkrajzew
// debugging nvwa usage
//
// Revision 1.16  2006/11/16 12:30:54  dkrajzew
// warnings removed
//
// Revision 1.15  2006/09/18 10:08:33  dkrajzew
// added vehicle class support to microsim
//
// Revision 1.14  2006/07/06 07:33:22  dkrajzew
// rertrieval-methods have the "get" prependix; EmitControl has no dictionary; MSVehicle is completely scheduled by MSVehicleControl; new lanechanging algorithm
//
// Revision 1.13  2006/05/15 05:54:11  dkrajzew
// debugging saving/loading of states
//
// Revision 1.13  2006/05/08 11:06:59  dkrajzew
// debugging loading/saving of states
//
// Revision 1.12  2006/03/17 09:01:12  dkrajzew
// .icc-files removed
//
// Revision 1.11  2005/10/07 11:37:45  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.10  2005/09/22 13:45:51  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.9  2005/09/15 11:10:46  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.8  2005/05/04 08:35:54  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; new mead data functionality; lane-changing offset computation debugged; simulation speed-up by avoiding multiplication with 1
//
// Revision 1.7  2003/10/17 06:52:01  dkrajzew
// acceleration is now time-dependent
//
// Revision 1.6  2003/07/30 10:02:38  dkrajzew
// support for the vehicle display removed by now
//
// Revision 1.5  2003/07/18 12:35:04  dkrajzew
// removed some warnings
//
// Revision 1.4  2003/05/20 09:31:47  dkrajzew
// emission debugged; movement model reimplemented (seems ok);
//  detector output debugged; setting and retrieval of some parameter added
//
// Revision 1.3  2003/02/07 10:41:50  dkrajzew
// updated
//
// Revision 1.2  2002/10/16 16:39:03  dkrajzew
// complete deletion within destructors implemented; clear-operator added
//  for container; global file include
//
// Revision 1.1  2002/10/16 14:48:26  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.5  2002/07/31 17:33:01  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.5  2002/07/31 14:41:05  croessel
// New methods return often used precomputed values.
//
// Revision 1.4  2002/06/06 07:21:10  croessel
// Changed inclusion from .iC to .icc
//
// Revision 1.3  2002/05/29 17:06:04  croessel
// Inlined some methods. See the .icc files.
//
// Revision 1.2  2002/04/11 15:25:56  croessel
// Changed SUMOReal to SUMOReal.
//
// Revision 1.1.1.1  2002/04/08 07:21:24  traffic
// new project name
//
// Revision 2.0  2002/02/14 14:43:20  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.9  2002/02/13 10:10:21  croessel
// Added two static methods: minDecel() and maxLength(). They are needed
// to calculate safe gaps in the case no predecessor resp. successor is
// known.
//
// Revision 1.8  2002/02/05 13:51:53  croessel
// GPL-Notice included.
// In *.cpp files also config.h included.
//
// Revision 1.7  2002/02/01 13:57:07  croessel
// Changed methods and members bmax/dmax/sigma to more meaningful names
// accel/decel/dawdle.
//
// Revision 1.6  2001/12/20 14:53:07  croessel
// Default ctor, copy-ctor and assignment-operator removed.
// using namespace std added.
//
// Revision 1.5  2001/12/13 12:04:59  croessel
// Default arguments in constructor removed.
//
// Revision 1.4  2001/11/15 17:12:13  croessel
// Outcommented the inclusion of the inline *.iC files. Currently not
// needed.
//
// Revision 1.3  2001/11/14 15:47:34  croessel
// Merged the diffs between the .C and .cpp versions. Numerous changes
// in MSLane, MSVehicle and MSJunction.
//
// Revision 1.2  2001/11/14 10:49:07  croessel
// CR-line-end removed.
//
// Revision 1.1  2001/10/24 07:18:12  traffic
// new extension
//
// Revision 1.5  2001/10/22 12:44:30  traffic
// single person simulation added
//
// Revision 1.4  2001/09/06 15:37:06  croessel
// Set default values in the constructor.
//
// Revision 1.3  2001/07/25 12:18:06  traffic
// CC problems with make_pair repaired
//
// Revision 1.2  2001/07/16 12:55:47  croessel
// Changed id type from unsigned int to string. Added string-pointer
// dictionaries and dictionary methods.
//
// Revision 1.1.1.1  2001/07/11 15:51:13  traffic
// new start
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

#include "MSVehicleType.h"
#include "MSNet.h"
#include <cassert>
#include <utils/bindevice/BinaryInputDevice.h>
#include <utils/common/FileHelpers.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * static member definitions
 * ======================================================================= */
MSVehicleType::DictType MSVehicleType::myDict;
SUMOReal MSVehicleType::myMinDecel  = 0;
SUMOReal MSVehicleType::myMaxLength = 0;


/* =========================================================================
 * method definitions
 * ======================================================================= */
MSVehicleType::~MSVehicleType()
{
}


MSVehicleType::MSVehicleType(const string &id, SUMOReal length,
                             SUMOReal maxSpeed, SUMOReal accel,
                             SUMOReal decel, SUMOReal dawdle,
                             SUMOVehicleClass vclass)
    : myID(id), myLength(length), myMaxSpeed(maxSpeed), myAccel(accel),
    myDecel(decel), myDawdle(dawdle), myTau(1), myVehicleClass(vclass)
{
    assert( myLength > 0 );
    assert( myMaxSpeed > 0 );
    assert( myAccel > 0 );
    assert( myDecel > 0 );
    assert( myDawdle >= 0 && myDawdle <= 1 );
    if(myMinDecel==0 || myDecel<myMinDecel) {
        myMinDecel = myDecel;
    }
    if (myLength>myMaxLength) {
        myMaxLength = myLength;
    }
    myInverseTwoDecel = SUMOReal( 1 ) / ( SUMOReal( 2 ) * myDecel );
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
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


MSVehicleType*
MSVehicleType::dict_Random()
{
    assert(myDict.size()!=0);
    size_t r = (size_t) ((SUMOReal) rand() / RAND_MAX * myDict.size());
    if(r>=myDict.size()) {
        r = myDict.size() - 1;
    }
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++, r-=1) {
        if(r==0) {
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
    for(DictType::iterator it=myDict.begin(); it!=myDict.end(); ++it) {
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
    while(size-->0) {
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


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/


// Local Variables:
// mode:C++
// End:
