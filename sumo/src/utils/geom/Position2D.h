/****************************************************************************/
/// @file    Position2D.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A position in a 2D-world
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
#ifndef Position2D_h
#define Position2D_h


// ===========================================================================
// included modules
// ===========================================================================
#include <iostream>
#include <cmath>

#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class Position2D
{
public:
    /// default constructor
    Position2D() : myX(0.0), myY(0.0) { }

    /// parametrised constructor
    Position2D(SUMOReal x, SUMOReal y)
            : myX(x), myY(y) { }

    /// Destructor
    ~Position2D() { }

    /// Returns the x-position
    SUMOReal x() const {
        return myX;
    }

    /// Returns the y-position
    SUMOReal y() const {
        return myY;
    }

    ///
    void set(SUMOReal x, SUMOReal y) {
        myX = x;
        myY = y;
    }

    ///
    void set(const Position2D &pos) {
        myX = pos.myX;
        myY = pos.myY;
    }


    /// Multiplies both positions with the given value
    void mul(SUMOReal val) {
        myX *= val;
        myY *= val;
    }

    /// Multiplies position with the given values
    void mul(SUMOReal mx, SUMOReal my) {
        myX *= mx;
        myY *= my;
    }

    /// Adds the given position to this one
    void add(const Position2D &pos) {
        myX += pos.myX;
        myY += pos.myY;
    }

    /// Adds the given position to this one
    void add(SUMOReal dx, SUMOReal dy) {
        myX += dx;
        myY += dy;
    }

    /// Substracts the given position from this one
    void sub(SUMOReal dx, SUMOReal dy) {
        myX -= dx;
        myY -= dy;
    }

    /// Substracts the given position from this one
    void sub(const Position2D &pos) {
        myX -= pos.myX;
        myY -= pos.myY;
    }

    SUMOReal scalar() const {
        return sqrt(myX*myX + myY*myY);
    }

    void norm() {
        SUMOReal val = scalar();
        myX = myX / val;
        myY = myY / val;
    }

    void reshiftRotate(SUMOReal xoff, SUMOReal yoff, SUMOReal rot) {
        SUMOReal x = myX * cos(rot) -myY * sin(rot) + xoff;
        SUMOReal y =myX * sin(rot) + yoff + myY * cos(rot);
        myX = x;
        myY = y;
    }


    /// Prints to the output
    friend std::ostream &operator<<(std::ostream &os, const Position2D &p) {
        os << p.x() << "," << p.y();
        return os;
    }

    friend bool operator==(const Position2D &p1, const Position2D &p2) {
        return p1.x()==p2.x() && p1.y()==p2.y();
    }

    friend bool operator!=(const Position2D &p1, const Position2D &p2) {
        return p1.x()!=p2.x() || p1.y()!=p2.y();
    }


    bool almostSame(const Position2D &p2, SUMOReal maxDiv=POSITION_EPS) const {
        return fabs(myX-p2.myX)<maxDiv && fabs(myY-p2.myY)<maxDiv;
    }

    float euclidDistance(const float& x, const float& y) {
        return sqrtf((myX-x)*(myX-x) + (myY-y)*(myY-y));
    }


private:
    /// The x-position
    SUMOReal myX;

    /// The y-position
    SUMOReal myY;

};


#endif

/****************************************************************************/

