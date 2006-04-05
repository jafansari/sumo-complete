#ifndef MapEdges_h
#define MapEdges_h
//---------------------------------------------------------------------------//
//                        MapEdges.h -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Jun 2005
//  copyright            : (C) 2005 by Danilo Boyom
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : danilo.tete-boyom@dlr.com
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
// $Log$
// Revision 1.4  2006/04/05 05:35:54  dkrajzew
// debugging
//
// Revision 1.3  2005/10/07 11:42:59  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.2  2005/09/23 06:05:18  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.1  2005/09/15 12:09:27  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.1  2005/09/09 12:53:17  dksumo
// tools added
//

/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)

/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utils/geom/Position2DVector.h>


/* =========================================================================
 * class definitions
 * ======================================================================= */

class MapEdges
{


public:
	 /// Constructor
    MapEdges();

    /// Constructor
	MapEdges(const char *netA, const char *netB );

    /// Destructor
    ~MapEdges();

	class Junction{
	public:
		explicit Junction(const std::string nid){
			id = nid;
		}
		~Junction();
		std::string id;
		Position2D pos;
	};


    /// load net-file and save the Junctions into a dictionnary 1 or 2
	void load(void);
    void loadNet(const char *net, int dic);

	/// set the junctions names
	void setJunctionA(std::string a, std::string b, std::string c);
	void setJunctionB(std::string a, std::string b, std::string c);

    // convert junction's positions in new positions
    void convertA(void);
    void convertB(void);

    void result(const std::string &file);

protected:
    /// the Name of the net-file-A
    const char *net_a;

	/// the Name of the net-file-B
    const char *net_b;

	/// Junctions name NetA
	std::string juncA1;
	std::string juncA2;
	std::string juncA3;

	/// Junction name NetB
	std::string juncB1;
	std::string juncB2;
	std::string juncB3;


	/// Definition of the dictionary type
    typedef std::map<std::string, Junction*> DictTypeJunction;

    /// The dictionaries
    static DictTypeJunction myJunctionDictA;
    static DictTypeJunction myJunctionDictB;

    static std::map<std::string, std::string> myEdge2JunctionAMap;
    static std::map<std::string, std::string> myEdge2JunctionBMap;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
