//---------------------------------------------------------------------------//
//                        GUINet.cpp -
//  A MSNet extended by some values for usage within the gui
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
// Revision 1.56  2006/12/12 12:11:01  dkrajzew
// removed simple/full geometry options; everything is now drawn using full geometry
//
// Revision 1.55  2006/11/30 07:43:35  dkrajzew
// added the inc-dua option in order to increase dua-computation
//
// Revision 1.54  2006/11/23 11:40:24  dkrajzew
// removed unneeded code
//
// Revision 1.53  2006/11/16 10:50:44  dkrajzew
// warnings removed
//
// Revision 1.52  2006/09/18 10:01:07  dkrajzew
// removed deprecated c2c functions, added new made by Danilot Boyom
//
// Revision 1.51  2006/07/06 06:40:38  dkrajzew
// applied current microsim-APIs
//
// Revision 1.50  2006/04/11 10:56:32  dkrajzew
// microsimID() now returns a const reference
//
// Revision 1.49  2006/02/23 11:27:56  dkrajzew
// tls may have now several programs
//
// Revision 1.48  2006/01/26 08:28:53  dkrajzew
// patched MSEdge in order to work with a generic router
//
// Revision 1.47  2006/01/09 11:53:24  dkrajzew
// new visualization settings implemented
//
// Revision 1.46  2005/11/29 13:24:03  dkrajzew
// added a minimum simulation speed definition before the simulation ends (unfinished)
//
// Revision 1.45  2005/10/10 11:53:36  dkrajzew
// debugging actuated tls-lights
//
// Revision 1.44  2005/10/07 11:37:17  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.43  2005/09/22 13:39:35  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.42  2005/09/15 11:06:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.41  2005/07/12 12:18:47  dkrajzew
// edge-based mean data implemented; previous lane-based is now optional
//
// Revision 1.40  2005/05/04 08:03:51  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added; vehicle position retrieval debugging
//
// Revision 1.39  2005/02/01 10:08:23  dkrajzew
// performance computation added; got rid of MSNet::Time
//
// Revision 1.38  2004/12/16 12:20:09  dkrajzew
// debugging
//
// Revision 1.37  2004/11/25 16:26:47  dkrajzew
// consolidated and debugged some detectors and their usage
//
// Revision 1.36  2004/11/24 08:46:43  dkrajzew
// recent changes applied
//
// Revision 1.35  2004/08/02 11:58:00  dkrajzew
// using OutputDevices instead of ostreams
//
// Revision 1.34  2004/07/02 08:57:29  dkrajzew
// using global object selection; detector handling
//  (handling of additional elements) revisited
//
// Revision 1.33  2004/04/02 11:17:07  dkrajzew
// simulation-wide output files are now handled by MSNet directly
//
// Revision 1.32  2004/03/19 12:57:55  dkrajzew
// porting to FOX
//
// Revision 1.31  2004/02/10 07:07:13  dkrajzew
// debugging of network loading after a network failed to be loaded;
//  memory leaks removal
//
// Revision 1.30  2004/02/05 16:30:59  dkrajzew
// multiplicate deletion of E3-detectors on application quit patched
//
// Revision 1.29  2004/01/26 06:59:38  dkrajzew
// work on detectors: e3-detectors loading and visualisation;
//  variable offsets and lengths for lsa-detectors;
//  coupling of detectors to tl-logics; different detector
//  visualistaion in dependence to his controller
//
// Revision 1.28  2003/12/11 06:24:55  dkrajzew
// implemented MSVehicleControl as the instance responsible for vehicles
//
// Revision 1.27  2003/12/09 11:28:23  dkrajzew
// removed some memory leaks
//
// Revision 1.26  2003/12/04 13:36:52  dkrajzew
// detector name changing applied
//
// Revision 1.25  2003/11/26 09:48:58  dkrajzew
// time display added to the tl-logic visualisation
//
// Revision 1.24  2003/11/20 13:05:32  dkrajzew
// loading and using of predefined vehicle colors added
//
// Revision 1.23  2003/11/18 14:30:40  dkrajzew
// debugged and completed lane merging detectors
//
// Revision 1.22  2003/11/12 14:01:54  dkrajzew
// visualisation of tl-logics added
//
// Revision 1.21  2003/11/11 08:13:23  dkrajzew
// consequent usage of Position2D instead of two SUMOReals
//
// Revision 1.20  2003/10/30 08:59:43  dkrajzew
// first implementation of aggregated views using E2-detectors
//
// Revision 1.19  2003/10/22 15:42:56  dkrajzew
// we have to distinct between two teleporter versions now
//
// Revision 1.18  2003/10/02 14:51:20  dkrajzew
// visualisation of E2-detectors implemented
//
// Revision 1.17  2003/09/22 12:38:51  dkrajzew
// detectors need const Lanes
//
// Revision 1.16  2003/09/05 15:01:24  dkrajzew
// first tries for an implementation of aggregated views
//
// Revision 1.15  2003/08/04 11:35:51  dkrajzew
// only GUIVehicles need a color definition; process of building cars changed
//
// Revision 1.14  2003/07/30 08:54:14  dkrajzew
// the network is capable to display the networks state, now
//
// Revision 1.13  2003/07/22 14:59:27  dkrajzew
// changes due to new detector handling
//
// Revision 1.12  2003/07/16 15:24:55  dkrajzew
// GUIGrid now handles the set of things to draw in another manner than
//  GUIEdgeGrid did; Further things to draw implemented
//
// Revision 1.11  2003/06/05 06:29:50  dkrajzew
// first tries to build under linux: warnings removed; moc-files included
//  Makefiles added
//
// Revision 1.10  2003/05/28 07:52:31  dkrajzew
// new usage of MSEventControl adapted
//
// Revision 1.9  2003/05/21 15:15:41  dkrajzew
// yellow lights implemented (vehicle movements debugged
//
// Revision 1.8  2003/05/20 09:26:57  dkrajzew
// data retrieval for new views added
//
// Revision 1.7  2003/04/16 09:50:06  dkrajzew
// centering of the network debugged; additional parameter of maximum display
//  size added
//
// Revision 1.6  2003/04/14 08:27:17  dkrajzew
// new globject concept implemented
//
// Revision 1.5  2003/03/20 16:19:28  dkrajzew
// windows eol removed; multiple vehicle emission added
//
// Revision 1.4  2003/03/12 16:52:06  dkrajzew
// centering of objects debuggt
//
// Revision 1.3  2003/02/07 10:39:17  dkrajzew
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

