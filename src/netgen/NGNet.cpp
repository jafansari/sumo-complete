/****************************************************************************/
/// @file    NGNet.cpp
/// @author  Markus Hartinger
/// @date    Mar, 2003
/// @version $Id$
///
// The class storing the generated network
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
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <netbuild/nodes/NBNode.h>
#include <netbuild/nodes/NBNodeCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/common/ToString.h>
#include "NGNet.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
NGNet::NGNet(NBNetBuilder &nb) throw()
        : myNetBuilder(nb)
{
    myLastID = 0;
}


NGNet::~NGNet() throw()
{
    {
        for (NGEdgeList::iterator ni=myEdgeList.begin(); ni!=myEdgeList.end(); ++ni) {
            delete *ni;
        }
    }
    {
        for (NGNodeList::iterator ni=myNodeList.begin(); ni!=myNodeList.end(); ++ni) {
            delete *ni;
        }
    }
}


std::string
NGNet::getNextFreeID() throw()
{
    return toString<int>(++myLastID);
}


NGNode*
NGNet::findNode(int xID, int yID) throw()
{
    for(NGNodeList::iterator ni = myNodeList.begin(); ni!= myNodeList.end(); ++ni) {
        if((*ni)->samePos(xID, yID)) {
            return *ni;
        }
    }
    return 0;
}


void
NGNet::createChequerBoard(int numX, int numY, SUMOReal spaceX, SUMOReal spaceY) throw()
{
    int ix, iy;
    NGNode *Node;
    for (ix=0; ix<numX; ix++) {
        for (iy=0; iy<numY; iy++) {
            // create Node
            string nodeID = toString<int>(ix) + "/" + toString<int>(iy);
            Node = new NGNode(nodeID, ix, iy);
            Node->setX(ix * spaceX);
            Node->setY(iy * spaceY);
            myNodeList.push_back(Node);
            // create Links
            if (ix > 0) {
                connect(Node, findNode(ix-1, iy));
            }
            if (iy > 0) {
                connect(Node, findNode(ix, iy-1));
            }
        }
    }
}


SUMOReal
NGNet::radialToX(SUMOReal radius, SUMOReal phi) throw()
{
    return cos(phi) * radius;
}


SUMOReal
NGNet::radialToY(SUMOReal radius, SUMOReal phi) throw()
{
    return sin(phi) * radius;
}


void
NGNet::createSpiderWeb(int numRadDiv, int numCircles, SUMOReal spaceRad, bool hasCenter) throw()
{
    if (numRadDiv < 3) numRadDiv = 3;
    if (numCircles < 1) numCircles = 1;

    int ir, ic;
    SUMOReal angle = (SUMOReal)(2*PI/numRadDiv);   // angle between radial divisions
    NGNode *Node;
    for (ir=1; ir<numRadDiv+1; ir++) {
        for (ic=1; ic<numCircles+1; ic++) {
            // create Node
            Node = new NGNode(
                       toString<int>(ir) + "/" + toString<int>(ic), ir, ic);
            Node->setX(radialToX((ic) * spaceRad, (ir-1) * angle));
            Node->setY(radialToY((ic) * spaceRad, (ir-1) * angle));
            myNodeList.push_back(Node);
            // create Links
            if (ir > 1) {
                connect(Node, findNode(ir-1, ic));
            }
            if (ic > 1) {
                connect(Node, findNode(ir, ic-1));
            }
            if (ir == numRadDiv) {
                connect(Node, findNode(1, ic));
            }
        }
    }
    if (hasCenter) {
        // node
        Node = new NGNode(getNextFreeID(), 0, 0, true);
        Node->setX(0);
        Node->setY(0);
        myNodeList.push_back(Node);
        // links
        for (ir=1; ir<numRadDiv+1; ir++) {
            connect(Node, findNode(ir, 1));
        }
    }
}


void
NGNet::connect(NGNode *node1, NGNode *node2) throw()
{
    string id1 = node1->getID() + "to" + node2->getID();
    string id2 = node2->getID() + "to" + node1->getID();
    NGEdge *link1 = new NGEdge(id1, node1, node2);
    NGEdge *link2 = new NGEdge(id2, node2, node1);
    myEdgeList.push_back(link1);
    myEdgeList.push_back(link2);
}


void
NGNet::toNB() const throw(ProcessError)
{
    for (NGNodeList::const_iterator i1=myNodeList.begin(); i1!=myNodeList.end(); i1++) {
        NBNode *node = (*i1)->buildNBNode(myNetBuilder);
        myNetBuilder.getNodeCont().insert(node);
    }
    for (NGEdgeList::const_iterator i2=myEdgeList.begin(); i2!=myEdgeList.end(); i2++) {
        NBEdge *edge = (*i2)->buildNBEdge(myNetBuilder);
        myNetBuilder.getEdgeCont().insert(edge);
    }
}


void
NGNet::add(NGNode *node) throw()
{
    myNodeList.push_back(node);
}


void
NGNet::add(NGEdge *edge) throw()
{
    myEdgeList.push_back(edge);
}


size_t
NGNet::nodeNo() const throw()
{
    return myNodeList.size();
}


/****************************************************************************/

