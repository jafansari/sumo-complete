//---------------------------------------------------------------------------//
//                        ROVehicle.cpp -
//  A single vehicle
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.19  2006/11/16 10:50:51  dkrajzew
// warnings removed
//
// Revision 1.18  2006/11/14 06:48:58  dkrajzew
// readapting changes in the router-API
//
// Revision 1.17  2006/02/13 07:26:18  dkrajzew
// code beautifying
//
// Revision 1.16  2006/01/26 08:37:24  dkrajzew
// removed warnings 4786
//
// Revision 1.15  2006/01/24 13:43:53  dkrajzew
// added vehicle classes to the routing modules
//
// Revision 1.14  2006/01/09 12:00:59  dkrajzew
// debugging vehicle color usage
//
// Revision 1.13  2005/10/07 11:42:15  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.12  2005/09/23 06:04:36  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.11  2005/09/15 12:05:11  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.10  2005/05/04 08:55:13  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.9  2004/11/23 10:25:52  dkrajzew
// debugging
//
// Revision 1.2  2004/10/29 05:53:29  dksumo
// fastened up the output of warnings and messages
//
// Revision 1.1  2004/10/22 12:50:25  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.8  2004/07/02 09:39:41  dkrajzew
// debugging while working on INVENT; preparation of classes to be derived for an online-routing
//
// Revision 1.7  2004/01/26 08:01:21  dkrajzew
// loaders and route-def types are now renamed in an senseful way; further changes in order to make both new routers work; documentation added
//
// Revision 1.6  2003/07/16 15:36:50  dkrajzew
// vehicles and routes may now have colors
//
// Revision 1.5  2003/03/20 16:39:17  dkrajzew
// periodical car emission implemented; windows eol removed
//
// Revision 1.4  2003/03/17 14:25:28  dkrajzew
// windows eol removed
//
// Revision 1.3  2003/03/03 15:22:38  dkrajzew
// debugging
//
// Revision 1.2  2003/02/07 10:45:06  dkrajzew
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

#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <string>
#include <iostream>
#include "ROVehicleType.h"
#include "RORouteDef.h"
#include "ROVehicle.h"
#include "RORouteDef_Alternatives.h"
#include "RORoute.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
ROVehicle::ROVehicle(ROVehicleBuilder &,
                     const std::string &id, RORouteDef *route,
                     unsigned int depart, ROVehicleType *type,
                     const RGBColor &color,
                     int period, int repNo)
    : myID(id), myColor(color), myType(type), myRoute(route),
	myDepartTime(depart),
	myRepetitionPeriod(period), myRepetitionNumber(repNo)
{
}

ROVehicle::~ROVehicle()
{
}


RORouteDef * const
ROVehicle::getRoute() const
{
    return myRoute;
}


const ROVehicleType * const
ROVehicle::getType() const
{
    return myType;
}


const std::string&
ROVehicle::getID() const
{
    return myID;
}

SUMOTime
ROVehicle::getDepartureTime() const
{
    return myDepartTime;
}


bool
ROVehicle::periodical() const
{
    return myRepetitionPeriod!=-1;
}


void
ROVehicle::saveXMLVehicle(std::ostream * const os) const
{
    (*os) << "<vehicle id=\"" << myID << "\"";
    (*os) << " type=\"" << myType->getID() << "\"";
    (*os) << " depart=\"" << myDepartTime << "\"";
    if(myColor!=RGBColor(-1,-1,-1)) {
        (*os) << " color=\"" << myColor << "\"";
    }
    if(myRepetitionPeriod!=-1) {
        (*os) << " period=\"" << myRepetitionPeriod << "\"";
        (*os) << " repno=\"" << myRepetitionNumber << "\"";
    }
    (*os) << ">" << endl;
}

void
ROVehicle::saveAllAsXML(std::ostream * const os,
                        std::ostream * const altos,
                        ROVehicleType &defType,
                        const RORouteDef * const route) const
{
    // get the reference vehicle's type
    ROVehicleType &type = getTypeForSaving(defType);
    // check whether it was saved before
    if(!type.isSaved()) {
        // ... save if not
        type.xmlOut(*os);
        if(altos!=0) {
            type.xmlOut(*altos);
        }
        type.markSaved();
    }

    // write the vehicle (new style, with included routes)
    (*os) << "   ";
	saveXMLVehicle(os);
    if(altos!=0) {
        (*altos) << "   ";
		saveXMLVehicle(altos);
    }

    // check whether the route shall be saved
    if(!route->isSaved()) {
        // write the route
        const ROEdgeVector &routee = route->getCurrentEdgeVector();
        (*os) << "      <route";
        if(periodical()) {
            // remark for further usage if referenced by more than one vehicle
            (*os) << " multi_ref=\"x\"";
        }
        const RGBColor &c = route->getColor();
        if(c!=RGBColor(-1,-1,-1)) {
            (*os) << " color=\"" << c << "\"";
        }
        (*os) << ">" << routee << "</route>" << endl;
        //route->xmlOutCurrent(*os, periodical());
        // check whether the alternatives shall be written
        if(altos!=0) {
            (*altos) << "      <routealt last=\"" << myRoute->getLastUsedIndex() << "\"";
            if(c!=RGBColor(-1,-1,-1)) {
                (*altos) << " color=\"" << c << "\"";
            }
            (*altos) << ">" << endl;
            if(myRoute->getAlternativesSize()!=1) {
                // ok, we have here a RORouteDef_Alternatives
                for(size_t i=0; i!=myRoute->getAlternativesSize(); i++) {
                    const RORoute &alt =
                        static_cast<RORouteDef_Alternatives*>(myRoute)->getAlternative(i);//_alternatives[i];
                    (*altos) << "         <route cost=\"" << alt.getCosts();
                    (*altos) << "\" probability=\"" << alt.getProbability();
                    (*altos) << "\">";
                    alt.xmlOutEdges((*altos));
                    (*altos) << "</route>" << endl;
                }
            } else {
                // ok, only one alternative; let's write it plain
                (*altos) << "         <route cost=\"" << routee.recomputeCosts(this, getDepartureTime());
                (*altos) << "\" probability=\"1";
                (*altos) << "\">" << routee << "</route>" << endl;
            }
            (*altos) << "      </routealt>" << endl;
        }
    }

    (*os) << "   </vehicle>" << endl;
    if(altos!=0) {
        (*altos) << "   </vehicle>" << endl;
    }
}


ROVehicleType &
ROVehicle::getTypeForSaving(ROVehicleType &defType) const
{
    if(myType==0) {
        WRITE_WARNING("The vehicle '" + getID() + "' has no valid type; Using default.");
        return defType;
    } else {
        return *myType;
    }
}


ROVehicle *
ROVehicle::copy(ROVehicleBuilder &vb,
                const std::string &id, unsigned int depTime,
                RORouteDef *newRoute)
{
    return new ROVehicle(vb, id, newRoute, depTime, myType, myColor,
        myRepetitionPeriod, myRepetitionNumber);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