#include <utility>
#include <set>
#include <microsim/MSNet.h>
#include <microsim/MSJunction.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicleTransfer.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEmitControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gfx/RGBColor.h>
#include "GUINetWrapper.h"
#include <guisim/guilogging/GLObjectValuePassConnector.h>
#include <guisim/GUIEdge.h>
#include <utils/gui/div/GUIExcp_VehicleIsInvisible.h>
#include <guisim/GUILaneSpeedTrigger.h>
#include <guisim/GUIDetectorWrapper.h>
#include <guisim/GUIInductLoop.h>
#include <guisim/GUI_E2_ZS_Collector.h>
#include <guisim/GUI_E2_ZS_CollectorOverLanes.h>
#include <guisim/GUIE3Collector.h>
#include <guisim/GUITrafficLightLogicWrapper.h>
#include <guisim/GUILaneStateReporter.h>
#include <guisim/GUIJunctionWrapper.h>
#include <guisim/GUIVehicleControl.h>
#include <guisim/GUIRouteHandler.h>
#include <gui/GUIGlobals.h>
#include <microsim/MSUpdateEachTimestepContainer.h>
#include <microsim/MSRouteLoader.h>
#include "GUIVehicle.h"
#include "GUINet.h"
#include "GUIHelpingJunction.h"
#include <utils/gui/globjects/GUIGlObjectGlobals.h>
#include "GUIGridBuilder.h"

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
GUINet::GUINet(SUMOTime startTimestep, SUMOTime stopTimestep,
               MSVehicleControl *vc,
               SUMOReal tooSlowRTF, bool logExecTime)
    : MSNet(startTimestep, stopTimestep, vc, tooSlowRTF, logExecTime),
    _grid(10, 10),
    myWrapper(new GUINetWrapper(gIDStorage, *this)),
    myLastSimDuration(0), /*myLastVisDuration(0),*/ myLastIdleDuration(0),
    myLastVehicleMovementCount(0), myOverallVehicleCount(0), myOverallSimDuration(0)
{
}


