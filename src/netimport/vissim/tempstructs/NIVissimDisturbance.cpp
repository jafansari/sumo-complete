//---------------------------------------------------------------------------//
//                        NIVissimDisturbance.cpp -  ccc
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
// Revision 1.12  2003/10/02 15:01:12  dkrajzew
// further work on Vissim-import
//
// Revision 1.11  2003/07/07 08:28:48  dkrajzew
// adapted the importer to the new node type description; some further work
//
// Revision 1.10  2003/06/16 08:01:57  dkrajzew
// further work on Vissim-import
//
// Revision 1.9  2003/06/05 11:46:56  dkrajzew
// class templates applied; documentation added
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H


#include <map>
#include <string>
#include <iostream>
#include <cassert>
#include <utils/convert/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Boundery.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNodeCont.h>
#include "NIVissimEdge.h"
#include "NIVissimConnection.h"
#include "NIVissimNodeDef.h"
#include "NIVissimDisturbance.h"
#include "NIVissimNodeParticipatingEdgeVector.h"

using namespace std;

NIVissimDisturbance::DictType NIVissimDisturbance::myDict;
int NIVissimDisturbance::myRunningID = 0;

int NIVissimDisturbance::refusedProhibits = 0;


NIVissimDisturbance::NIVissimDisturbance(int id,
                                         const std::string &name,
                                         const NIVissimExtendedEdgePoint &edge,
                                         const NIVissimExtendedEdgePoint &by,
                                         double timegap, double waygap,
                                         double vmax)
    : myID(id), myNode(-1), myName(name), myEdge(edge), myDisturbance(by),
    myTimeGap(timegap), myWayGap(waygap), myVMax(vmax)
{
}


NIVissimDisturbance::~NIVissimDisturbance()
{
}



bool
NIVissimDisturbance::dictionary(int id,
                                const std::string &name,
                                const NIVissimExtendedEdgePoint &edge,
                                const NIVissimExtendedEdgePoint &by,
                                double timegap, double waygap, double vmax)
{
    int nid = id;
    if(id<0) {
        nid = myRunningID++;
    }
    bool added = false;
    while(true) {
        NIVissimDisturbance *o =
            new NIVissimDisturbance(nid, name, edge, by,
                timegap, waygap, vmax);
        if(!dictionary(nid, o)) {
            delete o;
            nid = myRunningID++;
        } else {
            return true;
        }
    }
}


bool
NIVissimDisturbance::dictionary(int id, NIVissimDisturbance *o)
{
    DictType::iterator i=myDict.find(id);
    if(i==myDict.end()) {
        myDict[id] = o;
        return true;
    }
    return false;
}


NIVissimDisturbance *
NIVissimDisturbance::dictionary(int id)
{
    DictType::iterator i=myDict.find(id);
    if(i==myDict.end()) {
        return 0;
    }
    return (*i).second;
}

IntVector
NIVissimDisturbance::getWithin(const AbstractPoly &poly)
{
    IntVector ret;
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        if((*i).second->crosses(poly)) {
            ret.push_back((*i).second->myID);
        }
    }
    return ret;
}


void
NIVissimDisturbance::computeBounding()
{
    assert(myBoundery==0);
    Boundery *bound = new Boundery();
    bound->add(myEdge.getGeomPosition());
    bound->add(myDisturbance.getGeomPosition());
    myBoundery = bound;
    assert(myBoundery!=0&&myBoundery->xmax()>=myBoundery->xmin());
}



