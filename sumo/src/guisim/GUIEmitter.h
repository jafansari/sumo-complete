#ifndef GUIEmitter_h
#define GUIEmitter_h
//---------------------------------------------------------------------------//
//                        GUIEmitter.h -
//  The gui-version of MSEmitter
//                           -------------------
//  begin                : Thu, 21.07.2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
//  author               : Daniel Krajzewicz
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
// Revision 1.4  2006/02/27 12:01:42  dkrajzew
// routes visualization for simple geometry added
//
// Revision 1.3  2006/02/13 07:52:43  dkrajzew
// debugging
//
// Revision 1.2  2006/01/09 11:50:21  dkrajzew
// new visualization settings implemented
//
// Revision 1.1  2005/11/09 06:35:03  dkrajzew
// Emitters reworked
//
// Revision 1.4  2005/10/06 13:39:12  dksumo
// using of a configuration file rechecked
//
// Revision 1.3  2005/09/20 06:10:40  dksumo
// floats and doubles replaced by SUMOReal; warnings removed
//
// Revision 1.2  2005/09/09 12:50:30  dksumo
// complete code rework: debug_new and config added
//
// Revision 1.1  2005/08/01 13:06:51  dksumo
// further triggers added
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

#include <vector>
#include <string>
#include <utils/helpers/Command.h>
#include <microsim/trigger/MSEmitter.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/geom/HaveBoundary.h>
#include <utils/geom/Position2D.h>
#include <gui/GUIManipulator.h>
#include <utils/foxtools/FXRealSpinDial.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSNet;
class MSLane;
class GUIManipulator;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUIEmitter
 * This is the gui-version of the MSTriggeredSource-object
 */
class GUIEmitter
    : public MSEmitter,
    public GUIGlObject_AbstractAdd {
public:
    /** constructor */
    GUIEmitter(const std::string &id, MSNet &net,
        MSLane* destLanes, SUMOReal pos,
        const std::string &aXMLFilename);

    /** destructor */
    ~GUIEmitter();

    //@{ From GUIGlObject
    /// Returns an own popup-menu
    GUIGLObjectPopupMenu *getPopUpMenu(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// Returns an own parameter window
    GUIParameterTableWindow *getParameterWindow(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    /// returns the id of the object as known to microsim
    std::string microsimID() const;

    /// Returns the information whether this object is still active
    bool active() const;
    //@}

    //@{ From GUIAbstractAddGlObject
    /// Draws the detector in full-geometry mode
    void drawGL_FG(SUMOReal scale, SUMOReal upscale);

    /// Draws the detector in simple-geometry mode
    void drawGL_SG(SUMOReal scale, SUMOReal upscale);

    /// Returns the detector's coordinates
    Position2D getPosition() const;
    //@}

    Boundary getBoundary() const;

    GUIManipulator *openManipulator(GUIMainWindow &app,
        GUISUMOAbstractView &parent);

    void setUserFlow(SUMOReal factor);
    SUMOReal getUserFlow() const;

    void setActiveChild(int index);

	void toggleDrawRoutes();

protected:
    class GUIEmitterChild_UserTriggeredChild
        : public MSEmitter::MSEmitterChild, public Command {
    public:
        GUIEmitterChild_UserTriggeredChild(
            MSEmitter_FileTriggeredChild &s, MSEmitter &parent,
            SUMOReal flow);
        virtual ~GUIEmitterChild_UserTriggeredChild();
        SUMOTime execute();
        SUMOReal getUserFlow() const;

    protected:
        SUMOReal myUserFlow;
        MSVehicle *myVehicle;
        MSEmitter_FileTriggeredChild &mySource;
        bool myDescheduleVehicle;

    };

public:
    class GUIEmitterPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUIEmitterPopupMenu)
    public:

        GUIEmitterPopupMenu(GUIMainWindow &app,
            GUISUMOAbstractView &parent, GUIGlObject &o);

        ~GUIEmitterPopupMenu();

        /** @brief Called if the object's manipulator shall be shown */
        long onCmdOpenManip(FXObject*,FXSelector,void*);

        long onCmdDrawRoute(FXObject*,FXSelector,void*);

    protected:
        GUIEmitterPopupMenu() { }

    };

    class GUIManip_TriggeredEmitter : public GUIManipulator {
        FXDECLARE(GUIManip_TriggeredEmitter)
    public:
        enum {
            MID_USER_DEF = FXDialogBox::ID_LAST,
            MID_PRE_DEF,
            MID_OPTION,
            MID_CLOSE,
            ID_LAST
        };
        /// Constructor
        GUIManip_TriggeredEmitter(GUIMainWindow &app,
            const std::string &name, GUIEmitter &o,
            int xpos, int ypos);

        /// Destructor
        virtual ~GUIManip_TriggeredEmitter();

        long onCmdOverride(FXObject*,FXSelector,void*);
        long onCmdClose(FXObject*,FXSelector,void*);
        long onCmdUserDef(FXObject*,FXSelector,void*);
        long onUpdUserDef(FXObject*,FXSelector,void*);
        long onCmdChangeOption(FXObject*,FXSelector,void*);

    private:
        GUIMainWindow *myParent;

        FXRealSpinDial *myFlowFactorDial;

        FXint myChosenValue;

        FXDataTarget myChosenTarget;

        FXDataTarget myFlowFactorTarget;

        SUMOReal myFlowFactor;

        GUIEmitter *myObject;

    protected:
        GUIManip_TriggeredEmitter() { }

    };

private:
    void doPaint(const Position2D &pos, SUMOReal rot, SUMOReal scale, SUMOReal upscale);
    std::map<const MSEdge*, SUMOReal> getEdgeProbs() const;

private:
    /// The positions in full-geometry mode
    Position2D myFGPosition;

    /// The rotations in full-geometry mode
    SUMOReal myFGRotation;

    /// The positions in simple-geometry mode
    Position2D mySGPosition;

    /// The rotations in simple-geometry mode
    SUMOReal mySGRotation;

    /// The information whether the speed shall be shown in m/s or km/h
    //bool myShowAsKMH;

    /// Storage for last value to avoid string recomputation
    //SUMOReal myLastValue;

    /// Storage for speed string to avoid recomputation
    //std::string myLastValueString;

    mutable SUMOReal myUserFlow;

    MSEmitterChild *myUserEmitChild;

	bool myDrawRoutes;

};

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