GUINet::~GUINet()
{
    gIDStorage.clear();
    // delete allocated wrappers
        // of junctions
    for(std::vector<GUIJunctionWrapper*>::iterator i1=myJunctionWrapper.begin(); i1!=myJunctionWrapper.end(); i1++) {
        delete (*i1);
    }
        // of addition structures
    GUIGlObject_AbstractAdd::clearDictionary();
    // of tl-logics
    {
        for(Logics2WrapperMap::iterator i3=myLogics2Wrapper.begin(); i3!=myLogics2Wrapper.end(); i3++) {
            delete (*i3).second;
        }
    }
    {
        std::map<std::string, GUIDetectorWrapper*>::iterator i;
        for(i=myDetectorDict.begin(); i!=myDetectorDict.end(); ++i) {
            delete (*i).second;
        }
    }
        // of the network itself
    delete myWrapper;
}


const Boundary &
GUINet::getBoundary() const
{
    return _boundary;
}

/*
void
GUINet::preInitGUINet( SUMOTime startTimeStep,
                      MSVehicleControl *vc)
{
    myInstance = new GUINet();
    MSNet::preInit(startTimeStep, vc);
}


void
GUINet::initGUINet(std::string id, MSEdgeControl* ec,
                   MSJunctionControl* jc,
                   MSRouteLoaderControl *rlc,
                   MSTLLogicControl *tlc,
                   MSDetectorControl *dc,
                   MSTriggerControl *tc,
                   bool logExecutionTime,
                   const std::vector<OutputDevice*> &streams,
                   TimeVector stateDumpTimes,
                   std::string stateDumpFiles,
                   TimeVector dumpMeanDataIntervalls,
                   std::string baseNameDumpFiles,
                   TimeVector laneDumpMeanDataIntervalls,
                   std::string baseNameLaneDumpFiles,
                   const std::vector<int> &dumpBegins,
                   const std::vector<int> &dumpEnds)
{
    MSNet::init(id, ec, jc, rlc, tlc, dc, tc, logExecutionTime, streams,
        stateDumpTimes, stateDumpFiles,
        dumpMeanDataIntervalls, baseNameDumpFiles,
        laneDumpMeanDataIntervalls, baseNameLaneDumpFiles,
        dumpBegins, dumpEnds);
    GUINet *net = static_cast<GUINet*>(MSNet::getInstance());
    // initialise edge storage for gui
    GUIEdge::fill(net->myEdgeWrapper);
    // initialise junction storage for gui
    GUIHelpingJunction::fill(net->myJunctionWrapper, gIDStorage);
}

*/
void
GUINet::initDetectors()
{
    // e2-detectors
    {
        MSDetectorControl::E2Vect loopVec2 = myDetectorControl->getE2Vector();
        for(MSDetectorControl::E2Vect::const_iterator i2=loopVec2.begin(); i2!=loopVec2.end(); i2++) {
            const MSLane *lane = (*i2)->getLane();
            const GUIEdge * const edge = static_cast<const GUIEdge * const>(lane->getEdge());
            // build the wrapper
            if( (*i2)->getUsageType()==DU_SUMO_INTERNAL
                ||
                (*i2)->getUsageType()==DU_TL_CONTROL) {
                continue;
            }
            GUIDetectorWrapper *wrapper =
                static_cast<GUI_E2_ZS_Collector*>(*i2)->buildDetectorWrapper(
                    gIDStorage, edge->getLaneGeometry(lane));
            // add to dictionary
            myDetectorDict[wrapper->microsimID()] = wrapper;
        }
    }
    // e2 over lanes -detectors
    {
        MSDetectorControl::E2ZSOLVect loopVec3 = myDetectorControl->getE2OLVector();
        for(MSDetectorControl::E2ZSOLVect::const_iterator i3=loopVec3.begin(); i3!=loopVec3.end(); i3++) {
            // build the wrapper
            GUIDetectorWrapper *wrapper =
                static_cast<GUI_E2_ZS_CollectorOverLanes*>(*i3)->buildDetectorWrapper(
                    gIDStorage);
            // add to dictionary
            myDetectorDict[wrapper->microsimID()] = wrapper;
        }
    }
    // e1-detectors
    {
        MSDetectorControl::LoopVect loopVec = myDetectorControl->getLoopVector();
        for(MSDetectorControl::LoopVect::const_iterator i=loopVec.begin(); i!=loopVec.end(); i++) {
            const MSLane *lane = (*i)->getLane();
            const GUIEdge * const edge = static_cast<const GUIEdge * const>(lane->getEdge());
            // build the wrapper
            GUIDetectorWrapper *wrapper =
                static_cast<GUIInductLoop*>(*i)->buildDetectorWrapper(
                    gIDStorage, edge->getLaneGeometry(lane));
            // add to dictionary
            myDetectorDict[wrapper->microsimID()] = wrapper;
        }
    }
    // e3-detectors
    {
        MSDetectorControl::E3Vect loopVec4 = myDetectorControl->getE3Vector();
        for(MSDetectorControl::E3Vect::const_iterator i4=loopVec4.begin(); i4!=loopVec4.end(); i4++) {
            // build the wrapper
            GUIDetectorWrapper *wrapper =
                static_cast<GUIE3Collector*>(*i4)->buildDetectorWrapper(gIDStorage);
            // add to dictionary
            myDetectorDict[wrapper->microsimID()] = wrapper;
        }
    }
}


