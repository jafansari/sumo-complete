/****************************************************************************/
/// @file    MSSubject.h
/// @author  Christian Roessel
/// @date    Fri Nov 21 12:44:14 2003
/// @version $Id$
///
//	�missingDescription�
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
#ifndef MSSubject_h
#define MSSubject_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSObserver.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cassert>
#ifdef _MSC_VER
#include <utils/helpers/msvc6_TypeTraits.h>
#else
#include <utils/helpers/gcc_TypeTraits.h>
#endif


// See Gamma et al. "Design Patterns" for information about the
// Observer pattern.

// To observe various quantities of the class ObservedType, create
// dummy-classes in a namespace related to the concrete
// Observable. e.g. if the concrete observable is the class Vehicle,
// put dummy classes Move{}, LaneChange{} etc. in a namespace veh. Use
// this dummy classes as third template parameter. It is convenient to
// declare some typedefs in the veh namespace:
// namespace veh
// {
//     class Move{};
//     class LaneChange{};
//
//     typedef MSSubject< MSVehicle, false, Move >      MoveSubject;
//     typedef MSSubject< MSVehicle, true, LaneChange > LaneChangeSubject;
//
//     typedef MoveSubject::Observer       MoveObserver;
//     typedef LaneChangeSubject::Observer LaneChangeObserver;
// }
//
// Then derive ObservedType (Vehicle in the example) from MoveSubject
// and from LaneChangeSubject.
//
// In the case of several observable quantities, the concrete subject
// (Vehicle) must override the inherited attach and detach
// methods. Just forward the call to the appropriate base class:
//
//     void attach( veh::LaneChangeObserver* toAttach ) {
//             veh::LaneChangeSubject::attach( toAttach );
//         }
//     void detach( veh::LaneChangeObserver* toDetach ) {
//             veh::LaneChangeSubject::detach( toDetach );
//         }
//     void attach( veh::MoveObserver* toAttach ) {
//             veh::MoveSubject::attach( toAttach );
//         }
//     void detach( veh::MoveObserver* toDetach ) {
//             veh::MoveSubject::detach( toDetach );
//         }
//
// If you just have one quantity to observe, omit the third template
// parameter and do not override the detach and attach methods.
//
// The parameter passesObserved selects the way the Observer is
// updated. If true, a reference to the observed object (Vehicle) is
// passed, so the Observer need not hold a reference/pointer to the
// observable/subject. If passesObserved === false, nothing is passed
// and the Observer must know the observable/subject.
//
// If an observable state of the concrete subject changes, call the
// notify method of the appropriate base:
//             veh::MoveSubject::notify();
//             veh::LaneChangeSubject::notify( *this );
// or just notify(maybe with parameter) if you have only one
// observable quantity.

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSSubject
 */
class EmptyType
    {};

template<
class ObservedType
, class ObservedQuantity = EmptyType
>
class MSSubjectPassesObserved
{
public:
    typedef ObservedType Observed;
    typedef typename Loki::TypeTraits< Observed >::ParameterType ParameterType;

    typedef MSObserverPassesObserved<
    Observed
    , ObservedQuantity
    > Observer;

    typedef Observer* ObserverPtr;
    typedef std::vector< ObserverPtr > Observers;
    typedef typename Observers::iterator ObserversIt;


    void attach(ObserverPtr toAttach)
    {
        observersM.push_back(toAttach);
    }

    void detach(ObserverPtr toDetach)
    {
        ObserversIt eraseIt =
            std::find(observersM.begin(), observersM.end(), toDetach);
        assert(eraseIt != observersM.end());
        observersM.erase(eraseIt);
    }

protected:

    MSSubjectPassesObserved(void)
            : observersM()
    {}

    virtual ~MSSubjectPassesObserved(void)
    {
        observersM.clear();
    }

    void notify(ParameterType aObserved)
    {
        for (ObserversIt observer = observersM.begin();
                observer != observersM.end(); ++observer) {
            (*observer)->update(aObserved);
        }
    }

    Observers observersM;

private:

    MSSubjectPassesObserved(const MSSubjectPassesObserved&);
    MSSubjectPassesObserved& operator=(const MSSubjectPassesObserved&);
};



template<
class ObservedType
, class ObservedQuantity = EmptyType
>
class MSSubject
{
public:
    typedef ObservedType Observed;

    typedef MSObserver<
    Observed
    , ObservedQuantity
    > Observer;

    typedef Observer* ObserverPtr;
    typedef std::vector< ObserverPtr > Observers;
    typedef typename Observers::iterator ObserversIt;

    void attach(ObserverPtr toAttach)
    {
        observersM.push_back(toAttach);
    }

    void detach(ObserverPtr toDetach)
    {
        ObserversIt eraseIt =
            std::find(observersM.begin(), observersM.end(), toDetach);
        assert(eraseIt != observersM.end());
        observersM.erase(eraseIt);
    }

protected:

    MSSubject(void)
            : observersM()
    {}

    virtual ~MSSubject(void)
    {
        observersM.clear();
    }

    void notify(void)
    {
        std::for_each(observersM.begin(), observersM.end(),
                      std::mem_fun(&Observer::update));
    }

    Observers observersM;

private:

    MSSubject(const MSSubject&);
    MSSubject& operator=(const MSSubject&);
};


#endif

/****************************************************************************/

