/****************************************************************************/
/// @file    GNEApplicationWindow.h
/// @author  Daniel Krajzewicz
/// @date    22. Nov 2004
/// @version $Id: $
///
// Class for the main gui window
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
#ifndef GNEApplicationWindow_h
#define GNEApplicationWindow_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <vector>
#include <iostream>
#include <fx.h>
#include <utils/foxtools/MFXEventQue.h>
#include <utils/foxtools/FXThreadEvent.h>
#include <utils/foxtools/MFXInterThreadEventClient.h>
#include <utils/foxtools/FXRealSpinDial.h>
#include <utils/foxtools/FXLCDLabel.h>
#include <utils/helpers/ValueRetriever.h>
#include <utils/helpers/ValueSource.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <gui/GUISUMOViewParent.h>
#include "Image.h"
#include "Graph.h"
#include "ConfigDialog.h"
#include "InfoDialog.h"
#include <netbuild/NBNetBuilder.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUILoadThread;
class GUIRunThread;
class GUIMessageWindow;
class GUIEvent;
class GUIParameterTracker;
class GUIParameterTableWindow;
class GUIThreadFactory;
class Image;


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class GNEApplicationWindow
 * The main window of the SUMO-gui. Contains the file opening support and
 * a canvas to display the simulation representations in.
 *
 * Beside views on the simulation, showed within a MDI-window, the main window
 * may also have some further views (children) assigned which are stored
 * within a separate list.
 *
 * As the aggregated visualisation is quite memory consuming and needs also
 * some cpu-time, it may be swichted of on gui-startup. The information
 * whether aggregated views are allowed is stored within this class, too.
 */
