/****************************************************************************/
/// @file    GUIColorer_ByDeviceNumber.h
/// @author  Daniel Krajzewicz
/// @date    Mon, 2. Jan 2006
/// @version $Id$
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
#ifndef GUIColorer_ByDeviceNumber_h
#define GUIColorer_ByDeviceNumber_h


// ===========================================================================
// included modules
// ===========================================================================

#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "GUIBaseColorer.h"
#include <utils/gfx/RGBColor.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

template<class _T, class _D>
class GUIColorer_ByDeviceNumber : public GUIBaseColorer<_T>
{
public:
    /// Type of the function to execute.
    typedef bool(_T::* HasOperation)(_D) const;
    typedef SUMOReal(_T::* NumberOperation)(_D) const;

    GUIColorer_ByDeviceNumber(HasOperation hasOperation,
                              NumberOperation numberOperation,
                              bool catchNo, SUMOReal min, SUMOReal max,
                              const RGBColor &no, const RGBColor &minColor, const RGBColor &maxColor,
                              _D param)
            : myHasOperation(hasOperation), myNumberOperation(numberOperation),
            myNoColor(no), myMinColor(minColor), myMaxColor(maxColor),
            myMin(min), myMax(max), myParameter(param)
    {
        myScale = (SUMOReal) 1.0 / (myMax-myMin);
    }

    virtual ~GUIColorer_ByDeviceNumber()
    { }

    void setGlColor(const _T& i) const
    {
        if (!(i.*myHasOperation)((_D) myParameter)) {
            if (myCatchNo) {
                glColor3d(myNoColor.red(), myNoColor.green(), myNoColor.blue());
            } else {
                glColor3d(myMinColor.red(), myMinColor.green(), myMinColor.blue());
            }
        } else {
            SUMOReal val = (i.*myNumberOperation)((_D) myParameter) - myMin;
            if (val<myMin) {
                val = myMin;
            } else if (val>myMax) {
                val = myMax;
            }
            val = val * myScale;
            RGBColor c =
                (myMinColor * ((SUMOReal) 1.0 - val)) + (myMaxColor * val);
            glColor3d(c.red(), c.green(), c.blue());
        }
    }

    void setGlColor(SUMOReal val) const
    {
        glColor3d(1, 1, 0);
    }

    virtual ColorSetType getSetType() const
    {
        return CST_STATIC;
    }

    virtual void resetColor(const RGBColor &min)
    {}

    virtual const RGBColor &getSingleColor() const
    {
        throw 1;
    }

protected:
    HasOperation myHasOperation;
    NumberOperation myNumberOperation;
    bool myCatchNo;
    RGBColor myNoColor, myMinColor, myMaxColor;
    SUMOReal myMin, myMax, myScale;
    _D myParameter;

};


#endif

/****************************************************************************/

