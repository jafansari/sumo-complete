/****************************************************************************/
/// @file    NBJunctionLogicCont.h
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Class for the io-ing between junctions (nodes) and the computers
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef NBJunctionLogicCont_h
#define NBJunctionLogicCont_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <map>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBJunctionLogicCont
 * This class tries to load the logic for a given logickey even when
 * key rotation must have been done
 * If this is not possible (no permutation of the logic has been build),
 * The class may be used for saving the logic build by NBRequest under the
 * given key
 */
class NBJunctionLogicCont {
public:
    NBJunctionLogicCont();
    ~NBJunctionLogicCont();

    /// saves the given junction logic bitfield
    void add(const std::string &key, const std::string &xmlDescription);

    /// destroys all stored logics
    void clear();



    /** @brief Returns the pointer to the begin of the stored logics
     * @return The iterator to the beginning of stored logics
     */
    std::map<std::string, std::string>::const_iterator begin() const {
        return myMap.begin();
    }


    /** @brief Returns the pointer to the end of the stored logics
     * @return The iterator to the end of stored logics
     */
    std::map<std::string, std::string>::const_iterator end() const {
        return myMap.end();
    }


    /** @brief Returns the number of stored logics
     * @return The number of stored logics
     */
    size_t size() const {
        return myMap.size();
    }

private:
    /// tries to load a logic with the given key (and all of his permutations)
    bool exists(const std::string &key);

private:
    /// definition o the logic-storage: a map from request to respond
    typedef std::map<std::string, std::string> LogicMap;

    /// the storage for the computed logic
    LogicMap myMap;

private:
    /** invalid copy constructor */
    NBJunctionLogicCont(const NBJunctionLogicCont &s);

    /** invalid assignment operator */
    NBJunctionLogicCont &operator=(const NBJunctionLogicCont &s);
};


#endif

/****************************************************************************/

