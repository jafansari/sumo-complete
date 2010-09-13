/****************************************************************************/
/// @file    AGStreet.h
/// @author  Piotr Woznica
/// @date    July 2010
/// @version $Id$
///
// Represents a SUMO edge and contains people and work densities
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef AGSTREET_H
#define AGSTREET_H


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class AGStreet
 * @brief A model of the street in the city.
 *
 * AGStreet represents a street in the city. It contains all model relevant
 * properties and is associated with a ROEdge of the routing network.
 */
class AGStreet
{
public:
	AGStreet(const ROEdge* edge, float popD = 0, float workD = 0);

	/** @brief Provides the length of this edge.
	 *
	 * @return the length of this edge
	 */
	SUMOReal getLength() const throw();

	/** @brief Provides the id of this edge.
	 *
	 * @return the id of this edge
	 */
	const std::string& getName() const throw();

	/** @brief Provides the number of persons living in this street.
	 *
	 * @return the number of inhabitants
	 */
	int getPopulation() const throw();

	/** @brief Modifies the number of persons living in this street.
	 *
	 * @param[in] pop the new number of inhabitants
	 */
	void setPopulation(int pop) throw();

	/** @brief Provides the number of work places in this street.
	 *
	 * @return the number of work places
	 */
	int getWorkplaceNumber() const throw();

	/** @brief Modifies the number of work places in this street.
	 *
	 * @param[in] work the new number of work places
	 */
	void setWorkplaceNumber(int work) throw();

	/** @brief Offers access to the underlying ROEdge object.
	 *
	 * @return the associated ROEdge
	 */
	const ROEdge* getEdge() const throw();

	/** @brief Prints a summary of the properties of this street to standard
	 * output.
	 */
	void print() const throw();

private:
	const ROEdge* edge;
	int pop;
	int work;
};

#endif

/****************************************************************************/
