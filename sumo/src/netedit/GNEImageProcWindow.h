/****************************************************************************/
/// @file    GNEImageProcWindow.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 26 Jan 2005
/// @version $Id$
///
// missing_desc
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
#ifndef GNEImageProcWindow_h
#define GNEImageProcWindow_h
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

#include <fx.h>
#include <utils/gui/windows/GUIGlChildWindow.h>
#include "Image.h"
#include "InfoDialog.h"
#include "ConfigDialog.h"
#include "Graph.h"
#include <utils/common/MsgHandler.h>
#include <netbuild/NBNetBuilder.h>

#include "GNEApplicationWindow.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEImageProcWindow
 * This MDIChild allows to display and manipulate a loaded map
 */
class GNEImageProcWindow : public GUIGlChildWindow
{
    FXDECLARE(GNEImageProcWindow)
public:
    GNEImageProcWindow(GNEApplicationWindow *parent, NBNetBuilder *nb,
                       FXMDIClient* p, FXMDIMenu *mdimenu,
                       Image *img, bool extrFlag, const FXString& name,
                       FXIcon* ic=NULL, FXPopup* pup=NULL,FXuint opts=0,
                       FXint x=0,FXint y=0,FXint w=0,FXint h=0
                      );

    virtual ~GNEImageProcWindow();

    void create();

    virtual FXGLCanvas *getBuildGLCanvas() const
    {
        return 0;
    }

    virtual bool showLegend() const
    {
        return false;
    }
    virtual bool allowRotation() const
    {
        return false;
    }

    /// Extracts the bitmaps� pixels which are determined as road area
    long onCmdExtractStreets(FXObject*,FXSelector,void*);

    /// Erodes the Bitmap (only 0/1-Bitmap)
    long onCmdErode(FXObject*,FXSelector,void*);

    /// Dilates the Bitmap (only 0/1-Bitmap)
    long onCmdDilate(FXObject*,FXSelector,void*);

    /// Creates an morphological opening (dilatation followed by erosion) of the Image
    /// (only 0/1-Bitmap)
    long onCmdOpening(FXObject*,FXSelector,void*);

    /// Creates an morphological closing (erosion followed by dilatation) of the Image
    /// (only 0/1-Bitmap)
    long onCmdClosing(FXObject*,FXSelector,void*);

    /// minimizes small white areas in a black surronding (only 0/1-Bitmap)
    long onCmdCloseGaps(FXObject*,FXSelector,void*);

    /// Filters small black Spots in the Image (only 0/1-Bitmap)
    long onCmdEraseStains(FXObject*,FXSelector,void*);

    /// Skeletonizes the image (only 0/1-Bitmap)
    long onCmdSkeletonize(FXObject*,FXSelector,void*);

    /// Creates a graph from the �street skeleton�
    long onCmdCreateGraph(FXObject*,FXSelector,void*);

    //Opens a Dialog Window for Image-Filter Configuration
    long onCmdShowBMPDialog(FXObject*,FXSelector,void*);


    /// Opens the Show Graph on empty Bitmap Dialog
    long onCmdShowGraphOnEmptyBitmap(FXObject*,FXSelector,void*);

    /// Opens the Show Graph on actial Bitmap Dialog
    long onCmdShowGraphOnActualBitmap(FXObject*,FXSelector,void*);

    /// Opens the Reduce Vertices Dialog
    long onCmdReduceVertexes(FXObject*,FXSelector,void*);

    /// Opens the Reduce Vertices Plus Dialog
    long onCmdReduceVertexesPlus(FXObject*,FXSelector,void*);

    /// Opens the Reduce Edges Dialog
    long onCmdReduceEdges(FXObject*,FXSelector,void*);

    /// Opens the Merge Vertices Dialog
    long onCmdMergeVertexes(FXObject*,FXSelector,void*);

    /// Opens the Export Vertices XML Dialog
    long onCmdExportVertexesXML(FXObject*,FXSelector,void*);

    /// Opens the Export Edges XML Dialog
    long onCmdExportEdgesXML(FXObject*,FXSelector,void*);


    long onPaint(FXObject*,FXSelector,void*);
    long onMouseDown(FXObject*,FXSelector,void*);
    long onMouseUp(FXObject*,FXSelector,void*);
    long onMouseMove(FXObject*,FXSelector,void*);



    void retrieveMessage(const std::string &msg);
    void retrieveWarning(const std::string &msg);
    void retrieveError(const std::string &msg);

protected:
    Image *m_img;
    FXImageView *myImageViewer;
    FXToolBar *myIMGToolBar;
    //Popupbutton for painttool
    FXPopup* myPaintPop;

    //Popupbutton for painttool
    FXPopup* myRubberPop;

    bool myGraphFlag, mySkelFlag;

    ///flag for Streetextraction..Algorithm may only be used once
    bool myExtrFlag;

    InfoDialog *myInfoDialog;
    ConfigDialog *myConfigDialog;
    Graph myGraph;

    int                mdflag;                  // Mouse button down?
    int                dirty;                   // Canvas has been painted?
    FXColor            drawColor;               // Color for the line
    int                m_mergeTol;

    /** @brief The instances of message retriever encapsulations
        Needed to be deleted from the handler later on */
    MsgRetriever *myErrorRetriever, *myMessageRetriever, *myWarningRetreiver;

    GNEApplicationWindow *myParent;

    NBNetBuilder *myNetBuilder;


protected:
    GNEImageProcWindow()
    { }
};


#endif

/****************************************************************************/

