#include <string>
#include <vector>
#include <cmath>
//#include <cstdlib>
#include <iostream>
#include "ROEdge.h"
#include "RORouteDef.h"
#include "RORoute.h"
#include "RORouter.h"
//#include "RORouteAlternative.h"
#include "RORouteAlternativesDef.h"

using namespace std;

RORouteAlternativesDef::RORouteAlternativesDef(const std::string &id, 
                                               size_t lastUsed,
                                               double gawronBeta, double gawronA)
    : RORouteDef(id)/*, _current(-1)*/, _lastUsed(lastUsed),
    _gawronBeta(gawronBeta), _gawronA(gawronA)
{
}


RORouteAlternativesDef::~RORouteAlternativesDef()
{
    for(AlternativesVector::iterator i=_alternatives.begin(); i!=_alternatives.end(); i++) {
        delete *i;
    }
}


void 
RORouteAlternativesDef::addLoadedAlternative(RORoute *alt)
{
    _alternatives.push_back(alt);
}



ROEdge *
RORouteAlternativesDef::getFrom() const 
{
    // check whether the item was correctly initialised
    if(_alternatives.size()==0) {
        throw 1; // !!!
    }
    return _alternatives[0]->getFirst();
}


ROEdge *
RORouteAlternativesDef::getTo() const
{
    // check whether the item was correctly initialised
    if(_alternatives.size()==0) {
        throw 1; // !!!
    }
    return _alternatives[0]->getLast();
}


RORoute *
RORouteAlternativesDef::buildCurrentRoute(RORouter &router, long begin) 
{
    // recompute duration of the last route used
    _alternatives[_lastUsed]->recomputeCosts(begin);
    // build a new route to test whether it is better 
    //  !!! after some iterations, no further routes should be build
    RORoute *opt = 
        new RORoute(_id, 0, 1, router.compute(getFrom(), getTo(), begin));
    opt->setCosts(opt->recomputeCosts(begin));
    // check whether the same route was already used
    _lastUsed = findRoute(opt);
    _newRoute = true;
    // delete the route when it already existed
    if(_lastUsed>=0) {
        // this is not completely correct as the value does not
        //  come from the simulation itself but from the computing 
        //  using the network !!!
        _alternatives[_lastUsed]->setCosts(opt->getCosts());
        delete opt;
        _newRoute = false;
        return _alternatives[_lastUsed];
    }
    // return the build route
    return opt;
}


int
RORouteAlternativesDef::findRoute(RORoute *opt) const
{
    for(size_t i=0; i<_alternatives.size(); i++) {
        if(_alternatives[i]->equals(opt)) {
            return i;
        }
    }
    return -1;
}


void 
RORouteAlternativesDef::addAlternative(RORoute *current, long begin)
{
    // add the route when it's new
    if(_lastUsed<0) {
        _alternatives.push_back(current);
        _lastUsed = _alternatives.size()-1;
    }
    // recompute the costs
    AlternativesVector::iterator i;
    for(i=_alternatives.begin(); i!=_alternatives.end(); i++) {
        RORoute *alt = *i;
        // apply changes for old routes only
        if((*i)!=current||!_newRoute) {
            // recompute the costs for old routes
            double oldCosts = alt->getCosts();
            double newCosts = alt->recomputeCosts(begin);
            alt->setCosts(_gawronBeta * newCosts + (1.0-_gawronBeta) * oldCosts);
        }
    }
    // compute the propabilities
    for(i=_alternatives.begin(); i!=_alternatives.end()-1; i++) {
        RORoute *pR = *i;
        for(AlternativesVector::iterator j=i; j!=_alternatives.end(); j++) {
            RORoute *pS = *j;
            // see [Gawron, 1998] (4.2)
            double delta = 
                (pS->getCosts() - pR->getCosts()) / 
                (pS->getCosts() + pR->getCosts());
            // see [Gawron, 1998] (4.3a, 4.3b)
            double newPR = gawronF(pR->getPropability(), pS->getPropability(), delta);
            double newPS = pR->getPropability() + pS->getPropability() - newPR;
            pR->setPropability(newPR);
            pS->setPropability(newPS);
        }
    }
    // find the route to use
    double chosen = ( (double)rand() / (double)(RAND_MAX+1) * _alternatives.size()); 
    size_t pos = 0;
    for(i=_alternatives.begin(); i!=_alternatives.end()-1; i++, pos++) {
        chosen = chosen - (*i)->getPropability();
        if(chosen<=0) {
            _lastUsed = pos;
            return;
        }
    }
}


double 
RORouteAlternativesDef::gawronF(double pdr, double pds, double x) 
{
    return (pdr*(pdr+pds)*gawronG(_gawronA, x)) / 
        (pdr*gawronG(_gawronA, x)+pds);
}


double 
RORouteAlternativesDef::gawronG(double a, double x) 
{
    return exp((a*x)/(1.0-(x*x))); // !!! ??
}


void 
RORouteAlternativesDef::xmlOutCurrent(std::ostream &res) const
{
    _alternatives[_lastUsed]->xmlOut(res);
}


void 
RORouteAlternativesDef::xmlOutAlternatives(std::ostream &os) const
{
    os << "   <routealt id=\"" << _id << "\" last=\""
        << _lastUsed << "\">" << endl;
    for(size_t i=0; i!=_alternatives.size(); i++) {
//        if(i!=_current) {
            RORoute *alt = _alternatives[i];
            os << "      <route cost=\"" << alt->getCosts()
                << "\" propability=\"" << alt->getPropability()
                << "\">";
            alt->xmlOutEdges(os);
            os << "</route>" << endl;
//        }
    }
    os << "   </routealt>" << endl;
}


