/****************************************************************************/
/// @file    NBNodeShapeComputer.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: $
///
// This class computes shapes of junctions
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

#include <algorithm>
#include <utils/geom/Position2DVector.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/StdDefs.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include "NBNode.h"
#include "NBNodeShapeComputer.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
NBNodeShapeComputer::NBNodeShapeComputer(const NBNode &node,
        std::ofstream * const out)
        : myNode(node), myOut(out)
{}

NBNodeShapeComputer::~NBNodeShapeComputer()
{}


Position2DVector
NBNodeShapeComputer::compute()
{
    Position2DVector ret;
    bool isDeadEnd = false;
    if (myNode._allEdges.size()==1) {
        isDeadEnd = true;
    }
    if (myNode._allEdges.size()==2&&myNode.getIncomingEdges().size()==1) {
        if (myNode.getIncomingEdges()[0]->isTurningDirectionAt(&myNode, myNode.getOutgoingEdges()[0])) {
            isDeadEnd = true;
        }
    }
    if (isDeadEnd) {
        ret = computeNodeShapeByCrosses();
        return ret;
    }


    bool simpleContinuation = myNode.isSimpleContinuation();
    ret = computeContinuationNodeShape(simpleContinuation);
    // add the geometry of internal lanes
    if (OptionsSubSys::getOptions().getBool("add-internal-links")) {
        addInternalGeometry();
    }
    if (ret.size()<3) {
        ret = computeNodeShapeByCrosses();
    }
    {
        if (myOut!=0) {
            for (int i=0; i<(int) ret.size(); ++i) {
                (*myOut) << "   <poi id=\"end_" << myNode.getID() << "_"
                << toString(i) << "\" type=\"nodeshape.end\" color=\"1,0,1\""
                << " x=\"" << ret[i].x() << "\" y=\"" << ret[i].y() << "\"/>"
                << endl;
            }
        }
    }
    return ret;
}


void
NBNodeShapeComputer::addInternalGeometry()
{
    /*
    for(EdgeVector::const_iterator i=myNode._incomingEdges->begin(); i!=myNode._incomingEdges->end(); i++) {
        size_t noLanesEdge = (*i)->getNoLanes();
        for(size_t j=0; j<noLanesEdge; j++) {
            const EdgeLaneVector *elv = (*i)->getEdgeLanesFromLane(j);
            for(EdgeLaneVector::const_iterator k=elv->begin(); k!=elv->end(); k++) {
                if((*k).edge==0) {
                    continue;
                }
                Position2DVector shape =
                    computeInternalLaneShape(*i, j, (*k).edge, (*k).lane);
                if(shape.length()==0) {
                    continue;
                }
                Position2DVector l(shape);
                l.move2side(1.5);
                Position2DVector r(shape);
                l.move2side(-1.5);
                myPoly.push_back(shape);
            }
        }
    }
    */
}


void
computeSameEnd(Position2DVector& l1, Position2DVector &l2)
{
    Line2D sub(l1.lineAt(0).getPositionAtDistance(100), l1[1]);
    Line2D tmp(sub);
    tmp.rotateDegAtP1(90);
    tmp.extrapolateBy(100);
    if (l1.intersects(tmp.p1(), tmp.p2())) {
        SUMOReal offset1 = l1.intersectsAtLengths(tmp)[0];
        Line2D tl1 = Line2D(
                         l1.lineAt(0).getPositionAtDistance(offset1),
                         l1[1]);
        tl1.extrapolateBy(100);
        l1.replaceAt(0, tl1.p1());
    }
    if (l2.intersects(tmp.p1(), tmp.p2())) {
        SUMOReal offset2 = l2.intersectsAtLengths(tmp)[0];
        Line2D tl2 = Line2D(
                         l2.lineAt(0).getPositionAtDistance(offset2),
                         l2[1]);
        tl2.extrapolateBy(100);
        l2.replaceAt(0, tl2.p1());
    }
}


