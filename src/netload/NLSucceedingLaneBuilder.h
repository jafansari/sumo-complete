#ifndef NLSucceedingLaneBuilder_h
#define NLSucceedingLaneBuilder_h
/***************************************************************************
                          NLSucceedingLaneBuilder.h
			  Container for the succeding lanes of a lane during their building
                             -------------------
    project              : SUMO
    begin                : Mon, 22 Oct 2001
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// $Log$
// Revision 1.11  2006/09/18 10:14:04  dkrajzew
// patching junction-internal state simulation
//
// Revision 1.10  2005/11/09 06:43:20  dkrajzew
// TLS-API: MSEdgeContinuations added
//
// Revision 1.9  2005/10/10 12:10:59  dkrajzew
// reworking the tls-API: made tls-control non-static; made net an element of traffic lights
//
// Revision 1.8  2005/10/07 11:41:49  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/15 12:04:36  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/05/04 08:43:09  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.5  2003/09/05 15:20:20  dkrajzew
// loading of internal links added
//
// Revision 1.4  2003/07/30 09:25:17  dkrajzew
// loading of directions and priorities of links implemented
//
// Revision 1.3  2003/06/05 11:52:27  dkrajzew
// class templates applied; documentation added
//
// Revision 1.2  2003/02/07 11:18:56  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:36:49  dkrajzew
// moved from ROOT/sumo/netload to ROOT/src/netload; new format definition
//  parseable in one step
//
// Revision 1.4  2002/06/11 14:39:25  dkrajzew
// windows eol removed
//
// Revision 1.3  2002/06/11 13:44:34  dkrajzew
// Windows eol removed
//
// Revision 1.2  2002/06/07 14:39:59  dkrajzew
// errors occured while building larger nets and adaption of new
//  netconverting methods debugged
//
// Revision 1.1.1.1  2002/04/08 07:21:24  traffic
// new project name
//
// Revision 2.0  2002/02/14 14:43:25  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.4  2002/02/13 15:40:46  croessel
// Merge between SourgeForgeRelease and tesseraCVS.
//
// Revision 1.1  2001/12/06 13:36:11  traffic
// moved from netbuild
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

#include <string>
#include <microsim/MSLinkCont.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class MSJunction;
class NLJunctionControlBuilder;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class NLSucceedingLaneBuilder
 * A container for the temporary storage of a lanes succeeding lanes while
 * parsing them
 */
class NLSucceedingLaneBuilder {
public:
    /// standard constructor
    NLSucceedingLaneBuilder(NLJunctionControlBuilder &jb);

    /// Destructor
    ~NLSucceedingLaneBuilder();

    /** opens the computation of a container holding the succeding lanes of
        a lane */
    void openSuccLane(const std::string &laneId);

    /// add a succeeding lane
    void addSuccLane(bool yield, const std::string &laneId,
#ifdef HAVE_INTERNAL_LANES
        const std::string &viaID, SUMOReal pass,
#endif
        MSLink::LinkDirection dir, MSLink::LinkState state,
        bool internalEnd,
        const std::string &tlid="", size_t linkNo=0);

    /// closes the building
    void closeSuccLane();

    /// returns the name of the lane the succeeding lanes are added to
    std::string getSuccingLaneName() const;

private:
    /// the id of the lane the succeeding lanes are added to
    std::string m_CurrentLane;

    /// the list of connections
    MSLinkCont   *m_SuccLanes;

    NLJunctionControlBuilder &myJunctionControlBuilder;

private:
    /** invalid copy constructor */
    NLSucceedingLaneBuilder(const NLSucceedingLaneBuilder &s);

    /** invalid assignment operator */
    NLSucceedingLaneBuilder &operator=(const NLSucceedingLaneBuilder &s);

};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
