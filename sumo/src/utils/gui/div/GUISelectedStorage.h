#ifndef GUISelectedStorage_h
#define GUISelectedStorage_h
//---------------------------------------------------------------------------//
//                        GUISelectedStorage.h -
//  Storage for object selections
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Jun 2004
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
// $Log$
// Revision 1.5  2006/11/22 13:07:50  dkrajzew
// debugging right-click on shapes
//
// Revision 1.4  2005/10/07 11:44:53  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.3  2005/09/15 12:18:59  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.2  2004/12/16 12:12:59  dkrajzew
// first steps towards loading of selections between different applications
//
// Revision 1.1  2004/11/23 10:38:29  dkrajzew
// debugging
//
// Revision 1.1  2004/10/22 12:50:47  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.1  2004/07/02 08:08:33  dkrajzew
// global object selection added
//
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

#include <vector>
#include <string>
#include <fstream>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIDialog_GLChosenEditor;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUISelectedStorage
 */
class GUISelectedStorage {
public:
    /// Constructor
    GUISelectedStorage();

    /// Destructor
    ~GUISelectedStorage();

    /// Returns the information whether the object with the given type and id is selected
    bool isSelected(int type, size_t id);

    /// Selects the object with the given type and id
    void select(int type, size_t id, bool update=true);

    /// Toggles selection of an object
    void addObjectChecking(size_t id, long withShift);

    /// Deselects the object with the given type and id
    void deselect(int type, size_t id);

    /// Returns the list of all selected objects' ids
    const std::vector<size_t> &getAllSelected() const;

    /// Returns the list of ids of all selected objects' of a certain type
    const std::vector<size_t> &getSelected(int type) const;

    /// Clears the list of selected objects
    void clear();

    /// Loads a selection list
    void load(int type, const std::string &filename);

    /// Saves a selection list
    void save(int type, const std::string &filename);

    /// Adds a selected-dialog to be updated
    void add2Update(GUIDialog_GLChosenEditor *ed);

    /// Removes a selected-dialog to be updated
    void remove2Update(GUIDialog_GLChosenEditor *ed);

    /**
     * @class SingleTypeSelections
     */
    class SingleTypeSelections {
    public:
        /// Constructor
        SingleTypeSelections();

        /// Destrcutor
        ~SingleTypeSelections();

        /// returns the information whether the object with the given id is qithin the selection
        bool isSelected(size_t id);

        /// Adds the object with the given id to the list of selected objects
        void select(size_t id);

        /// Deselects the object with the given id from the list of selected objects
        void deselect(size_t id);

        /// Clears the list of selected objects
        void clear();

        /// Loads the list of selected objects to a file named as given
        void load(const std::string &filename);

        /// Saves the list of selected objects to a file named as given
        void save(const std::string &filename);

        /// Saves the list to the given stream
        void save(std::ofstream &strm);

        /// Returns the list of selected ids
        const std::vector<size_t> &getSelected() const;

    private:
        /// The list of selected ids
        std::vector<size_t> mySelected;

    };

private:
    /// List of selected vehicles
    SingleTypeSelections mySelectedVehicles;

    /// List of selected lanes
    SingleTypeSelections mySelectedLanes;

    /// List of selected edges
    SingleTypeSelections mySelectedEdges;

    /// List of selected junctions
    SingleTypeSelections mySelectedJunctions;

    /// List of selected emitters
    SingleTypeSelections mySelectedEmitters;

    /// List of selected detectors
    SingleTypeSelections mySelectedDetectors;

    /// List of selected tl-logics
    SingleTypeSelections mySelectedTLLogics;

    /// List of selected triggers
    SingleTypeSelections mySelectedTriggers;

    /// List of selected shapes
    SingleTypeSelections mySelectedShapes;

    /// List of selected objects
    std::vector<size_t> mySelected;

    /// The dialog to be updated
    GUIDialog_GLChosenEditor *my2Update;

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

