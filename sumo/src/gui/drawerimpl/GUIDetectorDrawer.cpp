//---------------------------------------------------------------------------//
//                        GUIDetectorDrawer.cpp -
//  Base class for detector drawing
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Wed, 14.Jan 2004
//  copyright            : (C) 2003 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.1  2006/12/12 12:10:37  dkrajzew
// removed simple/full geometry options; everything is now drawn using full geometry
//
// Revision 1.10  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.9  2005/10/07 11:36:48  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/22 13:30:40  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 11:05:28  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/04/27 09:44:26  dkrajzew
// level3 warnings removed
//
// Revision 1.5  2004/11/23 10:05:21  dkrajzew
// removed some warnings and adapted the new class hierarchy
//
// Revision 1.4  2004/07/02 08:12:11  dkrajzew
// detector drawers now also draw other additional items
//
// Revision 1.3  2004/03/19 12:34:30  dkrajzew
// porting to FOX
//
// Revision 1.2  2004/02/16 13:54:39  dkrajzew
// tried to patch a sometimes occuring visualisation bug
//
// Revision 1.1  2004/01/26 06:39:41  dkrajzew
// visualisation of e3-detectors added; documentation added
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

#include <utils/gui/windows/GUISUMOAbstractView.h>
#include "GUIDetectorDrawer.h"
#include <guisim/GUIDetectorWrapper.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * method definitions
 * ======================================================================= */
GUIDetectorDrawer::GUIDetectorDrawer(
        const std::vector<GUIGlObject_AbstractAdd*> &additionals)
    : myAdditionals(additionals)
{
}


GUIDetectorDrawer::~GUIDetectorDrawer()
{
}


void
GUIDetectorDrawer::setGLID(bool val)
{
    myShowToolTips = val;
}


void
GUIDetectorDrawer::drawGLDetectors(size_t *which,
                                       size_t maxDetectors,
                                       SUMOReal scale,
                                       SUMOReal upscale)
{
    initStep();
    for(size_t i=0; i<maxDetectors; i++ ) {
        if(which[i]==0) {
            continue;
        }
        size_t pos = 1;
        for(size_t j=0; j<32; j++, pos<<=1) {
            if((which[i]&pos)!=0) {
                if(myShowToolTips) {
                    glPushName(myAdditionals[j+(i<<5)]->getGlID());
                }
                myAdditionals[j+(i<<5)]->drawGL(scale, upscale);
                if(myShowToolTips) {
                    glPopName();
                }
            }
        }
    }
}


void
GUIDetectorDrawer::initStep()
{
    glLineWidth(1);
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