bool
NIVissimDisturbance::addToNode(NBNode *node)
{
    myNode = 0;
    NIVissimConnection *pc =
        NIVissimConnection::dictionary(myEdge.getEdgeID());
    NIVissimConnection *bc =
        NIVissimConnection::dictionary(myDisturbance.getEdgeID());
    if(pc==0 && bc==0) {
        // This has not been tested completely, yet
        // Both competing abstract edges are normal edges
        // We have to find a crossing point, build a node here,
        //  split both edges and add the connections
        NIVissimEdge *e1 = NIVissimEdge::dictionary(
            myEdge.getEdgeID());
        NIVissimEdge *e2 = NIVissimEdge::dictionary(
            myDisturbance.getEdgeID());
        MsgHandler::getWarningInstance()->inform(
            string("Ugly split to prohibit '") + toString<int>(e1->getID())
            + string("' by '") + toString<int>(e2->getID())
            + string("'."));
        Position2D pos = e1->crossesEdgeAtPoint(e2);
        string id1 =
            toString<int>(e1->getID()) + string("x")
            + toString<int>(e2->getID());
        string id2 =
            toString<int>(e2->getID()) + string("x")
            + toString<int>(e1->getID());
        NBNode *node1 = NBNodeCont::retrieve(id1);
        NBNode *node2 = NBNodeCont::retrieve(id2);
        NBNode *node = 0;
        assert(node1==0||node2==0);
        if(node1==0&&node2==0) {
            refusedProhibits++;
			return false;
/*            node = new NBNode(id1, pos.x(), pos.y(), "priority");
            if(!NBNodeCont::insert(node)) {
                 "nope, NIVissimDisturbance" << endl;
                throw 1;
            }*/
        } else {
            node = node1==0 ? node2 : node1;
        }
            NBEdgeCont::splitAt(
                NBEdgeCont::retrievePossiblySplitted(
                    toString<int>(e1->getID()), myEdge.getPosition()),
                    node);
            NBEdgeCont::splitAt(
                NBEdgeCont::retrievePossiblySplitted(
                    toString<int>(e2->getID()), myDisturbance.getPosition()),
                    node);
			// !!! in some cases, one of the edges is not being build because it's too short
			// !!! what to do in these cases?
			NBEdge *mayDriveFrom = NBEdgeCont::retrieve(
				toString<int>(e1->getID()) + string("[0]"));
			NBEdge *mayDriveTo = NBEdgeCont::retrieve(
				toString<int>(e1->getID()) + string("[1]"));
			NBEdge *mustStopFrom = NBEdgeCont::retrieve(
				toString<int>(e2->getID()) + string("[0]"));
			NBEdge *mustStopTo = NBEdgeCont::retrieve(
				toString<int>(e2->getID()) + string("[1]"));
			if(mayDriveFrom!=0&&mayDriveTo!=0&&mustStopFrom!=0&&mustStopTo!=0) {
	            node->addSortedLinkFoes(
					NBConnection(mayDriveFrom, mayDriveTo),
					NBConnection(mayDriveFrom, mayDriveTo));
			} else {
	            refusedProhibits++;
				return false;
				// !!! warning
			}
//        }
    } else if(pc!=0 && bc==0) {
        // The prohibited abstract edge is a connection, the other
        //  is not;
        // The connection will be prohibitesd by all connections
        //  outgoing from the "real" edge

        NBEdge *e = NBEdgeCont::retrievePossiblySplitted(
            toString<int>(myDisturbance.getEdgeID()), myDisturbance.getPosition());
        if(e->getFromNode()==e->getToNode()) {
            MsgHandler::getWarningInstance()->inform(
                string("Could not prohibit '")
                + toString<int>(myEdge.getEdgeID()) + string("' by '")
                + toString<int>(myDisturbance.getEdgeID())
                + string("'."));
            refusedProhibits++;
            // What to do with dummy edges?
            return false;
        }
            // get the begin of the prohibited connection
        string id_pcoe = toString<int>(pc->getFromEdgeID());
        string id_pcie = toString<int>(pc->getToEdgeID());
        NBEdge *pcoe = NBEdgeCont::retrievePossiblySplitted(id_pcoe, id_pcie, true);
        NBEdge *pcie = NBEdgeCont::retrievePossiblySplitted(id_pcie, id_pcoe, false);
            // check whether it's ending node is the node the prohibited
            //  edge end at
        if(pcoe!=0&&pcie!=0&&pcoe->getToNode()==e->getToNode()) {
            // if so, simply prohibit the connections
            NBNode *node = e->getToNode();
            size_t noLanes = e->getNoLanes();
            const EdgeVector &connected = e->getConnected();
            for(EdgeVector::const_iterator i=connected.begin(); i!=connected.end(); i++) {
                node->addSortedLinkFoes(
                    NBConnection(e, *i),
                    NBConnection(pcoe, pcie));
            }
        } else {
            MsgHandler::getWarningInstance()->inform(
                string("Would have to split edge '")
                + e->getID() + string("' to build a prohibition"));
            refusedProhibits++;
            // quite ugly - why was it not build?
            return false;
            /*
            string nid1 = e->getID() + "[0]";
            string nid2 = e->getID() + "[1]";

            if(NBEdgeCont::splitAt(e, node)) {
                node->addSortedLinkFoes(
                        NBConnection(
                            NBEdgeCont::retrieve(nid1),
                            NBEdgeCont::retrieve(nid2)
                        ),
                        getConnection(node, myEdge.getEdgeID())
                    );
            }
            */
        }
    } else if(bc!=0 && pc==0) {
        // The prohibiting abstract edge is a connection, the other
        //  is not;
        // We have to split the other one and add the prohibition
        //  description

        NBEdge *e = NBEdgeCont::retrievePossiblySplitted(
            toString<int>(myEdge.getEdgeID()), myEdge.getPosition());
        string nid1 = e->getID() + "[0]";
        string nid2 = e->getID() + "[1]";
        if( e->getFromNode()==e->getToNode()) {
            MsgHandler::getWarningInstance()->inform(
                string("Could not prohibit '")
                + toString<int>(myEdge.getEdgeID()) + string("' by '")
                + toString<int>(myDisturbance.getEdgeID())
                + string("'."));
            refusedProhibits++;
            // What to do with dummy edges?
            return false;
        }
            // get the begin of the prohibiting connection
        string id_bcoe = toString<int>(bc->getFromEdgeID());
        string id_bcie = toString<int>(bc->getToEdgeID());
        NBEdge *bcoe = NBEdgeCont::retrievePossiblySplitted(id_bcoe, id_bcie, true);
        NBEdge *bcie = NBEdgeCont::retrievePossiblySplitted(id_bcie, id_bcoe, false);
            // check whether it's ending node is the node the prohibited
            //  edge end at
        if(bcoe!=0&&bcie!=0&&bcoe->getToNode()==e->getToNode()) {
            // if so, simply prohibit the connections
            NBNode *node = e->getToNode();
            size_t noLanes = e->getNoLanes();
            const EdgeVector &connected = e->getConnected();
            for(EdgeVector::const_iterator i=connected.begin(); i!=connected.end(); i++) {
                node->addSortedLinkFoes(
                    NBConnection(bcoe, bcie),
                    NBConnection(e, *i));
            }
        } else {
            MsgHandler::getWarningInstance()->inform(
                string("Would have to split edge '")
                + e->getID() + string("' to build a prohibition"));
            refusedProhibits++;
            return false;
            /*
            // quite ugly - why was it not build?
            if(NBEdgeCont::splitAt(e, node)) {
                node->addSortedLinkFoes(
                        getConnection(node, myDisturbance.getEdgeID()),
                        NBConnection(
                            NBEdgeCont::retrieve(nid1),
                            NBEdgeCont::retrieve(nid2)
                        )
                    );
            }
            */
        }
    } else {
        // both the prohibiting and the prohibited abstract edges
        //  are connections
        // We can retrieve the conected edges and add the desription
        NBConnection conn1 = getConnection(node, myDisturbance.getEdgeID());
        NBConnection conn2 = getConnection(node, myEdge.getEdgeID());
        if(!conn1.check()||!conn2.check()) {
            refusedProhibits++;
            return false;
        }
        node->addSortedLinkFoes(conn1, conn2);
    }
    return true;
}