class GNEApplicationWindow :
            public GUIMainWindow, public MFXInterThreadEventClient
{
    // FOX-declarations
    FXDECLARE(GNEApplicationWindow)
public:

    /** constructor */
    GNEApplicationWindow(FXApp* a, GUIThreadFactory &threadFactory,
                         int glWidth, int glHeight, const std::string &config);

    /** destructor */
    virtual ~GNEApplicationWindow();

    /** @brief Creates the main window
        (required by FOX) */
    void create();

    /// Detaches the tool/menu bar
    void detach();

    void loadSelection(const std::string &file) const
    {
        throw 1;
    } // !!! to be implemented

public:
    /// Closes the log window
    void showLog();

public:
    void eventOccured();
    void handleEvent_SimulationLoaded(GUIEvent *e);
    void handleEvent_SimulationStep(GUIEvent *e);
    void handleEvent_Message(GUIEvent *e);
    void handleEvent_SimulationEnded(GUIEvent *e);


public:
    /** @brief Called by FOX if a simulation shall be opened
        Called either by FileMenu->Open or the Toolbar Open-Button */
    long onCmdOpen(FXObject*,FXSelector,void*);
    long onCmdReload(FXObject*,FXSelector,void*);
    long onCmdOpenRecent(FXObject*,FXSelector,void*);
    long onCmdImportNet(FXObject*,FXSelector,void*);
    long onCmdClose(FXObject*,FXSelector,void*);
    long onCmdLoadImage(FXObject*,FXSelector,void*);
    long onCmdSaveImage(FXObject*,FXSelector,void*);
    long onCmdSaveEdgesNodes(FXObject*,FXSelector,void*);
    long onCmdSaveNet(FXObject*,FXSelector,void*);

    long onUpdOpen(FXObject*,FXSelector,void*);
    long onUpdReload(FXObject*,FXSelector,void*);
    long onUpdOpenRecent(FXObject*,FXSelector,void*);
    long onUpdSaveImage(FXObject*,FXSelector,void*);
    long onUpdSaveEdgesNodes(FXObject*,FXSelector,void*);
    long onUpdSaveNet(FXObject*,FXSelector,void*);

    /** @brief Called by FOX if the application shall be closed
        Called either by FileMenu->Quit, the normal close-menu or SIGINT  */
    long onCmdQuit(FXObject*,FXSelector,void*);

    long onCmdEditChosen(FXObject*,FXSelector,void*);
    virtual long onCmdEditAddWeights(FXObject*,FXSelector,void*);
    long onCmdEditBreakpoints(FXObject*,FXSelector,void*);


    /// Opens the application settings menu
    long onCmdAppSettings(FXObject*,FXSelector,void*);

    /// Opens the simulation settings menu
    long onCmdSimSettings(FXObject*,FXSelector,void*);

    /// Shows the about dialog
    long onCmdAbout(FXObject*,FXSelector,void*);

    long onCmdStart(FXObject*,FXSelector,void*);
    long onCmdStop(FXObject*,FXSelector,void*);
    long onCmdStep(FXObject*,FXSelector,void*);

    long onCmdNewMicro(FXObject*,FXSelector,void*);

    long onUpdAddMicro(FXObject*,FXSelector,void*);
    long onUpdStart(FXObject*,FXSelector,void*);
    long onUpdStop(FXObject*,FXSelector,void*);
    long onUpdStep(FXObject*,FXSelector,void*);
    long onUpdEditChosen(FXObject*sender,FXSelector,void*ptr);
    virtual long onUpdEditAddWeights(FXObject*,FXSelector,void*);
    virtual long onUpdEditBreakpoints(FXObject*,FXSelector,void*);
    long onUpdSimSettings(FXObject*sender,FXSelector,void*ptr);
    long onCmdClearMsgWindow(FXObject*,FXSelector,void*);

    long onLoadThreadEvent(FXObject*, FXSelector, void*);
    long onRunThreadEvent(FXObject*, FXSelector, void*);
    /*
        long onPaint(FXObject*,FXSelector,void*);
        long onMouseDown(FXObject*,FXSelector,void*);
        long onMouseUp(FXObject*,FXSelector,void*);
        long onMouseMove(FXObject*,FXSelector,void*);
        */
    long onCmdClear(FXObject*,FXSelector,void*);
    long onUpdClear(FXObject*,FXSelector,void*);

    long onUpdPictureMenu(FXObject*,FXSelector,void*);
    long onCmdPictureMenu(FXObject*,FXSelector,void*);
    long onUpdGraphMenu(FXObject*,FXSelector,void*);
    long onCmdGraphMenu(FXObject*,FXSelector,void*);

    FXGLCanvas *getBuildGLCanvas() const;
    size_t getCurrentSimTime() const;

    FXCursor *getDefaultCursor();


private:
    /** starts to load a simulation */
    void load(const std::string &file);

    /** this method closes all windows and deletes the current simulation */
    void closeAllWindows();

    /** opens a new simulation display */
    void openNewView(GUISUMOViewParent::ViewType type);

protected:
    /// FOX needs this for static members
    GNEApplicationWindow()
    { }
    /** called when an event occures */
//    bool event(QEvent *e);

private:
    /// Builds the menu bar
    void fillMenuBar();

    /// Builds the tool bar
    void buildToolBars();

protected:
    /** the name of the simulation */
    std::string _name;

    /** the thread that loads simulations */
    GUILoadThread *myLoadThread;

    /** the thread that runs simulations */
    GUIRunThread *myRunThread;

    /** the information whether the simulation was started before */
    bool _wasStarted;

    /// The current view number
    size_t myViewNumber;

    /// information whether the gui is currently loading and the load-options shall be greyed out
    bool myAmLoading;

    /// the submenus
    FXMenuPane *myFileMenu, /**mySubFileMenu1,*/ *myEditMenu, *myImageMenu, *myGraphMenu,
    *mySettingsMenu, *myWindowsMenu, *myHelpMenu;

    /// A window to display messages, warnings and error in
    GUIMessageWindow *myMessageWindow;

    /// The splitter that divides the main window into vies and the log window
    FXSplitter *myMainSplitter;

    /// for some menu detaching fun
    FXToolBarShell *myToolBarDrag1, *myToolBarDrag2, *myToolBarDrag3,
    *myToolBarDrag4, *myToolBarDrag5,
    *myMenuBarDrag/*, *myIMGToolBarDrag*/;

    ///
    FXRealSpinDial *mySimDelayTarget;

    /// The simulation delay
    FXdouble mySimDelay;

    /// List of got requests
    MFXEventQue myEvents;

    /// The menu used for the MDI-windows
    FXMDIMenu *myMDIMenu;

    /// The application menu bar
    FXMenuBar *myMenuBar;

    /// The application tool bar
    FXToolBar *myToolBar1, *myToolBar2, *myToolBar3, *myToolBar4, *myToolBar5;

    // The Image Toolbar
//    FXToolBar *myIMGToolBar;

    /// The gl-visual used
    FXGLVisual *myGLVisual;

    /// the simulation step display
    FXEX::FXLCDLabel *myLCDLabel;

    /// io-event with the load-thread
    FXEX::FXThreadEvent myLoadThreadEvent;

    /// io-event with the run-thread
    FXEX::FXThreadEvent myRunThreadEvent;

    /// Font used for popup-menu titles
    FXFont *myBoldFont;

    /// List of recent files
    FXRecentFiles myRecentFiles;

    //Scrollwindow for the canvas
//    FXScrollWindow *scrollBox;

//    FXHorizontalFrame *hoFrame;

    // Drawing Canvas
//    FXCanvas *myCanvas;

    // Bitmap Member variable
    //Image *m_img;

    // Bitmap Member variable
    //Graph graph;

    // Image Configuration Dialog
    ConfigDialog *dialog;
    InfoDialog *dialog2;

    // default filename for the imagefiledialog
    FXString imgfilename;

    //Popupbutton for painttool
//	FXPopup* paintpop;

    //Popupbutton for painttool
//	FXPopup* rubberpop;

    //flag for Streetextraction..Algorithm may only be used once
//	bool extrFlag;

    NBNetBuilder myNetBuilder;

};


#endif

/****************************************************************************/

