#ifndef NBLoadedTLDef_h
#define NBLoadedTLDef_h
//---------------------------------------------------------------------------//
//                        NBLoadedTLDef.h -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Fri, 29.04.2005
//  copyright            : (C) 2005 by Daniel Krajzewicz
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
// Revision 1.8  2005/09/15 12:02:45  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.7  2005/07/12 12:32:47  dkrajzew
// code style adapted; guessing of ramps and unregulated near districts implemented; debugging
//
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <vector>
#include <string>
#include <set>
#include "NBTrafficLightDefinition.h"
#include "nodes/NBNode.h"
#include <utils/common/SUMOTime.h>


/* =========================================================================
 * class definitions
 * ======================================================================= */
class NBLoadedTLDef : public NBTrafficLightDefinition {
public:
    class SignalGroup : public Named {
    public:
        SignalGroup(const std::string &id);
        ~SignalGroup();
        void addConnection(const NBConnection &c);
        void addPhaseBegin(double time, TLColor color);
        void setYellowTimes(double tRedYellowe, double tYellow);
        DoubleVector getTimes(double cycleDuration) const;
        void sortPhases();
        size_t getLinkNo() const;
        bool mayDrive(double time) const;
        bool hasYellow(double time) const;
//        bool mustBrake(double time) const;
        bool containsConnection(NBEdge *from, NBEdge *to) const;
        bool containsIncoming(NBEdge *from) const;
        void remapIncoming(NBEdge *which, const EdgeVector &by);
        bool containsOutgoing(NBEdge *to) const;
        void remapOutgoing(NBEdge *which, const EdgeVector &by);
        const NBConnection &getConnection(size_t pos) const;
        void patchTYellow(size_t tyellow);
//        void patchFalseGreenPhases(double duration);
        void remap(NBEdge *removed, int removedLane,
            NBEdge *by, int byLane);

        friend class phase_by_time_sorter;

    private:
        class PhaseDef {
        public:
            PhaseDef(double time, TLColor color)
                : myTime(time), myColor(color) { }
            PhaseDef(const PhaseDef &p)
                : myTime(p.myTime), myColor(p.myColor) { }
            double myTime;
            TLColor myColor;
        };

        class phase_by_time_sorter {
        public:
            /// constructor
            explicit phase_by_time_sorter() { }

            int operator() (const PhaseDef &p1, const PhaseDef &p2) {
                return p1.myTime<p2.myTime;
            }
        };

        NBConnectionVector myConnections;
        typedef std::vector<PhaseDef> GroupsPhases;
        GroupsPhases myPhases;
        double myTRedYellow, myTYellow;
    };

    class Phase
        : public Named {
    public:
        Phase(const std::string &id, SUMOTime begin, SUMOTime end);
        ~Phase();
    private:
        std::string mySignalGroup;
        SUMOTime myBegin, myEnd;
        typedef std::map<std::string, TLColor> SignalGroupColorMap;
        SignalGroupColorMap _groupColors;
    };

    /// Definition of the container for signal groups
    typedef std::map<std::string, SignalGroup*> SignalGroupCont;

    /// Constructor
    NBLoadedTLDef(const std::string &id,
        const std::set<NBNode*> &junctions);

    /// Constructor
    NBLoadedTLDef(const std::string &id, NBNode *junction);

    /// Constructor
    NBLoadedTLDef(const std::string &id);

    /// Destructor
    ~NBLoadedTLDef();

    /// Returns the signal group which is responsible for the given connection
    SignalGroup *findGroup(NBEdge *from, NBEdge *to) const;

    /** @brief Sets the duration of a cycle
        valid only for loaded traffic lights */
    void setCycleDuration(size_t cycleDur);

    /// Adds a signal group
    void addSignalGroup(const std::string &id);

    /** @brief Adds a signal to a signal group
        The signal is described by the connection it is placed at */
    bool addToSignalGroup(const std::string &groupid,
        const NBConnection &connection);

    /** @brief Adds a list of signals to a signal group
        Each signal is described by the connection it is placed at */
    bool addToSignalGroup(const std::string &groupid,
        const NBConnectionVector &connections);

    /** @brief Sets the information about the begin of a phase
        Valid for loaded traffic lights only */
    void addSignalGroupPhaseBegin(const std::string &groupid,
        double time, TLColor color);

    /// Sets the times the light is yellow or red/yellow
    void setSignalYellowTimes(const std::string &groupid,
        double tRedYellowe, double tYellow);

    void setTLControllingInformation(const NBEdgeCont &ec) const;

public:
    void remapRemoved(NBEdge *removed,
        const EdgeVector &incoming, const EdgeVector &outgoing);

protected:
    /// Computes the traffic light logic
    NBTrafficLightLogicVector *myCompute(const NBEdgeCont &ec,
        size_t breakingTime, std::string type, bool buildAll);

    /// Collects the nodes participating in this traffic light
    void collectNodes();

    void collectLinks();

    bool mustBrake(const NBEdgeCont &ec,
        const NBConnection &possProhibited,
        const std::bitset<64> &green, const std::bitset<64> &yellow,
        size_t strmpos) const;

    void replaceRemoved(NBEdge *removed, int removedLane,
        NBEdge *by, int byLane);

private:
    struct Masks {
        std::bitset<64> driveMask;
        std::bitset<64> brakeMask;
        std::bitset<64> yellowMask;
    };

private:

    Masks buildPhaseMasks(const NBEdgeCont &ec, size_t time) const;

private:
    SignalGroupCont mySignalGroups;

    size_t myCycleDuration;

};

/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:
