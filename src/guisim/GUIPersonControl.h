/****************************************************************************/
/// @file    GUIPersonControl.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 13.06.2012
/// @version $Id: MSPersonControl.h 12128 2012-03-19 11:54:26Z dkrajzew $
///
// GUI-version of the person control for building gui persons
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef GUIPersonControl_h
#define GUIPersonControl_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <microsim/MSPersonControl.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GUIPersonControl
 * @brief GUI-version of the person control for building gui persons
 */
class GUIPersonControl : public MSPersonControl {
public:
    /// constructor
    GUIPersonControl();


    /// destructor
    virtual ~GUIPersonControl();


    /** @brief Builds a new person
     * @param[in] pars The parameter
     * @param[in] plan This person's plan
     */
    MSPerson *buildPerson(const SUMOVehicleParameter* pars, MSPerson::MSPersonPlan* plan) const;


};


#endif

/****************************************************************************/