NBConnection
NIVissimDisturbance::getConnection(NBNode *node, int aedgeid)
{
    if(NIVissimEdge::dictionary(myEdge.getEdgeID())==0) {
        NIVissimConnection *c = NIVissimConnection::dictionary(aedgeid);
        NBEdge *from =
            node->getPossiblySplittedIncoming(toString<int>(c->getFromEdgeID()));
        NBEdge *to =
            node->getPossiblySplittedOutgoing(toString<int>(c->getToEdgeID()));

        // source is a connection
        return NBConnection(toString<int>(c->getFromEdgeID()), from,
            toString<int>(c->getToEdgeID()), to);
    } else {
        MsgHandler::getWarningInstance()->inform(
            "NIVissimDisturbance: no connection");
        return NBConnection(0, 0);
//        throw 1; // !!! what to do?
    }

}

void
NIVissimDisturbance::clearDict()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


void
NIVissimDisturbance::dict_SetDisturbances()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        NIVissimDisturbance *d = (*i).second;
        NIVissimAbstractEdge::dictionary(d->myEdge.getEdgeID())->addDisturbance((*i).first);
        NIVissimAbstractEdge::dictionary(d->myDisturbance.getEdgeID())->addDisturbance((*i).first);
    }
/*    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        delete (*i).second;
    }
    */
}



void
NIVissimDisturbance::reportRefused()
{
    if(refusedProhibits>0) {
        MsgHandler::getWarningInstance()->inform(
            string("Warning: Could not build ") + toString<size_t>(refusedProhibits)
            + string(" of ") + toString<size_t>(myDict.size())
            + string(" disturbances."));
    }
}




/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "NIVissimDisturbance.icc"
//#endif

// Local Variables:
// mode:C++
// End:


