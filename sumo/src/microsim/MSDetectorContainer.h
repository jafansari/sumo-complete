#ifndef MSDETECTORCONTAINER_H
#define MSDETECTORCONTAINER_H

/**
 * @file   MSDetectorContainer.h
 * @author Christian Roessel
 * @date   Started Fri Sep 26 19:11:26 2003
 * @version $Id$
 * @brief
 *
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

#include "MSDetectorContainerBase.h"
#include "MSVehicle.h"
#include "MSPredicates.h"
#include <algorithm>
#include <functional>
#include <list>

template< typename InnerContainer >
struct MSDetectorContainer : public MSDetectorContainerBase
{
    typedef typename InnerContainer::const_iterator ContainerConstIt;
    typedef InnerContainer InnerCont;

    void enterDetectorByMove( MSVehicle* veh )
        {
            typedef typename InnerContainer::value_type ContainerItem;
            containerM.push_front( ContainerItem( veh ) );
        }

    void enterDetectorByEmitOrLaneChange( MSVehicle* veh )
        {
            typedef typename InnerContainer::value_type ContainerItem;
            typedef typename InnerContainer::iterator ContainerIt;
            typedef typename Predicate::PosGreaterC< ContainerItem >
                PosGreaterPredicate;
            ContainerIt insertIt =
                std::find_if( containerM.begin(), containerM.end(),
                              std::bind2nd(
                                  PosGreaterPredicate(), veh->pos() ) );
            containerM.insert( insertIt, ContainerItem( veh ) );
        }

    void leaveDetectorByMove( void )
        {
            containerM.pop_back();
        }

    void leaveDetectorByLaneChange( MSVehicle* veh )
        {
            typedef typename InnerContainer::value_type ContainerItem;
            typedef typename InnerContainer::iterator ContainerIt;
            typedef typename Predicate::VehEqualsC< ContainerItem >
                ErasePredicate;
            ContainerIt eraseIt =
                std::find_if( containerM.begin(), containerM.end(),
                              std::bind2nd(
                                  ErasePredicate(), veh ) );
            containerM.erase( eraseIt );
        }

    virtual void update( void )
        {}

    MSDetectorContainer( void )
        : containerM()
        {}

    virtual ~MSDetectorContainer( void )
        {
            containerM.clear();
        }

    InnerContainer containerM;

};

namespace DetectorContainer
{
    typedef MSDetectorContainer< std::list< MSVehicle* > > Vehicles;
}


#endif // MSDETECTORCONTAINER_H

// Local Variables:
// mode:C++
// End:
