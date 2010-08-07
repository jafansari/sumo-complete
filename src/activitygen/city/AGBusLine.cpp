/****************************************************************************/
/// @file    AGBusLine.cpp
/// @author  Piotr Woznica
/// @date    July 2010
/// @version $Id$
///
// Bus line of the city: contains all the buses of this line
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2010 TUM (Technische Universität München, http://www.tum.de/)
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
#include <utility>
#include <sstream>
#include <string>
#include <list>
#include "AGBusLine.h"
#include "AGBus.h"
#include "AGPosition.h"
#include "AGTime.h"

#define PAUSE_TIME 20		//time (in minutes) a bus waits before going in the opposite direction.


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
void
AGBusLine::setMaxTripTime(int time)
{
	this->maxTripTime = time;
}

void
AGBusLine::setBusNames()
{
	busNbr = 0;
	list<AGBus>::iterator it1 = buses.begin();	//iterator on buses in the first direction
	list<AGBus>::iterator it2 = revBuses.begin();	//iterator on buses in the second direction

	list<pair<int, string> > drivingBuses1, drivingBuses2;	//buses on the road or in the parking of the corresponding end: int: the time of availability

	while(it1!= buses.end() && it2!=revBuses.end())
	{
		if(it1->getDeparture() > it2->getDeparture())
		{
			if(drivingBuses2.size() == 0)
				drivingBuses2.push_front(make_pair(it2->getDeparture(), createName()));
			else if(drivingBuses2.front().first > it2->getDeparture())
				drivingBuses2.push_front(make_pair(it2->getDeparture(), createName()));
			//here the first in drivingBuses2 is available for the trip
			it2->setName(drivingBuses2.front().second);
			drivingBuses2.pop_front();
			//the same bus will be available for the main direction after some time (see function getReady):
			drivingBuses1.push_back(make_pair(getReady(it2->getDeparture()), it2->getName()));
			it2++;
		}
		else
		{
			if(drivingBuses1.size() == 0)
				drivingBuses1.push_front(make_pair(it1->getDeparture(), createName()));
			else if(drivingBuses1.front().first > it1->getDeparture())
				drivingBuses1.push_front(make_pair(it1->getDeparture(), createName()));
			//here the first in drivingBuses1 is available for the trip
			it1->setName(drivingBuses1.front().second);
			drivingBuses1.pop_front();
			//the same bus will be available for the return way after some time (see function getReady):
			drivingBuses2.push_back(make_pair(getReady(it1->getDeparture()), it1->getName()));
			it1++;
		}
	}
	if(it1 != buses.end())
	{
		if(drivingBuses1.size() == 0)
			it1->setName(createName());
		else if(drivingBuses1.front().first > it1->getDeparture())
			it1->setName(createName());
		else
		{
			it1->setName(drivingBuses1.front().second);
			drivingBuses1.pop_front();
		}
		it1++;
	}
	if(it2 != revBuses.end())
	{
		if(drivingBuses2.size() == 0)
			it2->setName(createName());
		else if(drivingBuses2.front().first > it2->getDeparture())
			it2->setName(createName());
		else
		{
			it2->setName(drivingBuses2.front().second);
			drivingBuses2.pop_front();
		}
		it2++;
	}
}

string
AGBusLine::createName()
{
	++busNbr; //initialized in setBusNames()
	std::ostringstream os;
	os << busNbr;
	return "bl" + lineNumber + "b" + os.str();
}

int
AGBusLine::getReady(int time)
{
	AGTime current(time);
	current.addSeconds(maxTripTime);
	current.addMinutes(PAUSE_TIME);
	return current.getTime();
}

int
AGBusLine::nbrBuses()
{
	return (this->buses).size();
}

void
AGBusLine::locateStation(AGPosition pos)
{
	stations.push_back(pos);
}

void
AGBusLine::generateBuses(int start, int stop, int rate)
{
	int t = start;
	AGBus* bus;
	while(t < stop)
	{
		bus = new AGBus(t);
		buses.push_back(*bus); //one direction
		bus = new AGBus(t);
		revBuses.push_back(*bus); //return direction
		t += rate;
	}
}


void
AGBusLine::printBuses()
{
	list<AGBus>::iterator it;
	cout << "\n ----------- BUS LINE " << lineNumber << " PRINTING -------------\n" << endl;
	cout << "\n -------------------------- First way ---------------------------\n" << endl;
	for(it=buses.begin() ; it!=buses.end() ; ++it)
		it->print();
	cout << "\n -------------------------- Second way --------------------------\n" << endl;
	for(it=revBuses.begin() ; it!=revBuses.end() ; ++it)
		it->print();
	cout << "\n ----------------------------------------------------------------\n" << endl;
}

/****************************************************************************/
