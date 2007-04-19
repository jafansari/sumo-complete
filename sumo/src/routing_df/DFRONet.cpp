/****************************************************************************/
/// @file    DFRONet.cpp
/// @author  Daniel Krajzewicz
/// @date    Thu, 16.03.2006
/// @version $Id$
///
// A DFROUTER-network
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
#pragma warning(disable: 4503)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <map>
#include <vector>
#include "DFRONet.h"
#include <routing_df/DFDetector.h>
#include <routing_df/DFRORouteDesc.h>
#include "DFDetectorFlow.h"
#include "RODFEdge.h"
#include <cmath>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
DFRONet::DFRONet()
{
    ro = NULL;
}

DFRONet::DFRONet(RONet * Ro, bool amInHighwayMode)
        : myAmInHighwayMode(amInHighwayMode),
        mySourceNumber(0), mySinkNumber(0), myInBetweenNumber(0), myInvalidNumber(0)
{
    ro = Ro;
}

DFRONet::~DFRONet()
{
    delete ro;
}

void
DFRONet::buildApproachList()
{
    std::vector<ROEdge *> r = ro->getMyEdgeCont()->getTempVector();
    std::vector<ROEdge *>::iterator rit = r.begin();
    for (; rit != r.end(); rit++) {
        size_t i = 0;
        size_t length_size = (*rit)->getNoFollowing();
        for (i=0; i<length_size; i++) {
            ROEdge *help = (*rit)->getFollower(i);
            // add the connection help->*rit to myApproachingEdges
            if (myApproachingEdges.find(help)==myApproachingEdges.end()) {
                myApproachingEdges[help] = std::vector<ROEdge*>();
            }
            myApproachingEdges[help].push_back((*rit));
            // add the connection *rit->help to myApproachingEdges
            if (myApproachedEdges.find((*rit))==myApproachedEdges.end()) {
                myApproachedEdges[(*rit)] = std::vector<ROEdge*>();
            }
            myApproachedEdges[(*rit)].push_back(help);

        }
    }

    //debug
    /*
    std::cout << "approaching" << std::endl;
    std::map<std::string, std::vector<std::string> >::iterator it;
    for ( it = myApproachingEdges.begin(); it != myApproachingEdges.end(); it++ )
    {
    std::cout << it->first << std::endl;
    std::vector<std::string>::iterator ti;
    for ( ti = it->second.begin(); ti != it->second.end(); ti++ )
    {
    std::cout << "\t" << (*ti) << std::endl;
    }
    }
    std::cout << "approached" << std::endl;
    //std::map<std::string, std::vector<std::string> >::iterator it;
    for ( it = myApproachedEdges.begin(); it != myApproachedEdges.end(); it++ )
    {
    std::cout << it->first << std::endl;
    std::vector<std::string>::iterator ti;
    for ( ti = it->second.begin(); ti != it->second.end(); ti++ )
    {
    std::cout << "\t" << (*ti) << std::endl;
    }
    }
    */
}


size_t
DFRONet::numberOfEdgesBefore(ROEdge *edge) const
{
    if (myApproachingEdges.find(edge)==myApproachingEdges.end()) {
        return 0;
    }
    return myApproachingEdges.find(edge)->second.size();
}

const std::vector<ROEdge *> &
DFRONet::getEdgesBefore(ROEdge *edge) const
{
    assert(myApproachingEdges.find(edge)!=myApproachingEdges.end());
    return myApproachingEdges.find(edge)->second;
}


size_t
DFRONet::numberOfEdgesAfter(ROEdge *edge) const
{
    if (myApproachedEdges.find(edge)==myApproachedEdges.end()) {
        return 0;
    }
    return myApproachedEdges.find(edge)->second.size();
}


const std::vector<ROEdge *> &
DFRONet::getEdgesAfter(ROEdge *edge) const
{
    assert(myApproachedEdges.find(edge)!=myApproachedEdges.end());
    return myApproachedEdges.find(edge)->second;
}


void
DFRONet::buildDetectorEdgeDependencies(DFDetectorCon &detcont) const
{
    myDetectorsOnEdges.clear();
    myDetectorEdges.clear();
    const std::vector<DFDetector*> &dets = detcont.getDetectors();
    {
        for (std::vector<DFDetector*>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
            ROEdge *e = getDetectorEdge(**i);

            if (myDetectorsOnEdges.find(e)==myDetectorsOnEdges.end()) {
                myDetectorsOnEdges[e] = std::vector<std::string>();
            }
            myDetectorsOnEdges[e].push_back((*i)->getID());

            myDetectorEdges[(*i)->getID()] = e;
        }
    }
}


void
DFRONet::computeTypes(DFDetectorCon &detcont,
                      bool sourcesStrict) const
{
    MsgHandler::getMessageInstance()->beginProcessMsg("Computing detector types...");
    const std::vector< DFDetector*> &dets = detcont.getDetectors();
    // build needed information. first
    buildDetectorEdgeDependencies(detcont);
    // compute detector types then
    {
        for (std::vector< DFDetector*>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
            if (isSource(**i, detcont, sourcesStrict)) {
                (*i)->setType(SOURCE_DETECTOR);
                mySourceNumber++;
            }
            if (isDestination(**i, detcont)) {
                (*i)->setType(SINK_DETECTOR);
                mySinkNumber++;
            }
            if ((*i)->getType()==TYPE_NOT_DEFINED) {
                (*i)->setType(BETWEEN_DETECTOR);
                myInBetweenNumber++;
            }
        }
    }
    // recheck sources
    {
        for (std::vector< DFDetector*>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
            if ((*i)->getType()==SOURCE_DETECTOR&&isFalseSource(**i, detcont)) {
                (*i)->setType(DISCARDED_DETECTOR);
                myInvalidNumber++;
                mySourceNumber--;
            }
        }
    }
    // print results
    MsgHandler::getMessageInstance()->endProcessMsg("done.");
    MsgHandler::getMessageInstance()->inform("Computed detector types:");
    MsgHandler::getMessageInstance()->inform(" " + toString(mySourceNumber) + " source detectors");
    MsgHandler::getMessageInstance()->inform(" " + toString(mySinkNumber) + " sink detectors");
    MsgHandler::getMessageInstance()->inform(" " + toString(myInBetweenNumber) + " in-between detectors");
    MsgHandler::getMessageInstance()->inform(" " + toString(myInvalidNumber) + " invalid detectors");
}


bool
DFRONet::hasInBetweenDetectorsOnly(ROEdge *edge,
                                   const DFDetectorCon &detectors) const
{
    assert(myDetectorsOnEdges.find(edge)!=myDetectorsOnEdges.end());
    const std::vector<std::string> &detIDs = myDetectorsOnEdges.find(edge)->second;
    std::vector<std::string>::const_iterator i;
    for (i=detIDs.begin(); i!=detIDs.end(); ++i) {
        const DFDetector &det = detectors.getDetector(*i);
        if (det.getType()!=BETWEEN_DETECTOR) {
            return false;
        }
    }
    return true;
}


bool
DFRONet::hasSourceDetector(ROEdge *edge,
                           const DFDetectorCon &detectors) const
{
    assert(myDetectorsOnEdges.find(edge)!=myDetectorsOnEdges.end());
    const std::vector<std::string> &detIDs = myDetectorsOnEdges.find(edge)->second;
    std::vector<std::string>::const_iterator i;
    for (i=detIDs.begin(); i!=detIDs.end(); ++i) {
        const DFDetector &det = detectors.getDetector(*i);
        if (det.getType()==SOURCE_DETECTOR) {
            return true;
        }
    }
    return false;
}



void
DFRONet::computeRoutesFor(ROEdge *edge, DFRORouteDesc *base, int /*no*/,
                          bool allEndFollower, bool keepUnfoundEnds,
                          bool keepShortestOnly,
                          std::vector<ROEdge*> &/*visited*/,
                          const DFDetector &det, DFRORouteCont &into,
                          const DFDetectorCon &detectors,
                          std::vector<ROEdge*> &seen) const
{
    std::vector<DFRORouteDesc*> unfoundEnds;
    std::vector<DFRORouteDesc*> toDiscard;
    priority_queue<DFRORouteDesc*, vector<DFRORouteDesc*>, DFRouteDescByTimeComperator> toSolve;
    std::map<ROEdge*, std::vector<ROEdge*> > dets2Follow;
    dets2Follow[edge] = std::vector<ROEdge*>();
    base->passedNo = 0;
    toSolve.push(base);
    while (!toSolve.empty()) {
        DFRORouteDesc *current = toSolve.top();
        toSolve.pop();
        ROEdge *last = *(current->edges2Pass.end()-1);
        if (hasDetector(last)) {
            if (dets2Follow.find(last)==dets2Follow.end()) {
                dets2Follow[last] = std::vector<ROEdge*>();
            }
            for (std::vector<ROEdge*>::reverse_iterator i=current->edges2Pass.rbegin()+1; i!=current->edges2Pass.rend(); ++i) {
                if (hasDetector(*i)) {
                    dets2Follow[*i].push_back(last);
                    break;
                }
            }
        }

        // do not process an edge twice
        if (find(seen.begin(), seen.end(), last)!=seen.end() && keepShortestOnly) {
            continue;
        }
        seen.push_back(last);
        // end if the edge has no further connections
        if (!hasApproached(last)) {
            // ok, no further connections to follow
            current->factor = 1.;
            into.addRouteDesc(current);
            continue;
        }
        // check for passing detectors:
        //  if the current last edge is not the one the detector is placed on ...
        bool addNextNoFurther = false;
        if (last!=getDetectorEdge(det)) {
            // ... if there is a detector ...
            if (hasDetector(last)) {
                if (!hasInBetweenDetectorsOnly(last, detectors)) {
                    // ... and it's not an in-between-detector
                    // -> let's add this edge and the following, but not any further
                    addNextNoFurther = true;
                    current->lastDetectorEdge = last;
                    current->duration2Last = (SUMOTime) current->duration_2;
                    current->distance2Last = current->distance;
                    current->endDetectorEdge = last;
                    if (hasSourceDetector(last, detectors)) {
///!!!                        //toDiscard.push_back(current);
                    }
                    current->factor = 1.;
                    into.addRouteDesc(current);
                    continue;
                } else {
                    // ... if it's an in-between-detector
                    // -> mark the current route as to be continued
                    current->passedNo = 0;
                    current->duration2Last = (SUMOTime) current->duration_2;
                    current->distance2Last = current->distance;
                    current->lastDetectorEdge = last;
                }
            }
        }
        // check for highway off-ramps
        if (myAmInHighwayMode) {
            // if it's beside the highway...
            if (last->getSpeed()<19.4&&last!=getDetectorEdge(det)) {
                // ... and has more than one following edge
                if (myApproachedEdges.find(last)->second.size()>1) {
                    // -> let's add this edge and the following, but not any further
                    addNextNoFurther = true;
                }

            }
        }
        // check for missing end connections
        if (!addNextNoFurther) {
            // ... if this one would be processed, but already too many edge
            //  without a detector occured
            if (current->passedNo>15) { // !!!
                // mark not to process any further
                MsgHandler::getWarningInstance()->inform("Could not close route for '" + det.getID() + "'");
                unfoundEnds.push_back(current);
                current->factor = 1.;
                into.addRouteDesc(current);
                continue; // !!!
            }
        }
        // ... else: loop over the next edges
        const std::vector<ROEdge*> &appr  = myApproachedEdges.find(last)->second;
        bool hadOne = false;
        for (size_t i=0; i<appr.size(); i++) {
            if (find(current->edges2Pass.begin(), current->edges2Pass.end(), appr[i])!=current->edges2Pass.end()) {
                // do not append an edge twice (do not build loops)
                continue;
            }
            DFRORouteDesc *t = new DFRORouteDesc(*current);
            t->duration_2 += (appr[i]->getLength()/appr[i]->getSpeed());//!!!
            t->distance += appr[i]->getLength();
            t->edges2Pass.push_back(appr[i]);
            if (!addNextNoFurther) {
                t->passedNo = t->passedNo + 1;
                toSolve.push(t);
            } else {
                if (!hadOne||allEndFollower) {
//a                    if(allEndFollower) {
                    t->factor = (SUMOReal) 1. / (SUMOReal) appr.size();
                    /*a                    } else {
                                            t->factor = (SUMOReal) 1.;
                                        }*/
                    into.addRouteDesc(t);
                    hadOne = true;
                }
            }
        }
    }
    into.setDets2Follow(dets2Follow);
    //
    if (!keepUnfoundEnds) {
        std::vector<DFRORouteDesc*>::iterator i;
        std::vector<const ROEdge*> lastDetEdges;
        for (i=unfoundEnds.begin(); i!=unfoundEnds.end(); ++i) {
            if (find(lastDetEdges.begin(), lastDetEdges.end(), (*i)->lastDetectorEdge)==lastDetEdges.end()) {
                lastDetEdges.push_back((*i)->lastDetectorEdge);
            } else {
                bool ok = into.removeRouteDesc(*i);
                assert(ok);
            }
        }

//        forunfoundEnds;
    } else {
        // !!! patch the factors
    }
    if (true) {
        std::vector<DFRORouteDesc*>::iterator i;
        for (i=toDiscard.begin(); i!=toDiscard.end(); ++i) {
            /*bool ok = */into.removeRouteDesc(*i);
//!!!            assert(ok);
        }
    }

    /*
    // ok, we have built the routes;
    //  now, we have to compute the relationships between them
    const std::vector<DFRORouteDesc*> &routes = into.get();
    if(routes.empty()) {
        return;
    }
        // build a tree of routes
    tree<ROEdge*> routesTree;
    map<ROEdge*, std::vector<DFRORouteDesc*> > edges2Routes;
    {
        routesTree.insert(routes[0]->edges2Pass[0]);
        edges2Routes[routes[0]->edges2Pass[0]] = std::vector<DFRORouteDesc*>();
        for(std::vector<DFRORouteDesc*>::const_iterator i=routes.begin(); i!=routes.end(); ++i) {
            DFRORouteDesc *c = *i;
            tree<ROEdge*>::iterator k = routesTree.begin();
            edges2Routes[routes[0]->edges2Pass[0]].push_back(c);
            for(std::vector<ROEdge*>::const_iterator j=c->edges2Pass.begin()+1; j!=c->edges2Pass.end(); ++j) {
                tree<ROEdge*>::iterator l = (*k).find(*j);
                if(l==(*k).end()) {
                    k = (*k).insert(*j);
                } else {
                    k = l;
                }
                if(edges2Routes.find(*j)==edges2Routes.end()) {
                    edges2Routes[*j] = std::vector<DFRORouteDesc*>();
                }
                edges2Routes[*j].push_back(*i);
            }
        }
    }
        // travel the tree and compute the distributions of routes
            // mark first detectors on edges?
    {
        std::vector<ROEdge*> solved;
        std::vector<tree<ROEdge*>::iterator> toSolve;
        std::map<ROEdge*, std::vector<ROEdge*> > split2Dets;
        toSolve.push_back(routesTree.begin());
        while(!toSolve.empty()) {
            tree<ROEdge*>::iterator i = toSolve.back();
            toSolve.pop_back();
            if((*i).size()==1) {
                toSolve.push_back((*i).begin());
                continue;
            }
            if((*i).size()==0) {
                // ok, we have found the end, let's move backwards
                ROEdge *lastWithDetector = 0;
                tree<ROEdge*> *parent = (*i).parent();
                tree<ROEdge*> *current = &(*i);
                while(parent!=0) {
                    ROEdge *currentEdge = *(current->get());
                    if(hasDetector(currentEdge)) {
                        lastWithDetector = currentEdge;
                    }
                    current = parent;
                    parent = current->parent();
                    if(parent->size()>1) {
                        if(split2Dets.find(*(parent->get()))==split2Dets.end()) {
                            split2Dets[*(parent->get())] = std::vector<ROEdge*>();
                        }
                        split2Dets[*(parent->get())].push_back(currentEdge);
                    }
                }
            }
            if((*i).size()>1) {
                tree<ROEdge*>::iterator j;
                for(j=(*i).begin(); j!=(*i).end(); ++j) {
                    toSolve.push_back(j);
                }
            }
        }
        /
        std::vector<tree<ROEdge*>::iterator> toSolve;
        // a map of (unsolved) edges to the list of solving edges
        std::map<ROEdge*, std::vector<std::pair<ROEdge*, bool> > > split2Dets;
        std::map<ROEdge*, ROEdge*> dets2Split;
        toSolve.push_back(routesTree.begin());
        while(!toSolve.empty()) {
            tree<ROEdge*>::iterator i = toSolve.back();
            ROEdge *c = *(i->get());
            if(dets2Split.find(c)!=dets2Split.end()&&hasDetector(c)) {
                std::vector<std::pair<ROEdge*, bool> >::iterator l;
                for(l=split2Dets[dets2Split[c]].begin(); l!=split2Dets[dets2Split[c]].end(); ++l) {
                    if((*l).first==c) {
                        (*l).second = true;
                        break;
                    }
                }
            }
            toSolve.pop_back();
            if((*i).size()==1) {
                toSolve.push_back((*i).begin());
                continue;
            }
            if((*i).size()==0) {
                // !!!
            }
            if((*i).size()>1) {
                // ok, a split; check what to do
                split2Dets[*(i->get())] = std::vector<std::pair<ROEdge*, bool> >();
                tree<ROEdge*>::iterator j;
                for(j=(*i).begin(); j!=(*i).end(); ++j) {
                    toSolve.push_back(j);
                    split2Dets[*(i->get())].push_back(make_pair(*(j->get()), false) );
                    dets2Split[*(j->get())] = *(i->get());
                }
            }
        }
        /
    }
    */
}


