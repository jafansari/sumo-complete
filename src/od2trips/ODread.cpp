/***************************************************************************
                         Odread.cpp

	 reads OD input data from file

			 usage		 : Odread(filename, data, size, totalcars)

                             -------------------
    project              : SUMO		 :
	subproject           : OD2TRIPS
    begin                : Tue, 10 September 2002
    copyright            : (C) 2002 by DLR/IVF http://ivf.dlr.de/
    author               : Peter Mieth
    email                : Peter.Mieth@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.13  2006/04/05 05:34:08  dkrajzew
// code beautifying: embedding string in strings removed
//
// Revision 1.12  2005/09/23 06:04:23  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.11  2005/09/15 12:04:48  dkrajzew
// LARGE CODE RECHECK
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
# include <iostream>
# include <sstream>
# include <fstream>
# include <vector>
# include <cstdlib>
# include <ctime>
# include <cmath>
# include <string>
# include <utils/common/UtilExceptions.h>
# include <utils/common/MsgHandler.h>
# include "ODmatrix.h"

//std::ostream& operator<< (std::ostream& os, const OD_IN& od) {
//  os << od.from << '\t'
//     << od.to << '\t'
//     << od.how_many ;
//  return (os);
//}


/* =========================================================================
 * used namespaces
 * ======================================================================= */
 using namespace std;


/* =========================================================================
 * member method definitions
 * ======================================================================= */
int ODread (string OD_filename,vector<OD_IN>& od_in, SUMOTime *maxele,
			long *total_cars, SUMOTime *start, SUMOTime *finish, SUMOReal *factor)
	{
	std::string cLine;
	int ferror = 0;
	int n = 0;
	int maximum=0;
    int i;
	std::ifstream fsSrc (OD_filename.c_str ());
	if (!fsSrc) {
        MsgHandler::getErrorInstance()->inform("Could not open " + OD_filename + ".");
		throw ProcessError();
	}
	n = 0;
	for(i=0;i<2;i++) (getline (fsSrc, cLine)); // read dummy lines
    SUMOReal start_ini, finish_ini;
	// read begin and end and convert to seconds
	fsSrc >> start_ini >> finish_ini;
	*start = int(start_ini);
	*finish = int(finish_ini);
	SUMOReal rest = 6000 * (start_ini - *start);
	*start = (SUMOTime) (3600. * *start + rest);
	rest = 6000 * (finish_ini - *finish);
	*finish = (SUMOTime) (3600. * *finish + rest);
	for(i=0;i<2;i++) (getline (fsSrc, cLine));
	fsSrc >> *factor;
	for(i=0;i<7;i++) (getline (fsSrc, cLine)); // read dummy lines
	while (getline (fsSrc, cLine)) {
		std::istringstream isFLine (cLine);
		if(n > *maxele-1) {
            MsgHandler::getErrorInstance()->inform("resize vector!");
			throw ProcessError();
		}
        OD_IN tmp;
		if (!(isFLine >> tmp.from
		>> tmp.to
		>> tmp.how_many))
		{
            MsgHandler::getErrorInstance()->inform("Invalid data set encountered: " + cLine);
			throw ProcessError();
		}
        od_in.push_back(tmp);
		*total_cars=*total_cars+tmp.how_many;
		if(tmp.how_many > maximum) maximum=tmp.how_many;
		++n;
	}
	*maxele=n;
	fsSrc.close ();
	return (ferror);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:


