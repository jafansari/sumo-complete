/****************************************************************************/
/// @file    GUIAbstractLoadThread.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// Class describing the thread that performs the loading of a simulation
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
#ifndef GUIAbstractLoadThread_h
#define GUIAbstractLoadThread_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <vector>
#include <fx.h>
#include <FXThread.h>
#include <utils/foxtools/FXSingleEventThread.h>
#include <utils/foxtools/FXThreadEvent.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUIMainWindow;
class MsgRetriever;
class MFXEventQue;
class GUIEdgeControlBuilder;
class NLEdgeControlBuilder;
class NLJunctionControlBuilder;
class GUINetBuilder;
class OptionsCont;
class GUIVehicleControl;
class GUINet;


// ===========================================================================
// class definitions
// ===========================================================================
class GUIAbstractLoadThread : public FXSingleEventThread
{
public:
    /// constructor
    GUIAbstractLoadThread(MFXInterThreadEventClient *mw, MFXEventQue &eq,
                          FXEX::FXThreadEvent &ev);

    /// destructor
    virtual ~GUIAbstractLoadThread();

    /// begins the loading of the given file
    virtual void load(const std::string &file, bool isNet);

    /** starts the thread
        the thread ends after the net has been loaded */
    virtual FXint run() = 0;

    /// Retrieves messages from the loading module
    void retrieveMessage(const std::string &msg);

    /// Retrieves warnings from the loading module
    void retrieveWarning(const std::string &msg);

    /// Retrieves error from the loading module
    void retrieveError(const std::string &msg);

    const std::string &getFileName() const;

protected:
    /// the parent window to inform about the loading
    MFXInterThreadEventClient *myParent;

    /// the path to load the simulation from
    std::string myFile;

    /** @brief The instances of message retriever encapsulations
        Needed to be deleted from the handler later on */
    MsgRetriever *myErrorRetriever, *myMessageRetriever, *myWarningRetreiver;

    MFXEventQue &myEventQue;

    FXEX::FXThreadEvent &myEventThrow;

    /// Information whether only the network shall be loaded
    bool myLoadNet;

};


#endif

/****************************************************************************/