void
DFRONet::buildRoutes(DFDetectorCon &detcont, bool allEndFollower,
                     bool keepUnfoundEnds, bool includeInBetween,
                     bool keepShortestOnly) const
{
    std::vector<std::vector<ROEdge*> > illegals;
    std::vector<ROEdge*> i1;
    i1.push_back(ro->getEdge("-51140604"));
    i1.push_back(ro->getEdge("-51140594"));
    i1.push_back(ro->getEdge("51140072"));
    i1.push_back(ro->getEdge("51140612"));
    illegals.push_back(i1);
    std::vector<ROEdge*> i2;
    i2.push_back(ro->getEdge("-51047761"));
    i2.push_back(ro->getEdge("-51047760"));
    i2.push_back(ro->getEdge("51047759"));
    i2.push_back(ro->getEdge("51047758"));
    illegals.push_back(i2);
    std::vector<ROEdge*> i3;
    i3.push_back(ro->getEdge("-51049672"));
    i3.push_back(ro->getEdge("-51049675"));
    i3.push_back(ro->getEdge("51049662"));
    i3.push_back(ro->getEdge("51049676"));
    illegals.push_back(i3);
    std::vector<ROEdge*> i4;
    i4.push_back(ro->getEdge("-51069817"));
    i4.push_back(ro->getEdge("-51069812"));
    i4.push_back(ro->getEdge("51069813"));
    i4.push_back(ro->getEdge("51069815"));
    illegals.push_back(i4);
    std::vector<ROEdge*> i5;
    i5.push_back(ro->getEdge("50872831"));
    i5.push_back(ro->getEdge("-50872833"));
    i5.push_back(ro->getEdge("-50872829"));
    i5.push_back(ro->getEdge("572267133"));
    illegals.push_back(i5);
    std::vector<ROEdge*> i6;
    i6.push_back(ro->getEdge("-51066847"));
    i6.push_back(ro->getEdge("-51066836"));
    i6.push_back(ro->getEdge("51066830"));
    i6.push_back(ro->getEdge("51066846"));
    illegals.push_back(i6);
    // build needed information first
    buildDetectorEdgeDependencies(detcont);
    // then build the routes
    std::map<ROEdge*, DFRORouteCont * > doneEdges;
    const std::vector< DFDetector*> &dets = detcont.getDetectors();
    for (std::vector< DFDetector*>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
        if ((*i)->getType()!=SOURCE_DETECTOR) {
            // do not build routes for other than sources
            continue;
        }
        /*
                // !!! maybe (optional) routes for between-detectors also should not be build
                if((*i)->getType()==BETWEEN_DETECTOR) {
                    // do not build routes for sinks
                    continue;
                }
        		*/
        ROEdge *e = getDetectorEdge(**i);
        if (doneEdges.find(e)!=doneEdges.end()) {
            // use previously build routes
            (*i)->addRoutes(new DFRORouteCont(*doneEdges[e]));
            continue;
        }
        std::vector<ROEdge*> seen;
        DFRORouteCont *routes = new DFRORouteCont(*this);
        doneEdges[e] = routes;
        DFRORouteDesc *rd = new DFRORouteDesc();
        rd->edges2Pass.push_back(e);
        rd->duration_2 = (e->getLength()/e->getSpeed());//!!!;
        rd->endDetectorEdge = 0;
        rd->lastDetectorEdge = 0;
        rd->distance = e->getLength();
        rd->distance2Last = 0;
        rd->duration2Last = 0;

        rd->overallProb = 0;

        std::vector<ROEdge*> visited;
        visited.push_back(e);
        computeRoutesFor(e, rd, 0, allEndFollower, keepUnfoundEnds, keepShortestOnly,
                         visited, **i, *routes, detcont, seen);
        routes->removeIllegal(illegals);
        (*i)->addRoutes(routes);
        //cout << (*i)->getID() << " : " << routes->get().size() << endl;

        // add routes to in-between detectors if wished
        if (includeInBetween) {
            // go through the routes
            const std::vector<DFRORouteDesc*> &r = routes->get();
            for (std::vector<DFRORouteDesc*>::const_iterator j=r.begin(); j!=r.end(); ++j) {
                DFRORouteDesc *mrd = *j;
                SUMOReal duration = mrd->duration_2;
                SUMOReal distance = mrd->distance;
                // go through each route's edges
                std::vector<ROEdge*>::const_iterator routeend = mrd->edges2Pass.end();
                for (std::vector<ROEdge*>::const_iterator k=mrd->edges2Pass.begin(); k!=routeend; ++k) {
                    // check whether any detectors lies on the current edge
                    if (myDetectorsOnEdges.find(*k)==myDetectorsOnEdges.end()) {
                        duration -= (*k)->getLength()/(*k)->getSpeed();
                        distance -= (*k)->getLength();
                        continue;
                    }
                    // get the detectors
                    const std::vector<std::string> &dets = myDetectorsOnEdges.find(*k)->second;
                    // go through the detectors
                    for (std::vector<std::string>::const_iterator l=dets.begin(); l!=dets.end(); ++l) {
                        const DFDetector &m = detcont.getDetector(*l);
                        if (m.getType()==BETWEEN_DETECTOR) {
                            DFRORouteDesc *nrd = new DFRORouteDesc();
                            copy(k, routeend, back_inserter(nrd->edges2Pass));
                            nrd->duration_2 = duration;//!!!;
                            nrd->endDetectorEdge = mrd->endDetectorEdge;
                            nrd->lastDetectorEdge = mrd->lastDetectorEdge;
                            nrd->distance = distance;
                            nrd->distance2Last = mrd->distance2Last;
                            nrd->duration2Last = mrd->duration2Last;
                            nrd->overallProb = mrd->overallProb;
                            nrd->factor = mrd->factor;
                            ((DFDetector&) m).addRoute(*this, nrd);
                        }
                    }
                    duration -= (*k)->getLength()/(*k)->getSpeed();
                    distance -= (*k)->getLength();
                }
            }
        }

    }
}


void
DFRONet::revalidateFlows(const DFDetector *detector,
                         DFDetectorFlows &flows,
                         SUMOTime startTime, SUMOTime endTime,
                         SUMOTime stepOffset)
{
    {
        if (flows.knows(detector->getID())) {
            const std::vector<FlowDef> &detFlows = flows.getFlowDefs(detector->getID());
            for (std::vector<FlowDef>::const_iterator j=detFlows.begin(); j!=detFlows.end(); ++j) {
                if ((*j).qPKW>0||(*j).qLKW>0) {
                    return;
                }
            }
        }
    }
    // ok, there is no information for the whole time;
    //  lets find preceding detectors and rebuild the flows if possible
    WRITE_WARNING("Detector '" + detector->getID() + "' has no flows."
                  + "\n Trying to rebuild.");
    // go back and collect flows
    std::vector<ROEdge*> previous;
    {
        std::vector<IterationEdge> missing;
        IterationEdge ie;
        ie.depth = 0;
        ie.edge = getDetectorEdge(*detector);
        missing.push_back(ie);
        bool maxDepthReached = false;
        while (!missing.empty()&&!maxDepthReached) {
            IterationEdge last = missing.back();
            missing.pop_back();
            std::vector<ROEdge*> approaching = myApproachingEdges[last.edge];
            for (std::vector<ROEdge*>::const_iterator j=approaching.begin(); j!=approaching.end(); ++j) {
                if (hasDetector(*j)) {
                    previous.push_back(*j);
                } else {
                    ie.depth = last.depth + 1;
                    ie.edge = *j;
                    missing.push_back(ie);
                    if (ie.depth>5) {
                        maxDepthReached = true;
                    }
                }
            }
        }
        if (maxDepthReached) {
            WRITE_WARNING(" Could not build list of previous flows.");
        }
    }
    // Edges with previous detectors are now in "previous";
    //  compute following
    std::vector<ROEdge*> latter;
    {
        std::vector<IterationEdge> missing;
        for (std::vector<ROEdge*>::const_iterator k=previous.begin(); k!=previous.end(); ++k) {
            IterationEdge ie;
            ie.depth = 0;
            ie.edge = *k;
            missing.push_back(ie);
        }
        bool maxDepthReached = false;
        while (!missing.empty()&&!maxDepthReached) {
            IterationEdge last = missing.back();
            missing.pop_back();
            std::vector<ROEdge*> approached = myApproachedEdges[last.edge];
            for (std::vector<ROEdge*>::const_iterator j=approached.begin(); j!=approached.end(); ++j) {
                if (*j==getDetectorEdge(*detector)) {
                    continue;
                }
                if (hasDetector(*j)) {
                    latter.push_back(*j);
                } else {
                    IterationEdge ie;
                    ie.depth = last.depth + 1;
                    ie.edge = *j;
                    missing.push_back(ie);
                    if (ie.depth>5) {
                        maxDepthReached = true;
                    }
                }
            }
        }
        if (maxDepthReached) {
            WRITE_WARNING(" Could not build list of latter flows.");
            return;
        }
    }
    // Edges with latter detectors are now in "latter";

    // lets not validate them by now - surely this should be done
    // for each time step: collect incoming flows; collect outgoing;
    std::vector<FlowDef> mflows;
    for (SUMOTime t=startTime; t<endTime; t+=stepOffset) {
        FlowDef inFlow;
        inFlow.qLKW = 0;
        inFlow.qPKW = 0;
        inFlow.vLKW = 0;
        inFlow.vPKW = 0;
        // collect incoming
        {
            // !! time difference is missing
            for (std::vector<ROEdge*>::iterator i=previous.begin(); i!=previous.end(); ++i) {
                const std::vector<FlowDef> &flows = static_cast<const RODFEdge*>(*i)->getFlows();
                if (flows.size()!=0) {
                    const FlowDef &srcFD = flows[(int)(t/stepOffset) - startTime];
                    inFlow.qLKW += srcFD.qLKW;
                    inFlow.qPKW += srcFD.qPKW;
                    inFlow.vLKW += srcFD.vLKW;
                    inFlow.vPKW += srcFD.vPKW;
                }
            }
        }
        inFlow.vLKW /= (SUMOReal) previous.size();
        inFlow.vPKW /= (SUMOReal) previous.size();
        // collect outgoing
        FlowDef outFlow;
        outFlow.qLKW = 0;
        outFlow.qPKW = 0;
        outFlow.vLKW = 0;
        outFlow.vPKW = 0;
        {
            // !! time difference is missing
            for (std::vector<ROEdge*>::iterator i=latter.begin(); i!=latter.end(); ++i) {
                const std::vector<FlowDef> &flows = static_cast<const RODFEdge*>(*i)->getFlows();
                if (flows.size()!=0) {
                    const FlowDef &srcFD = flows[(int)(t/stepOffset) - startTime];
                    outFlow.qLKW += srcFD.qLKW;
                    outFlow.qPKW += srcFD.qPKW;
                    outFlow.vLKW += srcFD.vLKW;
                    outFlow.vPKW += srcFD.vPKW;
                }
            }
        }
        outFlow.vLKW /= (SUMOReal) latter.size();
        outFlow.vPKW /= (SUMOReal) latter.size();
        //
        FlowDef mFlow;
        mFlow.qLKW = inFlow.qLKW - outFlow.qLKW;
        mFlow.qPKW = inFlow.qPKW - outFlow.qPKW;
        mFlow.vLKW = (inFlow.vLKW + outFlow.vLKW) / (SUMOReal) 2.;
        mFlow.vPKW = (inFlow.vPKW + outFlow.vPKW) / (SUMOReal) 2.;
        mflows.push_back(mFlow);
    }
    static_cast<RODFEdge*>(getDetectorEdge(*detector))->setFlows(mflows);
    flows.setFlows(detector->getID(), mflows);
}