void
replaceLastChecking(Position2DVector &g, bool decenter,
                    Position2DVector counter,
                    size_t counterLanes, SUMOReal counterDist,
                    int laneDiff)
{
    counter.extrapolate(100);
    Position2D counterPos = counter.positionAtLengthPosition(counterDist);
    if (GeomHelper::distance(g[-1], counterPos)<SUMO_const_laneWidth*(SUMOReal) counterLanes) {
        g.replaceAt(g.size()-1, counterPos);
    } else {
        g.push_back_noDoublePos(counterPos);
    }
    if (decenter) {
        Line2D l(g[-2], g[-1]);
        SUMOReal factor = laneDiff%2!=0 ? SUMO_const_halfLaneAndOffset : SUMO_const_laneWidthAndOffset;
        l.move2side(-factor);//SUMO_const_laneWidthAndOffset);
        g.replaceAt(g.size()-1, l.p2());
    }
}


void
replaceFirstChecking(Position2DVector &g, bool decenter,
                     Position2DVector counter,
                     size_t counterLanes, SUMOReal counterDist,
                     int laneDiff)
{
    counter.extrapolate(100);
    Position2D counterPos = counter.positionAtLengthPosition(counterDist);
    if (GeomHelper::distance(g[0], counterPos)<SUMO_const_laneWidth*(SUMOReal) counterLanes) {
        g.replaceAt(0, counterPos);
    } else {
        g.push_front_noDoublePos(counterPos);
    }
    if (decenter) {
        Line2D l(g[0], g[1]);
        SUMOReal factor = laneDiff%2!=0 ? SUMO_const_halfLaneAndOffset : SUMO_const_laneWidthAndOffset;
        /*
            SUMO_const_laneWidthAndOffset * (SUMOReal) (counterLanes-1)
            + SUMO_const_halfLaneAndOffset * (SUMOReal) (counterLanes%2);
            */
        l.move2side(-factor);//SUMO_const_laneWidthAndOffset);
        g.replaceAt(0, l.p1());
    }
}



