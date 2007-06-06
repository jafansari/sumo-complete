/****************************************************************************/
/// @file    Intersection.h
/// @author  unknown_author
/// @date    Jun 2005
/// @version $Id: $
///
//
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
#ifndef Intersection_h
#define Intersection_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utils/geom/Position2DVector.h>


// ===========================================================================
// class definitions
// ===========================================================================

class Intersection
{


public:
    /// Constructor
    Intersection();

    /// Constructor
    Intersection(const char *net, const char *polygons);

    /// Destructor
    ~Intersection();

    class Lane
    {
    public:
        explicit Lane(const std::string nid)
        {
            id = nid;
        }
        ~Lane();
        Position2DVector posi;
        std::string id;
        int length;
    };
    class Polygon
    {
    public:
        explicit Polygon(const std::string nid)
        {
            id = nid;
        }
        ~Polygon();
        Position2DVector posi;
        std::string id;
    };

    /// load net-file and save the Position intos a dictionnary
    void loadNet();

    /// load polygon-file file and save the Positions into a dictionnary
    void loadPolygon();

    // compare all value to find the intersection point
    // write results into a file
    void compare(const char *output);

    // gibt wie oft ein char in ein string vorkommt
    int getNumberOf(std::string str);

public:

    /// the Name of the net-file
    const char *net;

    /// the type of the polygon-file
    const char *polygons;

    /// Definition of the dictionary type
    typedef std::map<std::string, Lane*> DictTypeLane;
    typedef std::map<std::string, Polygon*> DictTypePolygon;

    /// The dictionary
    static DictTypeLane myLaneDict;
    static DictTypePolygon myPolyDict;

};


#endif

/****************************************************************************/

