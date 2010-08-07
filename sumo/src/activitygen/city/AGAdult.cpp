/****************************************************************************/
/// @file    AGAdult.cpp
/// @author  Piotr Woznica
/// @date    July 2010
/// @version $Id$
///
// Person in working age: can be liked to a work position.
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
#include <vector>
#include "AGAdult.h"
#include "AGWorkPosition.h"


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
void
AGAdult::print()
{
	cout << "- AGAdult: Age=" << age << " Work=" << work << endl;
}

bool
AGAdult::assocWork(float rate, vector<AGWorkPosition>* wps, int hasStillWork)
{
	if(decision(rate) && hasStillWork>0)
	{
		AGWorkPosition* wp = pickWork(wps);
		while(!assocWork(wp))
		{
			wp = pickWork(wps);
		}
		//employed = true;
	}
	else
	{
		if(employed)
		{
			work->let();
		}
		employed = false;
		work = NULL;
	}
	return employed;
}

AGWorkPosition*
AGAdult::pickWork(vector<AGWorkPosition> *wps)
{
	//vector<AGWorkPosition>::iterator it;
	int init = rand() % (wps->size());
	//it += init;

	//int offset = 0;
	//bool positive = false;
	//while(init>=0 && init<wps->size())//it!= wps->end() && it->isTaken())
	//{
		if(!wps->at(init).isTaken())
			return &(wps->at(init));
		else
			return pickWork(wps);
		//offset++;
		//if(positive)
		//	init += offset;
		//else
		//	init -= offset;
	//}
	return pickWork(wps);
}

bool
AGAdult::assocWork(AGWorkPosition *wp)
{
	if(wp->isTaken())
	{
		return false;
	}
	else
	{
		if(employed)
		{
			work->let();
			employed = false;
		}
		work = wp;
		work->take(this);
		employed = true;
		return true;
	}
}

bool
AGAdult::isWorking()
{
	return employed;
}

void
AGAdult::loseHisJob()
{
	employed = false;
}

bool
AGAdult::quiteHisJob()
{
	if(!isWorking())
		return true;
	else
		return work->let();
}

AGPosition
AGAdult::getWorkLocation()
{
	return work->getPosition();
}

int
AGAdult::getWorkOpening()
{
	return work->getOpening();
}

int
AGAdult::getWorkClosing()
{
	return work->getClosing();
}

/****************************************************************************/