void
DFRONet::revalidateFlows(const DFDetectorCon &detectors,
                         DFDetectorFlows &flows,
                         SUMOTime startTime, SUMOTime endTime,
                         SUMOTime stepOffset)
{
    const std::vector<DFDetector*> &dets = detectors.getDetectors();
    for (std::vector<DFDetector*>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
        // check whether there is at least one entry with a flow larger than zero
        revalidateFlows(*i, flows, startTime, endTime, stepOffset);
    }
}



void
DFRONet::removeEmptyDetectors(DFDetectorCon &detectors,
                              DFDetectorFlows &flows,
                              SUMOTime /*startTime*/, SUMOTime /*endTime*/,
                              SUMOTime /*stepOffset*/)
{
    const std::vector<DFDetector*> &dets = detectors.getDetectors();
    for (std::vector<DFDetector*>::const_iterator i=dets.begin(); i!=dets.end();) {
        bool remove = true;
        // check whether there is at least one entry with a flow larger than zero
        if (flows.knows((*i)->getID())) {
            const std::vector<FlowDef> &detFlows = flows.getFlowDefs((*i)->getID());
            for (std::vector<FlowDef>::const_iterator j=detFlows.begin(); remove&&j!=detFlows.end(); ++j) {
                    if ((*j).qPKW>0||(*j).qLKW>0) {
                        remove = false;
                    }
                }
        }
        if (remove) {
            cout << "Removed '" << (*i)->getID() << "'." << endl;
            flows.removeFlow((*i)->getID());
            detectors.removeDetector((*i)->getID());
            i = dets.begin();
        }
        else {
            i++;
        }
    }
}



