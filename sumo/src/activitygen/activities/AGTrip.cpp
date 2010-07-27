#include "AGTrip.h"

using namespace std;

void
AGTrip::print()
{
	cout << "Trip: " << endl;
	cout << "\t-From= ";
	from.print();
	cout << "\t-To= ";
	to.print();
	cout << "\t-At= " << atTime << " -Day= " << day << endl;
	cout << "\t-Vehicle= " << vehicle << endl;
	cout << "\t-type= " << type << endl;
}

void
AGTrip::addLayOver(AGPosition by)
{
	passBy.push_back(by);
}

void
AGTrip::addLayOver(AGTrip &trip)
{
	list<AGPosition>::iterator it;
	for(it=trip.passBy.begin() ; it!=trip.passBy.end() ; ++it)
	{
		passBy.push_back(*it);
	}
	passBy.push_back(trip.to);
}

std::list<AGPosition>*
AGTrip::getPassed()
{
	return &passBy;
}

string
AGTrip::getType()
{
	return type;
}

void
AGTrip::setType(string type)
{
	this->type = type;
}

AGPosition
AGTrip::getDep()
{
	return from;
}

AGPosition
AGTrip::getArr()
{
	return to;
}

int
AGTrip::getTime()
{
	return atTime;
}

int
AGTrip::getTimeTrip(float secPerKm)
{
	float dist = 0;
	list<AGPosition> positions;
	positions.push_back(from);
	list<AGPosition>::iterator it;
	for(it=passBy.begin() ; it!=passBy.end() ; ++it)
	{
		positions.push_back(*it);
	}
	positions.push_back(to);

	bool firstPass = true;
	AGPosition *temp;
	for(it=positions.begin() ; it!=positions.end() ; ++it)
	{
		if(firstPass)
		{
			temp = &*it;
			continue;
		}
		dist += temp->getDistanceTo(*it);
		temp = &*it;
	}
	return (int)( secPerKm * (dist / 1000.0) );
}

int
AGTrip::getArrTime(float secPerKm)
{
	int arrTime = atTime + getTimeTrip(secPerKm);
	return arrTime;
}

int
AGTrip::getRideBackArrTime(float secPerKm)
{
	int arrAtTime = getArrTime(secPerKm);
	int time = (int)( secPerKm * to.getDistanceTo(from) /1000.0 );
	int arrTime = arrAtTime + time;
	return arrTime;
}

void
AGTrip::setDepTime(int time)
{
	atTime = time;
}

int
AGTrip::estimateDepTime(int arrTime, float secPerKm)
{
	int depTime = arrTime - getTimeTrip(secPerKm);
	return depTime;
}

string
AGTrip::getVehicleName()
{
	return vehicle;
}

void
AGTrip::setVehicleName(string name)
{
	vehicle = name;
}

void
AGTrip::setArr(AGPosition arrival)
{
	to = *new AGPosition(arrival.street, arrival.position);
}

void
AGTrip::setDep(AGPosition departure)
{
	from = *new AGPosition(departure.street, departure.position);
}

bool
AGTrip::isDaily()
{
	if(day == 0)
		return true;
	else
		return false;
}

int
AGTrip::getDay()
{
	return day;
}

void
AGTrip::setDay(int d)
{
	day = d;
}

