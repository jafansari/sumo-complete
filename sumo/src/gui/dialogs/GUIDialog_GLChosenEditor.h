#ifndef GUIDialog_GLChosenEditor_h
#define GUIDialog_GLChosenEditor_h
//---------------------------------------------------------------------------//
//                        GUIDialog_GLChosenEditor.h -
//  Editor for the  list of chosen objects
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Thu, 11.03.2004
//  copyright            : (C) 2004 by Daniel Krajzewicz
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
// $Log$
// Revision 1.3  2004/11/23 10:00:08  dkrajzew
// new class hierarchy for windows applied
//
// Revision 1.2  2004/04/02 10:57:31  dkrajzew
// deselection of selected items added; saving of selected items names added
//
// Revision 1.1  2004/03/19 12:33:36  dkrajzew
// porting to FOX
//
// Revision 1.1  2004/03/19 12:32:26  dkrajzew
// porting to FOX
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <string>
#include <vector>
#include <fx.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIApplicationWindow;


/* =========================================================================
 * class definition
 * ======================================================================= */
/**
 * @class GUIDialog_GLChosenEditor
 * Instances of this class are windows that display the list of instances
 * from a given artifact like vehicles, edges or junctions and allow
 * one of their items
 */
class GUIDialog_GLChosenEditor : public FXMainWindow
{
    // FOX-declarations
    FXDECLARE(GUIDialog_GLChosenEditor)
public:
    /// constructor
    GUIDialog_GLChosenEditor(GUIMainWindow *parent);

    /// destructor
    ~GUIDialog_GLChosenEditor();

    FXbool close(FXbool notify=FALSE);

    long onCmdLoad(FXObject*,FXSelector,void*);
    long onCmdSave(FXObject*,FXSelector,void*);
    long onCmdDeselect(FXObject*,FXSelector,void*);
    long onCmdClear(FXObject*,FXSelector,void*);
    long onCmdClose(FXObject*,FXSelector,void*);

private:
    void rebuildList();

private:
    /// the list that holds the ids
    FXList *myList;

    /// the parent window
    GUIMainWindow *myParent;

protected:
    GUIDialog_GLChosenEditor() { }

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