Position2DVector
NBNodeShapeComputer::computeContinuationNodeShape(bool simpleContinuation)
{
    if (myNode._allEdges.size()<2) {
        return Position2DVector();
    }

    EdgeVector::const_iterator i, j;
    EdgeVector::iterator i2;
    std::map<NBEdge*, std::vector<NBEdge*> > same;
    std::map<NBEdge*, Position2DVector> geomsCCW;
    std::map<NBEdge*, Position2DVector> geomsCW;


    std::map<NBEdge*, NBEdge*> ccwBoundary;
    std::map<NBEdge*, NBEdge*> cwBoundary;
    for (i=myNode._allEdges.begin(); i!=myNode._allEdges.end(); i++) {
        cwBoundary[*i] = *i;
        ccwBoundary[*i] = *i;
    }

    // check which edges are parallel
    for (i=myNode._allEdges.begin(); i!=myNode._allEdges.end()-1; i++) {
        Position2DVector g1 =
            myNode.hasIncoming(*i)
            ? (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth)
            : (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        geomsCCW[*i] = (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        geomsCW[*i] = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        Line2D l1 = g1.lineAt(0);
        Line2D tmp = geomsCCW[*i].lineAt(0);
        tmp.extrapolateBy(100);
        geomsCCW[*i].replaceAt(0, tmp.p1());
        tmp = geomsCW[*i].lineAt(0);
        tmp.extrapolateBy(100);
        geomsCW[*i].replaceAt(0, tmp.p1());
        for (j=i+1; j!=myNode._allEdges.end(); j++) {
            Position2DVector g2 =
                myNode.hasIncoming(*j)
                ? (*j)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth)
                : (*j)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            geomsCCW[*j] = (*j)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            geomsCW[*j] = (*j)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            Line2D l2 = g2.lineAt(0);
            tmp = geomsCCW[*j].lineAt(0);
            tmp.extrapolateBy(100);
            geomsCCW[*j].replaceAt(0, tmp.p1());
            tmp = geomsCW[*j].lineAt(0);
            tmp.extrapolateBy(100);
            geomsCW[*j].replaceAt(0, tmp.p1());
            if (fabs(l1.atan2DegreeAngle()-l2.atan2DegreeAngle())<1) {
                if (same.find(*i)==same.end()) {
                    same[*i] = std::vector<NBEdge*>();
                }
                if (same.find(*j)==same.end()) {
                    same[*j] = std::vector<NBEdge*>();
                }
                if (find(same[*i].begin(), same[*i].end(), *j)==same[*i].end()) {
                    same[*i].push_back(*j);
                }
                if (find(same[*j].begin(), same[*j].end(), *i)==same[*j].end()) {
                    same[*j].push_back(*i);
                }
            }
        }
    }

    // compute unique direction list
    std::vector<NBEdge*> newAll = myNode._allEdges;
    std::map<NBEdge*, std::vector<NBEdge*> >::iterator k;
    bool changed = true;
    while (changed) {
        changed = false;
        for (i2=newAll.begin(); i2!=newAll.end()&&!changed; ++i2) {
            if ((*i2)->getBasicType()!=NBEdge::EDGEFUNCTION_NORMAL) {
                newAll.erase(i2);
                changed = true;
                continue;
            }
            std::vector<NBEdge*> other = same[*i2];
            for (j=other.begin(); j!=other.end(); ++j) {
                std::vector<NBEdge*>::iterator k =
                    find(newAll.begin(), newAll.end(), *j);
                if (k!=newAll.end()) {
                    if (myNode.hasIncoming(*i2)) {
                        if (myNode.hasIncoming(*j)) {}
                        else {
                            geomsCW[*i2] = geomsCW[*j];
                            cwBoundary[*i2] = *j;
                            computeSameEnd(geomsCW[*i2], geomsCCW[*i2]);
                        }
                    } else {
                        if (myNode.hasIncoming(*j)) {
                            ccwBoundary[*i2] = *j;
                            geomsCCW[*i2] = geomsCCW[*j];
                            computeSameEnd(geomsCW[*i2], geomsCCW[*i2]);
                        } else {}
                    }
                    newAll.erase(k);
                    changed = true;
                }
            }
        }
    }

    // compute boundaries
    if (newAll.size()<2) {
        return Position2DVector();
    }

    // combine all geoms
    std::map<NBEdge*, bool> myExtended;
    std::map<NBEdge*, SUMOReal> distances;
    for (i=newAll.begin(); i!=newAll.end(); ++i) {
        EdgeVector::const_iterator cwi = i;
        cwi++;
        if (cwi==newAll.end()) {
            cwi = newAll.begin();
        }
        EdgeVector::const_iterator ccwi = i;
        if (ccwi==newAll.begin()) {
            ccwi = newAll.end() - 1;
        } else {
            ccwi--;
        }

        assert(geomsCCW.find(*i)!=geomsCCW.end());
        assert(geomsCW.find(*ccwi)!=geomsCW.end());
        assert(geomsCW.find(*cwi)!=geomsCW.end());
        SUMOReal twoPI = (SUMOReal)(2.*3.1415926535897);
        SUMOReal pi = (SUMOReal) 3.1415926535897;
        SUMOReal angleI = geomsCCW[*i].lineAt(0).atan2PositiveAngle();
        SUMOReal angleCCW = geomsCW[*ccwi].lineAt(0).atan2PositiveAngle();
        SUMOReal angleCW = geomsCW[*cwi].lineAt(0).atan2PositiveAngle();
        SUMOReal ccad;
        SUMOReal cad;
        if (angleI>angleCCW) {
            ccad = angleI - angleCCW;
        } else {
            ccad = twoPI - angleCCW + angleI;
        }

        if (angleI>angleCW) {
            cad = twoPI - angleI + angleCW;
        } else {
            cad = angleCW - angleI;
        }

        if (ccad<0) {
            ccad = (SUMOReal) 3.1415926535897 * (SUMOReal) 2.0 + ccad;
        }
        if (ccad>(SUMOReal)(2.*3.1415926535897)) {
            ccad -= (SUMOReal)(2.*3.1415926535897);
        }
        if (cad<0) {
            cad = (SUMOReal) 3.1415926535897 * (SUMOReal) 2.0 + cad;
        }
        if (cad>(SUMOReal)(2.*3.1415926535897)) {
            cad -= (SUMOReal)(2.*3.1415926535897);
        }

        if (simpleContinuation&&ccad<(SUMOReal)(45./180.*PI)) {
            ccad += twoPI;
        }
        if (simpleContinuation&&cad<(SUMOReal)(45./180.*PI)) {
            cad += twoPI;
        }


        if (fabs(ccad-cad)<(SUMOReal) 0.1&&*cwi==*ccwi) {
            // only two edges, almost parallel
            //  compute the position where both would meet
            //   (assume the junction position for this)
            /*
                        // ok, just one further thing: maybe we have the number of lanes
                        //  has changed; let's apply the proper geometry in this case
                        if((*i)->getNoLanes()<(*cwi)->getNoLanes()) {
                            int diff = (*cwi)->getNoLanes() - (*i)->getNoLanes();
                            SUMOReal factor1 =
                                SUMO_const_laneWidthAndOffset * (SUMOReal) (diff-1)
                                + SUMO_const_halfLaneAndOffset * (SUMOReal) (diff%2);
                            SUMOReal factor2 = 0;
                            Position2DVector g = (*i)->getGeometry();
                            g.move2side(factor1);
                            (*i)->setGeometry(g);

                        }
                        /
                        if((*i)->getNoLanes()<(*cwi)->getNoLanes()&&myNode.hasIncoming(*i)) {
                            int diff = (*i)->getNoLanes() - (*cwi)->getNoLanes();
                            SUMOReal factor =
                                SUMO_const_laneWidthAndOffset * (SUMOReal) (diff-1)
                                + SUMO_const_halfLaneAndOffset * (SUMOReal) (diff%2);
                            Position2DVector g = (*i)->getGeometry();
                            g.move2side(factor, -1);
                            (*i)->setGeometry(g);
                        }
                        */
            // compute the mean position between both edges ends ...
            Position2D p;
            if (myExtended.find(*ccwi)!=myExtended.end()) {
                p = geomsCCW[*ccwi][0];
                p.add(geomsCW[*ccwi][0]);
                p.mul(0.5);
                /*
                if(myNode.hasIncoming(*ccwi)) {
                p = geomsCCW[*ccwi][0];
                p.add(geomsCW[*ccwi][0]);
                } else {
                p = (*ccwi)->getGeometry()[0];
                }
                */
            } else {
                p = geomsCCW[*ccwi][0];
                p.add(geomsCW[*ccwi][0]);
                p.add(geomsCCW[*i][0]);
                p.add(geomsCW[*i][0]);
                p.mul(0.25);
                /*
                if(myNode.hasIncoming(*i)) {
                p = (*i)->getGeometry().at(-1);
                p.add((*ccwi)->getGeometry()[0]);
                } else {
                p = (*i)->getGeometry()[0];
                p.add((*ccwi)->getGeometry().at(-1));
                }
                p.mul(.5);
                */
            }
            // ... compute the distance to this point ...
            SUMOReal dist = geomsCCW[*i].nearest_position_on_line_to_point(p);
            if (dist<0) {
                // ok, we have the problem that even the extrapolated geometry
                //  does not reach the point
                // in this case, the geometry has to be extenden... too bad ...
                // ... let's append the mean position to the geometry
                Position2DVector g = (*i)->getGeometry();
                if (myNode.hasIncoming(*i)) {
                    g.push_back_noDoublePos(p);
                } else {
                    g.push_front_noDoublePos(p);
                }
                (*i)->setGeometry(g);
                // and rebuild previous information
                geomsCCW[*i] = (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
                geomsCCW[*i].extrapolate(100);
                geomsCW[*i] = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
                geomsCW[*i].extrapolate(100);
                // the distance is now = zero (the point we have appended)
                distances[*i] = 100;
                myExtended[*i] = true;
            } else {
                if (!simpleContinuation) {
                    // let us put some geometry stuff into it
                    dist = (SUMOReal) 1.5 + dist;
                }
                distances[*i] = dist;
            }

        } else {
            if (ccad>(90.+45.)/180.*pi&&cad>(90.+45.)/180.*PI&&*ccwi!=*cwi/*&&cwBoundary[*i]==ccwBoundary[*i]*/) {
                // ok, in this case we have a street which is opposite to at least
                //  two edges which have a very similar angle
                // let's skip the computation for now, because we want the current
                //  edge to be almost at these edges' crossing point and maybe
                //  one of the edges' crossings is not yet computed...
                continue;
            } else {
                if (ccad<cad) {
                    if (!simpleContinuation) {
                        if (geomsCCW[*i].intersects(geomsCW[*ccwi])) {
                            distances[*i] = (SUMOReal) 1.5 + geomsCCW[*i].intersectsAtLengths(geomsCW[*ccwi])[0];
                            if (*cwi!=*ccwi&&geomsCW[*i].intersects(geomsCCW[*cwi])) {
                                SUMOReal a1 = distances[*i];
                                SUMOReal a2 = (SUMOReal) 1.5 + geomsCW[*i].intersectsAtLengths(geomsCCW[*cwi])[0];
                                if (ccad>(SUMOReal)((90.+45.)/180.*pi)&&cad>(SUMOReal)((90.+45.)/180.*PI)) {
                                    SUMOReal mmin = MIN2(distances[*cwi], distances[*ccwi]);
                                    if (mmin>100) {
                                        distances[*i] = (SUMOReal) 5. + (SUMOReal) 100. - (SUMOReal)(mmin-100);  //100 + 1.5;
                                    }
                                } else  if (a2>a1&&a2-a1<(SUMOReal) 10) {
                                    distances[*i] = a2;
                                }
                            }
                        } else {
                            if (*cwi!=*ccwi&&geomsCW[*i].intersects(geomsCCW[*cwi])) {
                                distances[*i] = (SUMOReal) 1.5 + geomsCW[*i].intersectsAtLengths(geomsCCW[*cwi])[0];
                            } else {
                                distances[*i] = (SUMOReal)(100. + 1.5);
                            }
                        }
                    } else {
                        if (geomsCCW[*i].intersects(geomsCW[*ccwi])) {
                            distances[*i] = geomsCCW[*i].intersectsAtLengths(geomsCW[*ccwi])[0];
                        } else {
                            distances[*i] = (SUMOReal) 100.;
                        }
                    }
                } else {
                    if (!simpleContinuation) {
                        if (geomsCW[*i].intersects(geomsCCW[*cwi])) {
                            distances[*i] = (SUMOReal)(1.5 + geomsCW[*i].intersectsAtLengths(geomsCCW[*cwi])[0]);
                            if (*cwi!=*ccwi&&geomsCCW[*i].intersects(geomsCW[*ccwi])) {
                                SUMOReal a1 = distances[*i];
                                SUMOReal a2 = (SUMOReal)(1.5 + geomsCCW[*i].intersectsAtLengths(geomsCW[*ccwi])[0]);
                                if (ccad>(90.+45.)/180.*pi&&cad>(90.+45.)/180.*PI) {
                                    SUMOReal mmin = MIN2(distances[*cwi], distances[*ccwi]);
                                    if (mmin>100) {
                                        distances[*i] = (SUMOReal)(5. + 100. - (mmin-100.));  //100 + 1.5;
                                    }
                                } else if (a2>a1&&a2-a1<10) {
                                    distances[*i] = a2;
                                }
                            }
                        } else {
                            if (*cwi!=*ccwi&&geomsCCW[*i].intersects(geomsCW[*ccwi])) {
                                distances[*i] = (SUMOReal) 1.5 + geomsCCW[*i].intersectsAtLengths(geomsCW[*ccwi])[0];
                            } else {
                                distances[*i] = (SUMOReal)(100. + 1.5);
                            }
                        }
                    } else {
                        if (geomsCW[*i].intersects(geomsCCW[*cwi])) {
                            distances[*i] = geomsCW[*i].intersectsAtLengths(geomsCCW[*cwi])[0];
                        } else {
                            distances[*i] = (SUMOReal) 100;
                        }
                    }
                }
            }
        }
    }


    for (i=newAll.begin(); i!=newAll.end(); ++i) {
        if (distances.find(*i)!=distances.end()) {
            continue;
        }
//        assert(!simpleContinuation);
        EdgeVector::const_iterator cwi = i;
        cwi++;
        if (cwi==newAll.end()) {
            cwi = newAll.begin();
        }
        EdgeVector::const_iterator ccwi = i;
        if (ccwi==newAll.begin()) {
            ccwi = newAll.end() - 1;
        } else {
            ccwi--;
        }

        assert(geomsCW.find(*ccwi)!=geomsCW.end());
        assert(geomsCW.find(*cwi)!=geomsCW.end());
        Position2D p1 = distances.find(*cwi)!=distances.end()&&distances[*cwi]!=-1
                        ? geomsCCW[*cwi].positionAtLengthPosition(distances[*cwi])
                        : geomsCCW[*cwi].positionAtLengthPosition((SUMOReal) -.1);
        Position2D p2 = distances.find(*ccwi)!=distances.end()&&distances[*ccwi]!=-1
                        ? geomsCW[*ccwi].positionAtLengthPosition(distances[*ccwi])
                        : geomsCW[*ccwi].positionAtLengthPosition((SUMOReal) -.1);
        Line2D l(p1, p2);
        l.extrapolateBy(1000);
        SUMOReal angleI = geomsCCW[*i].lineAt(0).atan2PositiveAngle();
        SUMOReal angleCCW = geomsCW[*ccwi].lineAt(0).atan2PositiveAngle();
        SUMOReal angleCW = geomsCW[*cwi].lineAt(0).atan2PositiveAngle();
        SUMOReal ccad;
        SUMOReal cad;
        SUMOReal twoPI = (SUMOReal)(2.*3.1415926535897);
        if (angleI>angleCCW) {
            ccad = angleI - angleCCW;
        } else {
            ccad = twoPI - angleCCW + angleI;
        }

        if (angleI>angleCW) {
            cad = twoPI - angleI + angleCW;
        } else {
            cad = angleCW - angleI;
        }

        if (ccad<0) {
            ccad = (SUMOReal) 3.1415926535897 * (SUMOReal) 2.0 + ccad;
        }
        if (ccad>2.*3.1415926535897) {
            ccad -= (SUMOReal)(2.*3.1415926535897);
        }
        if (cad<0) {
            cad = (SUMOReal) 3.1415926535897 * (SUMOReal) 2.0 + cad;
        }
        if (cad>2.*3.1415926535897) {
            cad -= (SUMOReal)(2.*3.1415926535897);
        }
        SUMOReal offset = 0;
        int laneDiff = (*i)->getNoLanes() - (*ccwi)->getNoLanes();
        if (*ccwi!=*cwi) {
            laneDiff -= (*cwi)->getNoLanes();
        }
        laneDiff = 0;
        if (myNode.hasIncoming(*i)&&(*ccwi)->getNoLanes()%2==1) {
            laneDiff = 1;
        }
        if (myNode.hasOutgoing(*i)&&(*cwi)->getNoLanes()%2==1) {
            laneDiff = 1;
        }

//        for(m=msame.begin(); m!=msame.end(); ++m) {
        Position2DVector g = (*i)->getGeometry();
        Position2DVector counter;
        if (myNode.hasIncoming(*i)) {
            if (myNode.hasOutgoing(*ccwi)&&myNode.hasOutgoing(*cwi)) {
                replaceLastChecking(g, (*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                    (*cwi)->getGeometry(), (*cwi)->getNoLanes(), distances[*cwi],
                                    laneDiff);
                /*
                                        counter = (*cwi)->getGeometry();
                                        counter.extrapolate(100);
                                        if(GeomHelper::distance(g.at(-1), counter.positionAtLengthPosition(distances[*cwi]))<3.*(*cwi)->getNoLanes()) {
                                            g.replaceAt(g.size()-1, counter.positionAtLengthPosition(distances[*cwi]));
                                        } else {
                                        g.push_back_noDoublePos(
                                            counter.positionAtLengthPosition(distances[*cwi]));
                                        }
                                        */
            } else {
                counter = (*ccwi)->getGeometry();
                if (myNode.hasIncoming(*ccwi)) {
                    counter = counter.reverse();
                }
                replaceLastChecking(g, (*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                    counter, (*ccwi)->getNoLanes(), distances[*ccwi],
                                    laneDiff);
                /*
                counter.extrapolate(100);
                if(GeomHelper::distance(g.at(-1), counter.positionAtLengthPosition(distances[*ccwi]))<3.*(*ccwi)->getNoLanes()) {
                    g.replaceAt(g.size()-1, counter.positionAtLengthPosition(distances[*ccwi]));
                } else {
                g.push_back_noDoublePos(
                    counter.positionAtLengthPosition(distances[*ccwi]));
                }
                */
            }
        } else {
            if (myNode.hasIncoming(*ccwi)&&myNode.hasIncoming(*cwi)) {
                //counter = (*ccwi)->getGeometry().reverse();
                replaceFirstChecking(g,(*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                     (*ccwi)->getGeometry().reverse(), (*ccwi)->getNoLanes(), distances[*ccwi],
                                     laneDiff);
                /*
                counter.extrapolate(100);
                if(GeomHelper::distance(g[0], counter.positionAtLengthPosition(distances[*ccwi]))<3.*(*ccwi)->getNoLanes()) {
                    g.replaceAt(0, counter.positionAtLengthPosition(distances[*ccwi]));
                } else {
                g.push_front_noDoublePos(
                    counter.positionAtLengthPosition(distances[*ccwi]));
                }
                */
            } else {
                counter = (*cwi)->getGeometry();
                if (myNode.hasIncoming(*cwi)) {
                    counter = counter.reverse();
                }
                replaceFirstChecking(g,(*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                     counter, (*cwi)->getNoLanes(), distances[*cwi],
                                     laneDiff);
                /*
                counter.extrapolate(100);
                if(GeomHelper::distance(g[0], counter.positionAtLengthPosition(distances[*cwi]))<3.*(*cwi)->getNoLanes()) {
                    g.replaceAt(0, counter.positionAtLengthPosition(distances[*cwi]));
                } else {
                g.push_front_noDoublePos(
                    counter.positionAtLengthPosition(distances[*cwi]));
                }
                */
            }
        }
        (*i)->setGeometry(g);

        if (cwBoundary[*i]!=*i) {
            Position2DVector g = cwBoundary[*i]->getGeometry();
            Position2DVector counter = (*cwi)->getGeometry();
            if (myNode.hasIncoming(*cwi)) {
                counter = counter.reverse();
            }
            //counter.extrapolate(100);
            if (myNode.hasIncoming(cwBoundary[*i])) {
                replaceLastChecking(g, (*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                    counter, (*cwi)->getNoLanes(), distances[*cwi],
                                    laneDiff);
                /*
                if(GeomHelper::distance(g.at(-1), counter.positionAtLengthPosition(distances[*cwi]))<3.*(*cwi)->getNoLanes()) {
                    g.replaceAt(g.size()-1, counter.positionAtLengthPosition(distances[*cwi]));
                } else {
                g.push_back_noDoublePos(
                    counter.positionAtLengthPosition(distances[*cwi]));
                }
                */
            } else {
                replaceFirstChecking(g,(*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                     counter, (*cwi)->getNoLanes(), distances[*cwi],
                                     laneDiff);
                /*
                if(GeomHelper::distance(g[0], counter.positionAtLengthPosition(distances[*cwi]))<3.*(*cwi)->getNoLanes()) {
                    g.replaceAt(0, counter.positionAtLengthPosition(distances[*cwi]));
                } else {
                g.push_front_noDoublePos(
                    counter.positionAtLengthPosition(distances[*cwi]));
                }
                */
            }
            cwBoundary[*i]->setGeometry(g);
            myExtended[cwBoundary[*i]] = true;
            geomsCW[*i] = cwBoundary[*i]->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        } else {
            geomsCW[*i] = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);

        }

        //geomsCW[*i] = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        geomsCW[*i].extrapolate(100);

        if (ccwBoundary[*i]!=*i) {
            Position2DVector g = ccwBoundary[*i]->getGeometry();
            Position2DVector counter = (*ccwi)->getGeometry();
            if (myNode.hasIncoming(*ccwi)) {
                counter = counter.reverse();
            }
            //counter.extrapolate(100);
            if (myNode.hasIncoming(ccwBoundary[*i])) {
                replaceLastChecking(g, (*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                    counter, (*ccwi)->getNoLanes(), distances[*ccwi],
                                    laneDiff);
                /*
                if(GeomHelper::distance(g.at(-1), counter.positionAtLengthPosition(distances[*ccwi]))<3.*(*ccwi)->getNoLanes()) {
                    g.replaceAt(g.size()-1, counter.positionAtLengthPosition(distances[*ccwi]));
                } else {
                g.push_back_noDoublePos(
                    counter.positionAtLengthPosition(distances[*ccwi]));
                }
                */
            } else {
                replaceFirstChecking(g,(*i)->getLaneSpreadFunction()==NBEdge::LANESPREAD_CENTER,
                                     counter, (*cwi)->getNoLanes(), distances[*cwi],
                                     laneDiff);
                /*
                                        if(GeomHelper::distance(g[0], counter.positionAtLengthPosition(distances[*cwi]))<3.*(*cwi)->getNoLanes()) {
                                            g.replaceAt(0, counter.positionAtLengthPosition(distances[*cwi]));
                                        } else {
                                        g.push_front_noDoublePos(
                                            counter.positionAtLengthPosition(distances[*cwi]));
                                        }
                                        */
            }
            ccwBoundary[*i]->setGeometry(g);
            myExtended[ccwBoundary[*i]] = true;
            geomsCCW[*i] = ccwBoundary[*i]->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        } else {
            geomsCCW[*i] = (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);

        }
        //geomsCCW[*i] = (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
        geomsCCW[*i].extrapolate(100);

        computeSameEnd(geomsCW[*i], geomsCCW[*i]);

        // and rebuild previous information
        if (((*cwi)->getNoLanes()+(*ccwi)->getNoLanes())>(*i)->getNoLanes()) {
            offset = 5;
        }
        if (ccwBoundary[*i]!=cwBoundary[*i]) {
            offset = 5;
        }

        myExtended[*i] = true;
        distances[*i] = 100 + offset;
    }

    // build
    Position2DVector ret;
    for (i=newAll.begin(); i!=newAll.end(); ++i) {
        Position2DVector l = geomsCCW[*i];
        SUMOReal len = l.length();
        SUMOReal offset = distances[*i];
        if (offset==-1) {
            offset = (SUMOReal) -.1;
        }
        Position2D p;
        if (len>=offset) {
            p = l.positionAtLengthPosition(offset);
        } else {
            p = l.positionAtLengthPosition(len);
        }
        ret.push_back_noDoublePos(p);
        //
        l = geomsCW[*i];
        len = l.length();
        if (len>=offset) {
            p = l.positionAtLengthPosition(offset);
        } else {
            p = l.positionAtLengthPosition(len);
        }
        ret.push_back_noDoublePos(p);
    }
    return ret;
}



Position2DVector
rotateAround(const Position2DVector &what, const Position2D &at, SUMOReal rot)
{
    Position2DVector rret;
    Position2DVector ret = what;
    ret.resetBy(-at.x(), -at.y());
    {
        SUMOReal x = ret[0].x() * cos(rot) + ret[0].y() * sin(rot);
        SUMOReal y = ret[0].y() * cos(rot) - ret[0].x() * sin(rot);
        rret.push_back(Position2D(x, y));
    }
    {
        SUMOReal x = ret[1].x() * cos(rot) + ret[1].y() * sin(rot);
        SUMOReal y = ret[1].y() * cos(rot) - ret[1].x() * sin(rot);
        rret.push_back(Position2D(x, y));
    }
    rret.resetBy(at.x(), at.y());
    return rret;
}

Position2DVector
NBNodeShapeComputer::computeNodeShapeByCrosses()
{
    Position2DVector ret;
    EdgeVector::const_iterator i;
    for (i=myNode._allEdges.begin(); i!=myNode._allEdges.end(); i++) {
        // compute crossing with normal
        {
            Position2DVector edgebound1 = (*i)->getCCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            Position2DVector edgebound2 = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            Position2DVector cross;
            cross.push_back(edgebound1[0]);
            cross.push_back(edgebound1[1]);
            cross = rotateAround(cross, myNode.getPosition(), (SUMOReal)(90/180.*3.1415926535897932384626433832795));
            cross.extrapolate(500);

            edgebound1.extrapolate(500);
            if (cross.intersects(edgebound1)) {
                ret.push_back_noDoublePos(cross.intersectsAtPoint(edgebound1));
            }
            if (cross.intersects(edgebound2)) {
                ret.push_back_noDoublePos(cross.intersectsAtPoint(edgebound2));
            }
        }
        /*
        {
            Position2DVector edgebound = (*i)->getCWBoundaryLine(myNode, SUMO_const_halfLaneWidth);
            Position2DVector cross;
            cross.push_back(edgebound[0]);
            cross.push_back_noDoublePos(edgebound[1]);
            cross = rotateAround(cross, myNode.getPosition(), (SUMOReal) (90/180.*3.1415926535897932384626433832795));
            if(cross.intersects(edgebound)) {
                ret.push_back_noDoublePos(cross.intersectsAtPoint(edgebound));
            } else {
                Position2DVector cross2 = cross;
                cross2.extrapolate(500);
                edgebound.extrapolate(500);
                if(cross2.intersects(edgebound)) {
                    ret.push_back_noDoublePos(cross2.intersectsAtPoint(edgebound));
                }
            }
        }
        */
    }
    {
        if (myOut!=0) {
            for (int i=0; i<(int) ret.size(); ++i) {
                (*myOut) << "   <poi id=\"cross1_" << myNode.getID() << "_" <<
                toString(i) << "\" type=\"nodeshape.cross1\" color=\"0,0,1\""
                << " x=\"" << ret[i].x() << "\" y=\"" << ret[i].y() << "\"/>"
                << endl;
            }
        }
    }
    return ret;
}



/****************************************************************************/