void
GUINet::initTLMap()
{
    // get the list of loaded tl-logics
    const vector<MSTrafficLightLogic*> &logics = getTLSControl().getAllLogics();
    // allocate storage for the wrappers
    myTLLogicWrappers.reserve(logics.size());
    // go through the logics
    for(vector<MSTrafficLightLogic*>::const_iterator i=logics.begin(); i!=logics.end(); ++i) {
        // get the logic
        MSTrafficLightLogic *tll = (*i);
        if(tll->getID()=="454") {
            string bla = tll->getSubID();
            int b = 0;
        }
        // get the links
        const MSTrafficLightLogic::LinkVectorVector &links = tll->getLinks();
        if(links.size()==0) {
            continue;
        }
        // build the wrapper
        GUITrafficLightLogicWrapper *tllw =
            new GUITrafficLightLogicWrapper(gIDStorage, *myLogics, *tll);
        // build the association link->wrapper
        MSTrafficLightLogic::LinkVectorVector::const_iterator j;
        for(j=links.begin(); j!=links.end(); j++) {
            MSTrafficLightLogic::LinkVector::const_iterator j2;
            for(j2=(*j).begin(); j2!=(*j).end(); j2++) {
                myLinks2Logic[*j2] = tll->getID();
            }
        }
        myLogics2Wrapper[tll] = tllw;
    }
}


Position2D
GUINet::getJunctionPosition(const std::string &name) const
{
    MSJunction *junction = MSJunction::dictionary(name);
    return Position2D(junction->getPosition());
}


bool
GUINet::hasPosition(GUIVehicle *vehicle) const
{
    const GUIEdge * const edge =
        static_cast<const GUIEdge * const>(vehicle->getEdge());
    if(edge==0) {
        return false;
    }
	if(&(vehicle->getLane())==0) {
        return false;
	}
    return true;
}

bool
GUINet::vehicleExists(const std::string &name) const
{
    return myVehicleControl->getVehicle(name)!=0;
}


Boundary
GUINet::getEdgeBoundary(const std::string &name) const
{
    GUIEdge *edge = static_cast<GUIEdge*>(MSEdge::dictionary(name));
    return edge->getBoundary();
}