ROEdge *
DFRONet::getDetectorEdge(const DFDetector &det) const
{
    string edgeName = det.getLaneID();
    edgeName = edgeName.substr(0, edgeName.rfind('_'));
    ROEdge *ret = ro->getEdge(edgeName);
    if(ret==0) {
        MsgHandler::getErrorInstance()->inform("Edge '" + edgeName + "' used by detector '" + det.getID() + "' is not known.");
        throw ProcessError();
    }
    return ret;
}


bool
DFRONet::hasApproaching(ROEdge *edge) const
{
    return
        myApproachingEdges.find(edge)!=myApproachingEdges.end()
        &&
        myApproachingEdges.find(edge)->second.size()!=0;
}


bool
DFRONet::hasApproached(ROEdge *edge) const
{
    return
        myApproachedEdges.find(edge)!=myApproachedEdges.end()
        &&
        myApproachedEdges.find(edge)->second.size()!=0;
}


bool
DFRONet::hasDetector(ROEdge *edge) const
{
    return
        myDetectorsOnEdges.find(edge)!=myDetectorsOnEdges.end()
        &&
        myDetectorsOnEdges.find(edge)->second.size()!=0;
}


SUMOReal
DFRONet::getAbsPos(const DFDetector &det) const
{
    if (det.getPos()>=0) {
        return det.getPos();
    }
    return getDetectorEdge(det)->getLength() + det.getPos();
}

bool
DFRONet::isSource(const DFDetector &det, const DFDetectorCon &detectors,
                  bool strict) const
{
    std::vector<ROEdge*> seen;
    return
        isSource(det, getDetectorEdge(det), seen, detectors, strict);
}

bool
DFRONet::isFalseSource(const DFDetector &det, const DFDetectorCon &detectors) const
{
    std::vector<ROEdge*> seen;
    return
        isFalseSource(det, getDetectorEdge(det), seen, detectors);
}

bool
DFRONet::isDestination(const DFDetector &det, const DFDetectorCon &detectors) const
{
    std::vector<ROEdge*> seen;
    return isDestination(det, getDetectorEdge(det), seen, detectors);
}


bool
DFRONet::isSource(const DFDetector &det, ROEdge *edge,
                  std::vector<ROEdge*> &seen,
                  const DFDetectorCon &detectors,
                  bool strict) const
{
    if(seen.size()==1000) { // !!!
        MsgHandler::getWarningInstance()->inform("Quitting checking for being a source for detector '" + det.getID() + "' due to seen edge limit.");
        return false;
    }
    if (edge==getDetectorEdge(det)) {
        // maybe there is another detector at the same edge
        //  get the list of this/these detector(s)
        const std::vector<std::string> &detsOnEdge = myDetectorsOnEdges.find(edge)->second;
        for (std::vector<std::string>::const_iterator i=detsOnEdge.begin(); i!=detsOnEdge.end(); ++i) {
            if ((*i)==det.getID()) {
                continue;
            }
            const DFDetector &sec = detectors.getDetector(*i);
            if (getAbsPos(sec)<getAbsPos(det)) {
                // ok, there is another detector on the same edge and it is
                //  before this one -> no source
                return false;
            }
        }
    }
    // it's a source if no edges are approaching the edge
    if (!hasApproaching(edge)) {
        if (edge!=getDetectorEdge(det)) {
            if (hasDetector(edge)) {
                return false;
            }
        }
        return true;
    }
    if (edge!=getDetectorEdge(det)) {
        // ok, we are at one of the edges in front
        if (myAmInHighwayMode) {
            if (edge->getSpeed()>=19.4) {
                if (hasDetector(edge)) {
                    // we are still on the highway and there is another detector
                    return false;
                }
                // the next is a hack for the A100 scenario...
                //  We have to look into further edges herein edges
                const std::vector<ROEdge*> &appr = myApproachingEdges.find(edge)->second;
                size_t noOk = 0;
                size_t noFalse = 0;
                size_t noSkipped = 0;
                for (size_t i=0; i<appr.size(); i++) {
                    if (!hasDetector(appr[i])) {
                        noOk++;
                    } else {
                        noFalse++;
                    }
                }
                if ((noFalse+noSkipped)==appr.size()) {
                    return false;
                }
            }
        }
    }

    if (myAmInHighwayMode) {
        if (edge->getSpeed()<19.4&&edge!=getDetectorEdge(det)) {
            // we have left the highway already
            //  -> the detector will be a highway source
            if (!hasDetector(edge)) {
                return true;
            }
        }
    }
    if (myDetectorsOnEdges.find(edge)!=myDetectorsOnEdges.end()
            &&
            myDetectorEdges.find(det.getID())->second!=edge) {
        return false;
    }

    // let's check the edges in front
    const std::vector<ROEdge*> &appr = myApproachingEdges.find(edge)->second;
    size_t noOk = 0;
    size_t noFalse = 0;
    size_t noSkipped = 0;
    seen.push_back(edge);
    for (size_t i=0; i<appr.size(); i++) {
        bool had = std::find(seen.begin(), seen.end(), appr[i])!=seen.end();
        if (!had) {
            if (isSource(det, appr[i], seen, detectors, strict)) {
                noOk++;
            } else {
                noFalse++;
            }
        } else {
            noSkipped++;
        }
    }
    if (!strict) {
        return (noFalse+noSkipped)!=appr.size();
    } else {
        return (noOk+noSkipped)==appr.size();
    }
}


bool
DFRONet::isDestination(const DFDetector &det, ROEdge *edge, std::vector<ROEdge*> &seen,
                       const DFDetectorCon &detectors) const
{
    if(seen.size()==1000) { // !!!
        MsgHandler::getWarningInstance()->inform("Quitting checking for being a destination for detector '" + det.getID() + "' due to seen edge limit.");
        return false;
    }
    if (edge==getDetectorEdge(det)) {
        // maybe there is another detector at the same edge
        //  get the list of this/these detector(s)
        const std::vector<std::string> &detsOnEdge = myDetectorsOnEdges.find(edge)->second;
        for (std::vector<std::string>::const_iterator i=detsOnEdge.begin(); i!=detsOnEdge.end(); ++i) {
            if ((*i)==det.getID()) {
                continue;
            }
            const DFDetector &sec = detectors.getDetector(*i);
            if (getAbsPos(sec)>getAbsPos(det)) {
                // ok, there is another detector on the same edge and it is
                //  after this one -> no destination
                return false;
            }
        }
    }
    if (!hasApproached(edge)) {
        if (edge!=getDetectorEdge(det)) {
            if (hasDetector(edge)) {
                return false;
            }
        }
        return true;
    }
    if (edge!=getDetectorEdge(det)) {
        // ok, we are at one of the edges coming behind
        if (myAmInHighwayMode) {
            if (edge->getSpeed()>=19.4) {
                if (hasDetector(edge)) {
                    // we are still on the highway and there is another detector
                    return false;
                }
            }
        }
    }

    if (myAmInHighwayMode) {
        if (edge->getSpeed()<19.4&&edge!=getDetectorEdge(det)) {
            if (hasDetector(edge)) {
                return true;
            }
            if (myApproachedEdges.find(edge)->second.size()>1) {
                return true;
            }

        }
    }

    if (myDetectorsOnEdges.find(edge)!=myDetectorsOnEdges.end()
            &&
            myDetectorEdges.find(det.getID())->second!=edge) {
        return false;
    }
    const std::vector<ROEdge*> &appr  = myApproachedEdges.find(edge)->second;
    bool isall = true;
    size_t no = 0;
    seen.push_back(edge);
    for (size_t i=0; i<appr.size()&&isall; i++) {
        //printf("checking %s->\n", appr[i].c_str());
        bool had = std::find(seen.begin(), seen.end(), appr[i])!=seen.end();
        if (!had) {
            if (!isDestination(det, appr[i], seen, detectors)) {
                no++;
                isall = false;
            }
        }
    }
    return isall;//no==appr.size();//isall;
}

bool
DFRONet::isFalseSource(const DFDetector &det, ROEdge *edge, std::vector<ROEdge*> &seen,
                       const DFDetectorCon &detectors) const
{
    if(seen.size()==1000) { // !!!
        MsgHandler::getWarningInstance()->inform("Quitting checking for being a false source for detector '" + det.getID() + "' due to seen edge limit.");
        return false;
    }
    seen.push_back(edge);
    if (edge!=getDetectorEdge(det)) {
        // ok, we are at one of the edges coming behind
        if (hasDetector(edge)) {
            const std::vector<std::string> &dets = myDetectorsOnEdges.find(edge)->second;
            for (std::vector<std::string>::const_iterator i=dets.begin(); i!=dets.end(); ++i) {
                if (detectors.getDetector(*i).getType()==SINK_DETECTOR) {
                    return false;
                }
                if (detectors.getDetector(*i).getType()==BETWEEN_DETECTOR) {
                    return false;
                }
                if (detectors.getDetector(*i).getType()==SOURCE_DETECTOR) {
                    return true;
                }
            }
        } else {
            if (myAmInHighwayMode&&edge->getSpeed()<19.) {
                return false;
            }
        }
    }

    if (myApproachedEdges.find(edge)==myApproachedEdges.end()) {
        return false;
    }

    const std::vector<ROEdge*> &appr  = myApproachedEdges.find(edge)->second;
    bool isall = false;
    for (size_t i=0; i<appr.size()&&!isall; i++) {
        //printf("checking %s->\n", appr[i].c_str());
        bool had = std::find(seen.begin(), seen.end(), appr[i])!=seen.end();
        if (!had) {
            if (isFalseSource(det, appr[i], seen, detectors)) {
                isall = true;
            }
        }
    }
    return isall;
}


void
DFRONet::buildEdgeFlowMap(const DFDetectorFlows &flows,
                          const DFDetectorCon &detectors,
                          SUMOTime startTime, SUMOTime endTime,
                          SUMOTime stepOffset)
{
    std::map<ROEdge*, std::vector<std::string> >::iterator i;
    for (i=myDetectorsOnEdges.begin(); i!=myDetectorsOnEdges.end(); ++i) {
        ROEdge *into = (*i).first;
        const std::vector<std::string> &dets = (*i).second;
        std::map<SUMOReal, std::vector<std::string> > cliques;
        size_t maxCliqueSize = 0;
        for (std::vector<std::string>::const_iterator j=dets.begin(); j!=dets.end(); ++j) {
            if (!flows.knows(*j)) {
                continue;
            }
            const DFDetector &det = detectors.getDetector(*j);
            bool found = false;
            for (std::map<SUMOReal, std::vector<std::string> >::iterator k=cliques.begin(); !found&&k!=cliques.end(); ++k) {
                if (fabs((*k).first-det.getPos())<1) {
                    (*k).second.push_back(*j);
                    maxCliqueSize = MAX2(maxCliqueSize, (*k).second.size());
                    found = true;
                }
            }
            if (!found) {
                cliques[det.getPos()] = std::vector<std::string>();
                cliques[det.getPos()].push_back(*j);
                maxCliqueSize = MAX2(maxCliqueSize, (size_t) 1);
            }
        }
        std::vector<std::string> firstClique;
        for (std::map<SUMOReal, std::vector<std::string> >::iterator m=cliques.begin(); firstClique.size()==0&&m!=cliques.end(); ++m) {
            if ((*m).second.size()==maxCliqueSize) {
                firstClique = (*m).second;
            }
        }
        std::vector<FlowDef> mflows; // !!! reserve
        SUMOTime t;
        for (t=startTime; t<endTime; t+=stepOffset) {
            FlowDef fd;
            fd.qPKW = 0;
            fd.qLKW = 0;
            fd.vLKW = 0;
            fd.vPKW = 0;
            fd.fLKW = 0;
            fd.isLKW = 0;
            mflows.push_back(fd);
        }
        for (std::vector<std::string>::iterator l=firstClique.begin(); l!=firstClique.end(); ++l) {
            const std::vector<FlowDef> &dflows = flows.getFlowDefs(*l);
            for (t=startTime; t<endTime; t+=stepOffset) {
                const FlowDef &srcFD = dflows[(int)(t/stepOffset) - startTime];
                FlowDef &fd = mflows[(int)(t/stepOffset) - startTime];
                fd.qPKW += srcFD.qPKW;
                fd.qLKW += srcFD.qLKW;
                fd.vLKW += (srcFD.vLKW / (SUMOReal) firstClique.size());
                fd.vPKW += (srcFD.vPKW / (SUMOReal) firstClique.size());
                fd.fLKW += (srcFD.fLKW / (SUMOReal) firstClique.size());
                fd.isLKW += (srcFD.isLKW / (SUMOReal) firstClique.size());
            }
        }
        static_cast<RODFEdge*>(into)->setFlows(mflows);
    }
}


