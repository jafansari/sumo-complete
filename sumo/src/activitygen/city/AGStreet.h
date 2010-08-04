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

#include <iostream>
#include <string>
#include "../../router/RONet.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGStreet
{
public:
	AGStreet(const std::string edg, RONet* net, int popD = 0, int workD = 0) :
	  edgeName(edg),
		  net(net),
		  popDensity(popD),
		  workDensity(workD)
	  {
		computeLength();
	  };
	void print();
	float getLength();
	std::string getName();
	int getPopDensity();
	int getWorkDensity();
	void setPopulation(int pop);
	void setWorkPositions(int work);

public:
	RONet* net;

private:
	void computeLength();

	std::string edgeName;
	int popDensity;
	int workDensity;
	float length;
};

#endif

/****************************************************************************/
