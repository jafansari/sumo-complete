/**
 * @file   MSDetectorControl.cpp
 * @author Daniel Krajzewicz
 * @date   Tue Jul 29 10:43:35 2003
 * @version $Id$
 * @brief  Implementation of class MSDetectorControl
 *
 */

/* Copyright (C) 2003 by German Aerospace Center (http://www.dlr.de) */

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
// Revision 1.6  2006/04/18 08:05:44  dkrajzew
// beautifying: output consolidation
//
// Revision 1.5  2006/04/11 10:59:06  dkrajzew
// all structures now return their id via getID()
//
// Revision 1.4  2006/01/23 10:25:29  dkrajzew
// post-release changes
//
// Revision 1.3  2006/01/09 11:55:04  dkrajzew
// lanestates removed
//
// Revision 1.2  2005/10/07 11:37:46  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.1  2005/09/15 11:09:33  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2005/09/09 12:51:22  dksumo
// complete code rework: debug_new and config added
//
// Revision 1.1  2005/08/01 13:16:16  dksumo
// getting rid of singleton dictionaries
//
// Revision 1.4  2005/06/14 11:21:05  dksumo
// documentation added
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

#include <iostream>
#include "MSDetectorControl.h"
#include "MSDetector2File.h"
#include "MSInductLoop.h"
#include <microsim/MSUnit.h>
#include <microsim/MSUpdateEachTimestepContainer.h>
#include "MSDetectorHaltingContainerWrapper.h"
#include "MSDetectorOccupancyCorrection.h"
#include "e2_detectors/MS_E2_ZS_CollectorOverLanes.h"
#include "meandata/MSMeanData_Net.h"
#include "meandata/MSMeanData_Net_Utils.h"
#include <microsim/output/e2_detectors/MSE2Collector.h>
#include <microsim/output/e2_detectors/MS_E2_ZS_CollectorOverLanes.h>
#include <microsim/output/e3_detectors/MSE3Collector.h>
#include <microsim/output/MSInductLoop.h>

#include "MSInductLoop.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * member method definitions
 * ======================================================================= */
MSDetectorControl::MSDetectorControl()
{
    MSUnit::create(1.0, 1.0);
}


MSDetectorControl::~MSDetectorControl()
{
    myDetector2File.close();
    myLoops.clear();
    myE2Detectors.clear();
    myE3Detectors.clear();
    myE2OverLanesDetectors.clear();

    delete MSUpdateEachTimestepContainer<
        DetectorContainer::UpdateHaltings >::getInstance();
    delete MSUpdateEachTimestepContainer<
        Detector::UpdateE2Detectors >::getInstance();
    delete MSUpdateEachTimestepContainer<
        Detector::UpdateOccupancyCorrections >::getInstance();
    delete MSUpdateEachTimestepContainer<
        MSDetectorHaltingMapWrapper>::getInstance();
    delete MSUpdateEachTimestepContainer<
        MSUpdateEachTimestep<MSDetectorHaltingMapWrapper> >::getInstance();
    delete MSUnit::getInstance();
    // delete mean data
    for(MSMeanData_Net_Cont::iterator i6=myMeanData.begin(); i6!=myMeanData.end(); ++i6) {
        delete *i6;
    }
    myMeanData.clear();
}


MSInductLoop *
MSDetectorControl::findInductLoop(const std::string &id)
{
    return myLoops.get(id);
}


void
MSDetectorControl::add(MSInductLoop *il,
                      OutputDevice *device,
                      int splInterval)
{
    // insert object into dictionary
    if ( ! myLoops.add( il->getID(), il ) ) {
        MsgHandler::getErrorInstance()->inform("induct loop '" + il->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
    myDetector2File.addDetectorAndInterval(il, device, splInterval); // !!! test
    // !!!! a command may have been added
}


void
MSDetectorControl::add(MSE2Collector *e2,
                      OutputDevice *device,
                      int splInterval)
{
    // insert object into dictionary
    if ( ! myE2Detectors.add( e2->getID(), e2 ) ) {
        MsgHandler::getErrorInstance()->inform("e2-detector '" + e2->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
    myDetector2File.addDetectorAndInterval(e2, device, splInterval); // !!! test
}


void
MSDetectorControl::add(MS_E2_ZS_CollectorOverLanes *e2ol,
                      OutputDevice *device,
                      int splInterval)
{
    // insert object into dictionary
    if ( ! myE2OverLanesDetectors.add( e2ol->getID(), e2ol ) ) {
        MsgHandler::getErrorInstance()->inform("e2-overlanes-detector '" + e2ol->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
    myDetector2File.addDetectorAndInterval(e2ol, device, splInterval); // !!! test
}


void
MSDetectorControl::add(MSE2Collector *e2)
{
    // insert object into dictionary
    if ( ! myE2Detectors.add( e2->getID(), e2 ) ) {
        MsgHandler::getErrorInstance()->inform("e2-detector '" + e2->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
}


void
MSDetectorControl::add(MS_E2_ZS_CollectorOverLanes *e2ol)
{
    // insert object into dictionary
    if ( ! myE2OverLanesDetectors.add( e2ol->getID(), e2ol ) ) {
        MsgHandler::getErrorInstance()->inform("e2-overlanes-detector '" + e2ol->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
}


void
MSDetectorControl::add(MSE3Collector *e3,
                      OutputDevice *device,
                      int splInterval)
{
    // insert object into dictionary
    if ( ! myE3Detectors.add( e3->getID(), e3 ) ){
        MsgHandler::getErrorInstance()->inform("e3-detector '" + e3->getID() + "' could not be build;"
            + "\n (declared twice?)");
        throw ProcessError();
    }
    myDetector2File.addDetectorAndInterval(e3, device, splInterval); // !!! test
}


void
MSDetectorControl::addMeanData(MSMeanData_Net *newMeanData)
{
    /* !!!!
    myMeanData.push_back(newMeanData);
    // we may add it before the network is loaded
    if(myEdges!=0) {
        myEdges->addToLanes(newMeanData);
    }
    */
}


size_t
MSDetectorControl::getMeanDataSize() const
{
    return myMeanData.size();
}


MSDetector2File &
MSDetectorControl::getDet2File()
{
    return myDetector2File;
}

MSDetectorControl::LoopVect
MSDetectorControl::getLoopVector() const
{
    return myLoops.getTempVector();
}


MSDetectorControl::E2Vect
MSDetectorControl::getE2Vector() const
{
    return myE2Detectors.getTempVector();
}


MSDetectorControl::E3Vect
MSDetectorControl::getE3Vector() const
{
    return myE3Detectors.getTempVector();
}


MSDetectorControl::E2ZSOLVect
MSDetectorControl::getE2OLVector() const
{
    return myE2OverLanesDetectors.getTempVector();
}


void
MSDetectorControl::resetInterval(MSDetectorFileOutput *il,
                                 SUMOTime interval)
{
    myDetector2File.resetInterval(il, interval);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

