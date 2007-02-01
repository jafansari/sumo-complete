/****************************************************************************/
/// @file    MSVehicleContainer.h
/// @author  Christian Roessel
/// @date    Mon, 12 Mar 2001
/// @version $Id: $
///
// vehicles sorted by their departures
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
#ifndef MSVehicleContainer_h
#define MSVehicleContainer_h
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

#include <vector>
#include <iostream>
#include "MSNet.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicleContainer
 * A storage for vehicles, mainly used by the vehicle/route loading structures
 * and the emit control. Stores vehicles in a heap of vehiclevector/departure-
 * pairs.
 */
class MSVehicleContainer
{
public:
    /// definition of a list of vehicles which have the same departure time
    typedef std::vector<MSVehicle*> VehicleVector;

    /** definition of a structure storing the departure time and a list
        of vehicles leaving at this time */
    typedef std::pair<SUMOTime, VehicleVector> VehicleDepartureVector;

public:
    /// Constructor
    MSVehicleContainer(size_t capacity=10);

    /// Destructor
    ~MSVehicleContainer();

    /// Adds a single vehicle
    void add(MSVehicle *veh);

    /// Adds a container with vehicles departing at the given time
    void add(SUMOTime time, const VehicleVector &cont);

    /** @briefMoves vehicles from the given container
        (The given container is empty afterwards) */
    void moveFrom(MSVehicleContainer &cont);

    /// Returns the information whether any vehicles want to depart at the given time
    bool anyWaitingFor(SUMOTime time) const;

    /// Returns the uppermost vehicle vector
    const VehicleVector &top();

    /// Returns the time the uppermost vehicle vector is assigned to
    SUMOTime topTime() const;

    /// Removes the uppermost vehicle vector
    void pop();

    /// Returns the information whether the container is empty
    bool isEmpty() const;

    /// Returns the size of the container
    size_t size() const;

    /// Prints the container (the departure times)
    void showArray() const;

    /// Prints the contents of the container
    friend std::ostream &operator << (std::ostream &strm,
                                      MSVehicleContainer &cont);

private:
    /** @brief Replaces the existing single departure time vector by the one given
    */
    void addReplacing(const VehicleDepartureVector &cont);

    /** Returns the information whether the container must be extended */
    bool isFull() const;

    /// Sort-criterion for vehicle departure lists
    class VehicleDepartureVectorSortCrit
    {
    public:
        /// comparison operator
        bool operator()(const VehicleDepartureVector& e1,
                        const VehicleDepartureVector& e2) const;
    };

    /// Searches for the VehicleDepartureVector with the wished depart
    class DepartFinder
    {
    public:
        /// constructor
        explicit DepartFinder(SUMOTime time);

        /// comparison operator
        bool operator()(const VehicleDepartureVector& e) const;

    private:
        /// the searched departure time
        SUMOTime myTime;
    };

    /// Number of elements in heap
    int currentSize;

    /// Definition of the heap type
    typedef std::vector<VehicleDepartureVector> VehicleHeap;

    /// The vehicle vector heap
    VehicleHeap array;

    /// Moves the elements down
    void percolateDown(int hole);

};


#endif

/****************************************************************************/

