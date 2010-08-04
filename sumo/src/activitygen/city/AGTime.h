#ifndef AGTIME_H
#define AGTIME_H

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>


// ===========================================================================
// class definitions
// ===========================================================================
class AGTime
{
public:
	AGTime() {};
	AGTime(int seconds) :
		sec(seconds)
	{};
	AGTime(int hour, int minutes) :
		sec(convert(0, hour, minutes, 0))
	{};
	AGTime(int day, int hour, int min) :
		sec(convert(day, hour, min, 0))
	{};
	AGTime(const AGTime& time);
	bool operator==(const AGTime &time);
	bool operator<(const AGTime &time);
	bool operator<=(const AGTime &time);
	void operator+=(const AGTime &time);
	void operator-=(const AGTime &time);
	AGTime operator+(const AGTime &time);

	/********************
	 * In/Out functions *
	 ********************/
	int getDay();
	int getHour();
	int getMinute();
	int getSecond();
	/**
	 * @brief: returns the number of seconds from the beginning of the first day of simulation
	 * this includes
	 */
	int getTime();

	void setDay(int d);
	void setHour(int h);
	void setMinute(int m);
	void setSecond(int s);
	/**
	 * @brief: sets the time from the beginning of the first day of simulation in seconds
	 */
	void setTime(int sec);


	/**************************
	 * Manipulation functions *
	 **************************/
	/**
	 * @brief addition of seconds to the current moment
	 *
	 * @param[in] sec the number of seconds
	 */
	void addSeconds(int sec);

	/**
	 * @brief addition of minutes to the current moment
	 *
	 * @param[in] min the number of minutes
	 */
	void addMinutes(int min);

	/**
	 * @brief addition of hours to the current moment
	 *
	 * @param[in] hours the number of hours to add
	 */
	void addHours(int hours);

	/**
	 * @brief addition of days to the current moment
	 *
	 * @param[in] days the number of days to add
	 */
	void addDays(int days);

	/**
	 * @brief computes the number of seconds in the given minutes
	 *
	 * @param[in] minutes, can be fraction of minutes
	 *
	 * @return number of seconds
	 */
	int getSecondsOf(float minutes);

private:
	/**
	 * @brief converts days, hours and minutes to seconds
	 */
	int convert(int days, int hours, int minutes, int seconds);


	// @brief: the seconds representing this date (day, hour, minute)
	// @brief: used for in/out
	int sec;
};

#endif

/****************************************************************************/