ROEdge *
DFRONet::getEdge(const std::string &name) const
{
    return ro->getEdge(name);
}


void
DFRONet::buildDetectorDependencies(DFDetectorCon &detectors)
{
    // !!! this will not work when several detectors are lying on the same edge on different positions


    buildDetectorEdgeDependencies(detectors);
    // for each detector, compute the lists of predecessor and following detectors
    std::map<std::string, ROEdge*>::const_iterator i;
    for (i=myDetectorEdges.begin(); i!=myDetectorEdges.end(); ++i) {
        const DFDetector &det = detectors.getDetector((*i).first);
        if (!det.hasRoutes()) {
            continue;
        }
        // mark current detectors
        vector<DFDetector*> last;
        {
            const vector<string> &detNames = myDetectorsOnEdges.find((*i).second)->second;
            for (vector<string>::const_iterator j=detNames.begin(); j!=detNames.end(); ++j) {
                last.push_back((DFDetector*) &detectors.getDetector(*j));
            }
        }
        // iterate over the current detector's routes
        const std::vector<DFRORouteDesc*> &routes = det.getRouteVector();
        for (std::vector<DFRORouteDesc*>::const_iterator j=routes.begin(); j!=routes.end(); ++j) {
            const std::vector<ROEdge*> &edges2Pass = (*j)->edges2Pass;
            for (std::vector<ROEdge*>::const_iterator k=edges2Pass.begin()+1; k!=edges2Pass.end(); ++k) {
                if (myDetectorsOnEdges.find(*k)!=myDetectorsOnEdges.end()) {
                    const vector<string> &detNames = myDetectorsOnEdges.find(*k)->second;
                    // ok, consecutive detector found
                    for (vector<DFDetector*>::iterator l=last.begin(); l!=last.end(); ++l) {
                        // mark as follower of current
                        for (vector<string>::const_iterator m=detNames.begin(); m!=detNames.end(); ++m) {
                            ((DFDetector*) &detectors.getDetector(*m))->addPriorDetector((DFDetector*) &(*l));
                            (*l)->addFollowingDetector((DFDetector*) &detectors.getDetector(*m));
                        }
                    }
                    last.clear();
                    for (vector<string>::const_iterator m=detNames.begin(); m!=detNames.end(); ++m) {
                        last.push_back((DFDetector*) &detectors.getDetector(*m));
                    }
                }
            }
        }
    }
}


void
DFRONet::computeID4Route(DFRORouteDesc &desc) const
{
    ROEdge *first = *(desc.edges2Pass.begin());
    ROEdge *last = *(desc.edges2Pass.end()-1);
    pair<ROEdge*, ROEdge*> c(desc.edges2Pass[0], desc.edges2Pass.back());
    desc.routename = first->getID() + "_to_" + last->getID();
    if(myConnectionOccurences.find(c)==myConnectionOccurences.end()) {
        myConnectionOccurences[c] = 0;
    } else {
        myConnectionOccurences[c] = myConnectionOccurences[c] + 1;
        desc.routename = desc.routename + "_" + toString(myConnectionOccurences[c]);
    }
}


void
DFRONet::mesoJoin(DFDetectorCon &detectors, DFDetectorFlows &flows)
{
    buildDetectorEdgeDependencies(detectors);
    std::map<ROEdge*, std::vector<std::string> >::iterator i;
    for (i=myDetectorsOnEdges.begin(); i!=myDetectorsOnEdges.end(); ++i) {
        ROEdge *into = (*i).first;
        const std::vector<std::string> &dets = (*i).second;
        std::map<SUMOReal, std::vector<std::string> > cliques;
        // compute detector cliques
        for (std::vector<std::string>::const_iterator j=dets.begin(); j!=dets.end(); ++j) {
            const DFDetector &det = detectors.getDetector(*j);
            bool found = false;
            for (std::map<SUMOReal, std::vector<std::string> >::iterator k=cliques.begin(); !found&&k!=cliques.end(); ++k) {
                if (fabs((*k).first-det.getPos())<10.) {
                    (*k).second.push_back(*j);
                    found = true;
                }
            }
            if (!found) {
                cliques[det.getPos()] = std::vector<std::string>();
                cliques[det.getPos()].push_back(*j);
            }
        }
        // join detector cliques
        for (std::map<SUMOReal, std::vector<std::string> >::iterator m=cliques.begin(); m!=cliques.end(); ++m) {
            std::vector<std::string> clique = (*m).second;
            // do not join if only one
            if (clique.size()==1) {
                continue;
            }
            string nid;
            for (std::vector<std::string>::iterator n=clique.begin(); n!=clique.end(); ++n) {
                cout << *n << " ";
                if (n!=clique.begin()) {
                    nid = nid + "_";
                }
                nid = nid + *n;
            }
            cout << ":" << nid << endl;
            flows.mesoJoin(nid, (*m).second);
            detectors.mesoJoin(nid, (*m).second);
        }
    }
}



/****************************************************************************/