GUINetWrapper *
GUINet::getWrapper() const
{
    return myWrapper;
}


unsigned int
GUINet::getLinkTLID(MSLink *link) const
{
    Links2LogicMap::const_iterator i = myLinks2Logic.find(link);
    assert(i!=myLinks2Logic.end());
    if(myLogics2Wrapper.find(myLogics->getActive((*i).second))==myLogics2Wrapper.end()) {
        myLogics->getActive((*i).second);
        return -1;
    }
    return myLogics2Wrapper.find(myLogics->getActive((*i).second))->second->getGlID();
}


void
GUINet::guiSimulationStep()
{
    MSUpdateEachTimestepContainer<MSUpdateEachTimestep<GLObjectValuePassConnector<SUMOReal> > >::getInstance()->updateAll();
    MSUpdateEachTimestepContainer<MSUpdateEachTimestep<GLObjectValuePassConnector<CompletePhaseDef> > >::getInstance()->updateAll();
}


std::vector<size_t>
GUINet::getJunctionIDs() const
{
    std::vector<size_t> ret;
    for(std::vector<GUIJunctionWrapper*>::const_iterator i=myJunctionWrapper.begin(); i!=myJunctionWrapper.end(); i++) {
        ret.push_back((*i)->getGlID());
    }
    return ret;
}


void
GUINet::initGUIStructures()
{
    // initialise detector storage for gui
    initDetectors();
    // initialise the tl-map
    initTLMap();
    // initialise edge storage for gui
    GUIEdge::fill(myEdgeWrapper);
    // initialise junction storage for gui
    GUIHelpingJunction::fill(myJunctionWrapper, gIDStorage);
    // build the grid
	GUIGridBuilder b(*this, _grid);
	b.build();
    // get the boundary
    _boundary = _grid.getBoundary();
}


int
GUINet::getWholeDuration() const
{
    return myLastSimDuration+/*myLastVisDuration+*/myLastIdleDuration;
}


int
GUINet::getSimDuration() const
{
    return myLastSimDuration;
}

/*
int
GUINet::getVisDuration() const
{
    return myLastVisDuration;
}
*/


SUMOReal
GUINet::getRTFactor() const
{
    if(myLastSimDuration==0) {
        return -1;
    }
    return (SUMOReal) 1000. / (SUMOReal) myLastSimDuration;
}


SUMOReal
GUINet::getUPS() const
{
    if(myLastSimDuration==0) {
        return -1;
    }
    return (SUMOReal) myLastVehicleMovementCount / (SUMOReal) myLastSimDuration * (SUMOReal) 1000.;
}


SUMOReal
GUINet::getMeanRTFactor(int duration) const
{
    if(myOverallSimDuration==0) {
        return -1;
    }
    return ((SUMOReal)(duration)*(SUMOReal) 1000./(SUMOReal)myOverallSimDuration);
}


SUMOReal
GUINet::getMeanUPS() const
{
    if(myOverallSimDuration==0) {
        return -1;
    }
    return ((SUMOReal)myVehiclesMoved / (SUMOReal)myOverallSimDuration * (SUMOReal) 1000.);
}


int
GUINet::getIdleDuration() const
{
    return myLastIdleDuration;
}


void
GUINet::setSimDuration(int val)
{
    myLastSimDuration = val;
    myOverallSimDuration += val;
    myLastVehicleMovementCount = getVehicleControl().getRunningVehicleNo();
    myOverallVehicleCount += myLastVehicleMovementCount;
}

/*
void
GUINet::setVisDuration(int val)
{
    myLastVisDuration = val;
}
*/

void
GUINet::setIdleDuration(int val)
{
    myLastIdleDuration = val;
}


MSRouteLoader *
GUINet::buildRouteLoader(const std::string &file, int incDUABase, int incDUAStage)
{
    // return a new build route loader
    //  the handler is
    //  a) not adding the vehicles directly
    //  b) using colors
    return new MSRouteLoader(*this, new GUIRouteHandler(file, *myVehicleControl, false, incDUABase, incDUAStage));
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


