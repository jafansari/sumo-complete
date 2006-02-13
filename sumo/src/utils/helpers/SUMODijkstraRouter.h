#ifndef SUMODijkstraRouter_h
#define SUMODijkstraRouter_h
//---------------------------------------------------------------------------//
//                        SUMODijkstraRouter.h -
//  The dijkstra-router
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Mon, 25 July 2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
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
// $Log$
// Revision 1.6  2006/02/13 07:29:20  dkrajzew
// made dijkstra-router checking for closures optionally
//
// Revision 1.5  2006/01/26 08:39:50  dksumo
// made the abstract router usable within microsim and routers
//
// Revision 1.4  2005/10/06 13:39:50  dksumo
// using of a configuration file rechecked
//
// Revision 1.3  2005/09/23 13:17:33  dksumo
// debugging building for sumo0.8.3
//
// Revision 1.2  2005/09/20 06:13:04  dksumo
// floats and doubles replaced by SUMOReal; warnings removed
//
// Revision 1.1  2005/09/09 12:56:09  dksumo
// helpers added
//
// Revision 1.1  2005/08/01 13:45:17  dksumo
// rerouting within the simulation added
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

#include <string>
#include <functional>
#include <vector>
#include <set>
#include <limits>
#include <queue>
#include <string>
#include <utils/common/InstancePool.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include "SUMOAbstractRouter.h"


/* =========================================================================
 * class definitions
 * ======================================================================= */
/*
template<class E, class V>
typedef inline bool ( ProhibitionFunction )(const E *edge, const V *vehicle);
*/

template<class E, class V>
struct prohibited_withRestrictions {
public:
    inline bool operator()(const E *edge, const V *vehicle) {
        if(std::find(myProhibited.begin(), myProhibited.end(), edge)!=myProhibited.end()) {
	        return true;
        }
	    return edge->prohibits(vehicle);
    }

    void prohibit(const std::vector<E*> &toProhibit) {
        myProhibited = toProhibit;
    }

protected:
    std::vector<E*> myProhibited;

};

template<class E, class V>
struct prohibited_noRestrictions {
public:
    inline bool operator()(const E *edge, const V *vehicle) {
        return false;
    }
};


/**
 * @class SUMODijkstraRouter
 * @brief Lays the given route over the edges using the dijkstra algorithm
 */
template<class E, class V, class PF >
class SUMODijkstraRouter : public SUMOAbstractRouter<E, V>, public PF {
public:
    /// Constructor
    SUMODijkstraRouter(size_t noE, bool unbuildIsWarningOnly)
        : myNoE(noE), myReusableEdgeLists(true), myReusableEdgeInfoLists(true),
		myUnbuildIsWarningOnly(unbuildIsWarningOnly)
    { }

    /// Destructor
    virtual ~SUMODijkstraRouter() { }

    /**
     * @struct EdgeInfo
     * A definition about a route's edge with the effort needed to reach it and
     *  the information about the previous edge.
     */
    class EdgeInfo {
    public:
        /// Constructor
        EdgeInfo()
            : edge(0), effort(0), prev(0)
        {
        }


        /// Constructor
        EdgeInfo(const E *edgeArg, SUMOReal effortArg, EdgeInfo *prevArg)
            : edge(edgeArg), effort(effortArg), prev(prevArg)
        {
        }

        /// Constructor
        EdgeInfo(const E *edgeArg, SUMOReal effortArg, EdgeInfo *prevArg, SUMOReal distArg)
            : edge(edgeArg), effort(effortArg), prev(prevArg), dist(distArg)
        {
        }

        /// The current edge
        const E *edge;

        /// Effort to reach the edge
        SUMOReal effort;

        /// The previous edge
        EdgeInfo *prev;

		/// Distance from the begin
		SUMOReal dist;

    };

    /**
     * @class EdgeInfoByEffortComperator
     * Class to compare (and so sort) nodes by their effort
     */
    class EdgeInfoByEffortComperator /*: public std::less<ROEdge*>*/ {
    public:
        /// Constructor
        explicit EdgeInfoByEffortComperator() { }

        /// Destructor
        ~EdgeInfoByEffortComperator() { }

        /// Comparing method
        bool operator()(EdgeInfo *nod1, EdgeInfo *nod2) const {
            return nod1->effort>nod2->effort;
        }
    };


