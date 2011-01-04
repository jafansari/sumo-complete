/****************************************************************************/
/// @file    MSMeanData_HBEFA.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 10.05.2004
/// @version $Id$
///
// Emission data collector for edges/lanes
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "MSMeanData_HBEFA.h"
#include <utils/common/HelpersHBEFA.h>
#include <limits>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// MSMeanData_HBEFA::MSLaneMeanDataValues - methods
// ---------------------------------------------------------------------------
MSMeanData_HBEFA::MSLaneMeanDataValues::MSLaneMeanDataValues(MSLane * const lane, const SUMOReal length, const bool doAdd,
        const std::set<std::string>* const vTypes,
        const MSMeanData_HBEFA *parent) throw()
        : MSMeanData::MeanDataValues(lane, length, doAdd, vTypes), myParent(parent), CO2(0), CO(0), HC(0), NOx(0), PMx(0), fuel(0) {}


MSMeanData_HBEFA::MSLaneMeanDataValues::~MSLaneMeanDataValues() throw() {
}


void
MSMeanData_HBEFA::MSLaneMeanDataValues::reset(bool) throw() {
    sampleSeconds = 0.;
    travelledDistance = 0.;
    CO2 = 0;
    CO = 0;
    HC = 0;
    NOx = 0;
    PMx = 0;
    fuel = 0;
}


void
MSMeanData_HBEFA::MSLaneMeanDataValues::addTo(MSMeanData::MeanDataValues &val) const throw() {
    MSLaneMeanDataValues& v = (MSLaneMeanDataValues&) val;
    v.sampleSeconds += sampleSeconds;
    v.travelledDistance += travelledDistance;
    v.CO2 += CO2;
    v.CO += CO;
    v.HC += HC;
    v.NOx += NOx;
    v.PMx += PMx;
    v.fuel += fuel;
}


void
MSMeanData_HBEFA::MSLaneMeanDataValues::notifyMoveInternal(SUMOVehicle& veh, SUMOReal timeOnLane, SUMOReal speed) throw() {
    sampleSeconds += timeOnLane;
    travelledDistance += speed * timeOnLane;
    const double a = veh.getPreDawdleAcceleration();
    CO += (timeOnLane * HelpersHBEFA::computeCO(veh.getVehicleType().getEmissionClass(), (double) speed, a));
    CO2 += (timeOnLane * HelpersHBEFA::computeCO2(veh.getVehicleType().getEmissionClass(), (double) speed, a));
    HC += (timeOnLane * HelpersHBEFA::computeHC(veh.getVehicleType().getEmissionClass(), (double) speed, a));
    NOx += (timeOnLane * HelpersHBEFA::computeNOx(veh.getVehicleType().getEmissionClass(), (double) speed, a));
    PMx += (timeOnLane * HelpersHBEFA::computePMx(veh.getVehicleType().getEmissionClass(), (double) speed, a));
    fuel += (timeOnLane * HelpersHBEFA::computeFuel(veh.getVehicleType().getEmissionClass(), (double) speed, a));
}


void
MSMeanData_HBEFA::MSLaneMeanDataValues::write(OutputDevice &dev, const SUMOTime period,
        const SUMOReal /*numLanes*/, const int /*numVehicles*/) const throw(IOError) {
    dev<<std::resetiosflags(std::ios::floatfield);
    const SUMOReal normFactor = SUMOReal(3600. * 1000. / STEPS2TIME(period) / myLaneLength);
    dev << "\" CO_abs=\""<<SUMOReal(CO*1000.) <<
    "\" CO2_abs=\""<<SUMOReal(CO2*1000.) <<
    "\" HC_abs=\""<<SUMOReal(HC*1000.) <<
    "\" PMx_abs=\""<<SUMOReal(PMx*1000.) <<
    "\" NOx_abs=\""<<SUMOReal(NOx*1000.) <<
    "\" fuel_abs=\""<<SUMOReal(fuel*1000.) <<
    "\"\n            CO_normed=\""<<normFactor * CO <<
    "\" CO2_normed=\""<<normFactor * CO2<<
    "\" HC_normed=\""<<normFactor * HC <<
    "\" PMx_normed=\""<<normFactor * PMx <<
    "\" NOx_normed=\""<<normFactor * NOx <<
    "\" fuel_normed=\""<<normFactor * fuel;
    if (sampleSeconds > myParent->myMinSamples) {
        SUMOReal vehFactor = myParent->myMaxTravelTime / sampleSeconds;
        SUMOReal traveltime = myParent->myMaxTravelTime;
        if (travelledDistance > 0.f) {
            vehFactor = MIN2(vehFactor, myLaneLength / travelledDistance);
            traveltime = MIN2(traveltime, myLaneLength * sampleSeconds / travelledDistance);
        }
        dev<<"\"\n            traveltime=\"" << traveltime<<
        "\" CO_perVeh=\""<<CO*vehFactor<<
        "\" CO2_perVeh=\""<<CO2*vehFactor<<
        "\" HC_perVeh=\""<<HC*vehFactor<<
        "\" PMx_perVeh=\""<<PMx*vehFactor<<
        "\" NOx_perVeh=\""<<NOx*vehFactor<<
        "\" fuel_perVeh=\""<<fuel*vehFactor;
    }
    dev<<"\"/>\n";
    dev<<std::setiosflags(std::ios::fixed); // use decimal format
}



// ---------------------------------------------------------------------------
// MSMeanData_HBEFA - methods
// ---------------------------------------------------------------------------
MSMeanData_HBEFA::MSMeanData_HBEFA(const std::string &id,
                                   const SUMOTime dumpBegin, const SUMOTime dumpEnd,
                                   const bool useLanes, const bool withEmpty, const bool withInternal,
                                   const bool trackVehicles,
                                   const SUMOReal maxTravelTime, const SUMOReal minSamples,
                                   const std::set<std::string> vTypes) throw()
        : MSMeanData(id, dumpBegin, dumpEnd, useLanes, withEmpty, withInternal, trackVehicles, maxTravelTime, minSamples, vTypes) {
}


MSMeanData_HBEFA::~MSMeanData_HBEFA() throw() {}


MSMeanData::MeanDataValues*
MSMeanData_HBEFA::createValues(MSLane * const lane, const SUMOReal length, const bool doAdd) const throw(IOError) {
    return new MSLaneMeanDataValues(lane, length, doAdd, &myVehicleTypes, this);
}


/****************************************************************************/