    /** @brief Builds the route between the given edges using the minimum afford at the given time
        The definition of the afford depends on the wished routing scheme */
    virtual void compute(const E *from, const E *to, const V * const vehicle,
        SUMOTime time, std::vector<const E*> &into) {

		// get structures to reuse
        std::vector<bool> *visited = myReusableEdgeLists.getFreeInstance();
        if(visited==0) {
            visited = new std::vector<bool>(myNoE, false);
        } else {
            for(size_t i=0; i<myNoE; i++) {
                (*visited)[i] = false; // !!!
            }
        }
        EdgeInfoCont *storage = myReusableEdgeInfoLists.getFreeInstance();
        if(storage==0) {
            storage = new EdgeInfoCont(myNoE);
        }
        storage->reset();

        // check the nodes
        if(from==0||to==0) {
			throw std::exception();
        }

		// begin computation
        std::priority_queue<EdgeInfo*,
			std::vector<EdgeInfo*>,
            EdgeInfoByEffortComperator> frontierList;
			// add begin node
	    const E *actualKnot = from;
	    if(from != 0) {
            EdgeInfo *ei = storage->add(actualKnot, 0, 0);
            frontierList.push(ei);
	    }

        // loop
	    while(!frontierList.empty()) {
            // use the node with the minimal length
            EdgeInfo *minimumKnot = frontierList.top();
            const E *minEdge = minimumKnot->edge;
            frontierList.pop();
                // check whether the destination node was already reached
            if(minEdge == to) {
				buildPathFrom(minimumKnot, into);
                clearTemporaryStorages(visited, storage);
				return;
                //return ret;
            }
			//assert(minEdge->getNumericalID()<9);
            (*visited)[minEdge->getNumericalID()] = true;
            SUMOReal effort = (SUMOReal) (minimumKnot->effort
		    	+ minEdge->getEffort((SUMOTime) (time + minimumKnot->effort)));
    		// check all ways from the node with the minimal length
            size_t i = 0;
            size_t length_size = minEdge->getNoFollowing();
            for(i=0; i<length_size; i++) {
                const E* help = minEdge->getFollower(i);
                // check whether it can be used
                if(PF::operator()(help, vehicle)) {
					continue;
				}
                //
                if( !(*visited)[help->getNumericalID()] && effort < storage->getEffort(help) ) {
                    if(help!=from) {
                        frontierList.push(storage->add(help, effort, minimumKnot));
                    }
                }
		    }
	    }
        if(!myUnbuildIsWarningOnly) {
            MsgHandler::getErrorInstance()->inform("No connection between '" + from->getID() + "' and '" + to->getID() + "' found.");
        } else {
            WRITE_WARNING("No connection between '" + from->getID() + "' and '" + to->getID() + "' found.");
        }
        clearTemporaryStorages(visited, storage);
    }

public:
    /// Builds the path from marked edges
    void buildPathFrom(EdgeInfo *rbegin, std::vector<const E *> &edges) {
//        B ret;
		std::deque<const E*> tmp;
        while(rbegin!=0) {
            tmp.push_front((E *) rbegin->edge); // !!!
            rbegin = rbegin->prev;
        }
		std::copy(tmp.begin(), tmp.end(), std::back_inserter(edges));
    }

public:
    /**
     * @class EdgeInfoCont
     * A class holding the instances of effort-to-reach and predeccessor
     *  information for every ede within the network.
     * This class is used instead of the former saving of these values within
     *  the edges to allow parallel route computation in multithreading mode.
     */
    class EdgeInfoCont {
    public:
        /// Constructor
        EdgeInfoCont(size_t toAlloc)
            : myEdgeInfos(toAlloc+1, EdgeInfo())
        { }

        /// Destructor
        ~EdgeInfoCont() { }

        /// Adds the information about the effort to get to an edge and its predeccessing edge
        EdgeInfo *add(const E *edgeArg, SUMOReal effortArg, EdgeInfo *prevArg)
        {
            EdgeInfo *ret = &(myEdgeInfos[edgeArg->getNumericalID()]);
            ret->edge = edgeArg; // !!! may be set within the constructor
            ret->effort = effortArg;
            ret->prev = prevArg;
            ret->dist = 0;
            return ret;
        }

        /// Adds the information about the effort to get to an edge and its predeccessing edge
        EdgeInfo *add(const E *edgeArg, SUMOReal effortArg, EdgeInfo *prevArg,
			SUMOReal distArg)
        {
            EdgeInfo *ret = &(myEdgeInfos[edgeArg->getNumericalID()]);
            ret->edge = edgeArg; // !!! may be set within the constructor
            ret->effort = effortArg;
            ret->prev = prevArg;
            ret->dist = distArg;
            return ret;
        }

        /// Resets all effort-information
        void reset()
        {
            for(typename std::vector<EdgeInfo>::iterator i=myEdgeInfos.begin(); i!=myEdgeInfos.end(); i++) {
                (*i).effort = std::numeric_limits<SUMOReal>::max();
            }
        }


        /** @brief Returns the effort to get to the specify edge
            The value is valid if the edge was already visited */
        SUMOReal getEffort(const E *to) const
        {
            return myEdgeInfos[to->getNumericalID()].effort;
        }

    private:
        /// The container of edge information
        std::vector<EdgeInfo> myEdgeInfos;

    };

protected:
    /// Saves the temporary storages for further usage
    void clearTemporaryStorages(std::vector<bool> *edgeList,
        EdgeInfoCont *consecutionList)
    {
        myReusableEdgeLists.addFreeInstance(edgeList);
        myReusableEdgeInfoLists.addFreeInstance(consecutionList);
    }


protected:
    /// The network to use
    size_t myNoE;

    /// A container for reusage of examined edges lists
    InstancePool<std::vector<bool> > myReusableEdgeLists;

    /// A container for reusage of edge consecution lists
    InstancePool<EdgeInfoCont> myReusableEdgeInfoLists;

	bool myUnbuildIsWarningOnly;


};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

