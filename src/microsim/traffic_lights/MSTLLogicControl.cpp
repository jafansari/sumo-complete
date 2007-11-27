/****************************************************************************/
/// @file    MSTLLogicControl.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// A class that stores and controls tls and switching of their programs
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
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#pragma warning(disable: 4503)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <algorithm>
#include <cassert>
#include "MSTrafficLightLogic.h"
#include "MSSimpleTrafficLightLogic.h"
#include "MSTLLogicControl.h"
#include "MSOffTrafficLightLogic.h"
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * method definitions for the Switching Procedures
 * ----------------------------------------------------------------------- */
SUMOReal
MSTLLogicControl::WAUTSwitchProcedure::getGSPValue(MSTrafficLightLogic *from) const
{
    string val = from->getParameterValue("GSP");
    if (val.length()==0) {
        return -1;
    }
    return TplConvert<char>::_2SUMOReal(val.c_str());
}


bool
MSTLLogicControl::WAUTSwitchProcedure::isPosAtGSP(SUMOTime step, MSSimpleTrafficLightLogic *testLogic)
{
    MSSimpleTrafficLightLogic *givenLogic = (MSSimpleTrafficLightLogic*) testLogic;
    size_t CycleTime = givenLogic->getCycleTime();
    SUMOReal gspFrom = getGSPValue(givenLogic);
    ///get the position of the given signalprogramm at the actual simulationsecond
    size_t posFrom = givenLogic -> getPosition(step);

    if (gspFrom == CycleTime)	{
        gspFrom = 0;
    }
    ///compare the position of the given programm with the GSP (GSP = "G�nstigerSchaltPunkt")
    if (gspFrom == posFrom) {
        return true;
    } else {
        return false;
    }
}


size_t
MSTLLogicControl::WAUTSwitchProcedure::getDiffToStartOfPhase(MSSimpleTrafficLightLogic *givenLogic, size_t pos)
{
    MSSimpleTrafficLightLogic *myLogic = givenLogic;
    size_t myPos = pos;
    size_t stepOfMyPos = myLogic->getStepFromPos(myPos);
    size_t startOfPhase = myLogic->getPosFromStep(stepOfMyPos);
    MSPhaseDefinition myPhase = myLogic->getPhaseFromStep(stepOfMyPos);
    size_t durOfPhase = myPhase.duration;

    assert(myPos >= startOfPhase);
    size_t diff = myPos - startOfPhase;
    assert(diff <= durOfPhase);
    return diff;
}


void
MSTLLogicControl::WAUTSwitchProcedure::switchToPos(SUMOTime simStep, MSSimpleTrafficLightLogic *givenLogic, size_t pos)
{
    MSSimpleTrafficLightLogic *myLogic = givenLogic;
    size_t posTo = pos;
    size_t stepTo = myLogic->getStepFromPos(posTo);
    size_t diff = getDiffToStartOfPhase(myLogic, posTo);
    MSPhaseDefinition myPhase = myLogic->getPhaseFromStep(stepTo);
    size_t dur = myPhase.duration - diff;
    myLogic->changeStepAndDuration(myControl ,simStep, stepTo, dur);
}



MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::WAUTSwitchProcedure_JustSwitch(
    MSTLLogicControl &control, WAUT &waut,
    MSTrafficLightLogic *from, MSTrafficLightLogic *to, bool synchron)
        : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron)
{}


MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::~WAUTSwitchProcedure_JustSwitch()
{}


bool
MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::trySwitch(SUMOTime /*step*/)
{
    return true;
}






MSTLLogicControl::WAUTSwitchProcedure_GSP::WAUTSwitchProcedure_GSP(
    MSTLLogicControl &control, WAUT &waut,
    MSTrafficLightLogic *from, MSTrafficLightLogic *to, bool synchron)
        : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron)
{}


MSTLLogicControl::WAUTSwitchProcedure_GSP::~WAUTSwitchProcedure_GSP()
{}


bool
MSTLLogicControl::WAUTSwitchProcedure_GSP::trySwitch(SUMOTime step)
{
    MSSimpleTrafficLightLogic *LogicFrom = (MSSimpleTrafficLightLogic*) myFrom;
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    SUMOReal posTo = 0;
    ///switch to the next programm if the GSP is reached
    if (isPosAtGSP(step, LogicFrom)==true) {
        posTo = getGSPValue(myTo);
        if (mySwitchSynchron) {
            adaptLogic(step);
        } else {
            switchToPos(step, LogicTo, (size_t) posTo);
        }
        return true;
    }
    return false;
}


void
MSTLLogicControl::WAUTSwitchProcedure_GSP::adaptLogic(SUMOTime step)
{
    SUMOTime simStep = step;
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    SUMOReal gspTo = getGSPValue(myTo);
    size_t stepTo = LogicTo->getStepFromPos((size_t) gspTo);
    size_t cycleTimeTo = LogicTo->getCycleTime();
    // gets the actual position from the myToLogic
    size_t actPosTo = LogicTo->getPosition(simStep);
    size_t deltaToStretch= 0;
    if (gspTo == cycleTimeTo) {
        gspTo=0;
    }
    size_t diff = getDiffToStartOfPhase(LogicTo, (size_t) gspTo);
    if (gspTo >= actPosTo) {
        deltaToStretch = (size_t)(gspTo - actPosTo);
    } else {
        deltaToStretch = (size_t)(cycleTimeTo - actPosTo + gspTo);
    }
    size_t newdur = LogicTo->getPhaseFromStep(stepTo).duration - diff + deltaToStretch;
    LogicTo->changeStepAndDuration(myControl, simStep, stepTo, newdur);
}










MSTLLogicControl::WAUTSwitchProcedure_Stretch::WAUTSwitchProcedure_Stretch(
    MSTLLogicControl &control, WAUT &waut,
    MSTrafficLightLogic *from, MSTrafficLightLogic *to, bool synchron)
        : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron)
{}


MSTLLogicControl::WAUTSwitchProcedure_Stretch::~WAUTSwitchProcedure_Stretch()
{}


bool
MSTLLogicControl::WAUTSwitchProcedure_Stretch::trySwitch(SUMOTime step)
{
    MSSimpleTrafficLightLogic *LogicFrom = (MSSimpleTrafficLightLogic*) myFrom;
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    SUMOReal posTo = 0;
    ///switch to the next programm if the GSP is reached
    if (isPosAtGSP(step, LogicFrom)==true) {
        posTo = getGSPValue(myTo);
        if (mySwitchSynchron) {
            adaptLogic(step, posTo);
        } else {
            switchToPos(step, LogicTo, (size_t) posTo);
        }
        return true;
    }
    return false;
}


void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::adaptLogic(SUMOTime step, SUMOReal position)
{
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    size_t cycleTime = LogicTo->getCycleTime();
    // the position, in which the logic has to be switched
    size_t startPos = (size_t) position;
    // this is the position, where the Logic have to be after synchronisation
    size_t posAfterSyn = LogicTo->getPosition(step);

    // switch the toLogic to the startPosition
    // fehlt!!!!
    // erfolgt in cutLogic und/oder stretchLogic!


    // calculate the difference, that has to be equalized
    size_t deltaToCut = 0;
    if (posAfterSyn < startPos) {
        deltaToCut = posAfterSyn + cycleTime - startPos;
    } else deltaToCut =  posAfterSyn - startPos;
    // test, wheter cutting of the Signalplan is possible
    size_t deltaPossible = 0;
    int noBereiche = getStretchBereicheNo(myTo);
    for (int i=0; i<noBereiche; i++) {
        StretchBereichDef def = getStretchBereichDef(myTo, i+1);
        assert(def.end >= def.begin) ;
        deltaPossible = deltaPossible + (size_t)(def.end - def.begin);
    }
    int stretchUmlaufAnz = (int) TplConvert<char>::_2SUMOReal(LogicTo->getParameterValue("StretchUmlaufAnz").c_str());
    deltaPossible = stretchUmlaufAnz * deltaPossible;
    if ((deltaPossible > deltaToCut)&&(deltaToCut < (cycleTime / 2))) {
        cutLogic(step, startPos, deltaToCut);
    } else {
        size_t deltaToStretch = cycleTime - deltaToCut;
        if (deltaToStretch == cycleTime) {
            deltaToStretch = 0;
        }
        stretchLogic(step, startPos, deltaToStretch);
    }

}


void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::cutLogic(SUMOTime step, size_t pos, size_t deltaToCut)
{
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    size_t startPos = pos;
    size_t actStep = LogicTo->getStepFromPos(startPos);
    size_t allCutTime = deltaToCut;
    // switches to startPos and cuts this phase, if there is a "Bereich"
    int noBereiche = getStretchBereicheNo(myTo);
    size_t toCut = 0;
    for (int i=0; i<noBereiche; i++) {
        StretchBereichDef def = getStretchBereichDef(myTo, i+1);
        size_t begin = (size_t) def.begin;
        size_t end = (size_t) def.end;
        size_t stepOfBegin = LogicTo->getStepFromPos(begin);
        if (stepOfBegin == actStep)	{
            if (begin < startPos) {
                toCut = end - startPos;
            } else {
                toCut = end - begin;
            }
            if (allCutTime < toCut) {
                toCut = allCutTime;
            }
            allCutTime = allCutTime - toCut;
        }
    }
    size_t remainingDur = LogicTo->getPhaseFromStep(actStep).duration - getDiffToStartOfPhase(LogicTo, startPos);
    size_t newDur = remainingDur - toCut;
    myTo->changeStepAndDuration(myControl,step,actStep,newDur);

    // changes the duration of all other phases
    int currStep = actStep + 1;
    if (currStep == (int) LogicTo->getPhases().size()) {
        currStep = 0;
    }
    while (allCutTime > 0) {
        for (int i=currStep; i<(int) LogicTo->getPhases().size(); i++) {
            size_t beginOfPhase = LogicTo->getPosFromStep(i);
            size_t durOfPhase = LogicTo->getPhaseFromStep(i).duration;
            size_t endOfPhase = beginOfPhase + durOfPhase;
            for (int i=0; i<noBereiche; i++) {
                StretchBereichDef def = getStretchBereichDef(myTo, i+1);
                size_t begin = (size_t) def.begin;
                size_t end = (size_t) def.end;
                if ((beginOfPhase <= begin) && (endOfPhase >= end)) {
                    size_t maxCutOfPhase = end - begin;
                    if (allCutTime< maxCutOfPhase) {
                        maxCutOfPhase = allCutTime;
                    }
                    allCutTime = allCutTime - maxCutOfPhase;
                    durOfPhase = durOfPhase - maxCutOfPhase;
                }
            }
            LogicTo->addOverridingDuration(durOfPhase);
        }
        currStep = 0;
    }
}

void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::stretchLogic(SUMOTime step, size_t startPos, size_t deltaToStretch)
{
    MSSimpleTrafficLightLogic *LogicTo = (MSSimpleTrafficLightLogic*) myTo;
    size_t currPos = startPos;
    size_t currStep = LogicTo->getStepFromPos(currPos);
    size_t durOfPhase = LogicTo->getPhaseFromStep(currStep).duration;
    size_t allStretchTime = deltaToStretch;
    size_t remainingStretchTime = allStretchTime;
    int StretchTimeOfPhase = 0;
    size_t stretchUmlaufAnz = (size_t) TplConvert<char>::_2SUMOReal(LogicTo->getParameterValue("StretchUmlaufAnz").c_str());
    float facSum = 0;
    int noBereiche = getStretchBereicheNo(myTo);
    int x;
    for (x=0; x<noBereiche; x++) {
        StretchBereichDef def = getStretchBereichDef(myTo, x+1);
        size_t fac = (size_t) def.fac;
        facSum = facSum + fac;
    }
    facSum = facSum * stretchUmlaufAnz;


    //switch to startPos and stretch this phase, if there is a end of "bereich" between startpos and end of phase
    size_t diffToStart = getDiffToStartOfPhase(LogicTo, currPos);
    for (x=0; x<noBereiche; x++) {
        StretchBereichDef def = getStretchBereichDef(myTo, x+1);
        size_t end = (size_t) def.end;
        size_t endOfPhase = (size_t)(currPos + durOfPhase - diffToStart);
        if (end <= endOfPhase && end >= currPos) {
            float fac = def.fac;
            float actualfac = fac / facSum;
            facSum = facSum - fac;
            StretchTimeOfPhase = (int)((float)remainingStretchTime * actualfac + 0.5);
            remainingStretchTime = allStretchTime - StretchTimeOfPhase;
        }
    }
    durOfPhase = durOfPhase - diffToStart + StretchTimeOfPhase;
    myTo->changeStepAndDuration(myControl,step,currStep,durOfPhase);

    currStep ++;
    if (currStep >= LogicTo->getPhases().size()) {
        currStep = 0;
    }

    // stretch all other phases, if there is a "bereich"
    while (remainingStretchTime > 0) {
        for (int i=currStep; i<(int) LogicTo->getPhases().size() && remainingStretchTime > 0; i++) {
            durOfPhase = LogicTo->getPhaseFromStep(i).duration;
            size_t beginOfPhase = LogicTo->getPosFromStep(i);
            size_t endOfPhase = beginOfPhase + durOfPhase;
            for (int j=0; j<noBereiche && remainingStretchTime > 0; j++) {
                StretchBereichDef def = getStretchBereichDef(myTo, j+1);
                size_t end = (size_t) def.end;
                SUMOReal fac = def.fac;
                if ((beginOfPhase <= end) && (endOfPhase >= end)) {
                    float actualfac = fac / facSum;
                    StretchTimeOfPhase = (int)((float)remainingStretchTime * actualfac + 0.5) ;
                    facSum = facSum - fac;
                    durOfPhase = durOfPhase + StretchTimeOfPhase;
                    remainingStretchTime = remainingStretchTime - StretchTimeOfPhase;
                }
            }
            LogicTo->addOverridingDuration(durOfPhase);
        }
        currStep = 0;
    }
}

int
MSTLLogicControl::WAUTSwitchProcedure_Stretch::getStretchBereicheNo(MSTrafficLightLogic *from) const
{
    int no = 0;
    while (from->getParameterValue("B" + toString(no+1) + ".begin")!="") {
        no++;
    }
    return no;
}


MSTLLogicControl::WAUTSwitchProcedure_Stretch::StretchBereichDef
MSTLLogicControl::WAUTSwitchProcedure_Stretch::getStretchBereichDef(MSTrafficLightLogic *from, int index) const
{
    StretchBereichDef def;
    def.begin = TplConvert<char>::_2SUMOReal(from->getParameterValue("B" + toString(index) + ".begin").c_str());
    def.end = TplConvert<char>::_2SUMOReal(from->getParameterValue("B" + toString(index) + ".end").c_str());
    def.fac = TplConvert<char>::_2SUMOReal(from->getParameterValue("B" + toString(index) + ".factor").c_str());
    return def;
}


/* -------------------------------------------------------------------------
 * method definitions for MSTLLogicControl
 * ----------------------------------------------------------------------- */
MSTLLogicControl::MSTLLogicControl()
        : myNetWasLoaded(false)
{}


MSTLLogicControl::~MSTLLogicControl()
{
    // delete logics
    for (std::map<std::string, TLSLogicVariants>::const_iterator i=myLogics.begin(); i!=myLogics.end(); ++i) {
        const TLSLogicVariants &vars = (*i).second;
        std::map<std::string, MSTrafficLightLogic *>::const_iterator j;
        for (j=vars.ltVariants.begin(); j!=vars.ltVariants.end(); ++j) {
            delete(*j).second;
        }
    }
    // delete WAUTs
    for (std::map<std::string, WAUT*>::const_iterator i=myWAUTs.begin(); i!=myWAUTs.end(); ++i) {
        delete(*i).second;
    }
}


void
MSTLLogicControl::maskRedLinks()
{
    for_each(myActiveLogics.begin(), myActiveLogics.end(), mem_fun(&MSTrafficLightLogic::maskRedLinks));
}


void
MSTLLogicControl::maskYellowLinks()
{
    for_each(myActiveLogics.begin(), myActiveLogics.end(), mem_fun(&MSTrafficLightLogic::maskYellowLinks));
}


std::vector<MSTrafficLightLogic*>
MSTLLogicControl::getAllLogics() const
{
    std::vector<MSTrafficLightLogic*> ret;
    std::map<std::string, TLSLogicVariants>::const_iterator i;
    for (i=myLogics.begin(); i!=myLogics.end(); ++i) {
        const TLSLogicVariants &vars = (*i).second;
        std::map<std::string, MSTrafficLightLogic *>::const_iterator j;
        for (j=vars.ltVariants.begin(); j!=vars.ltVariants.end(); ++j) {
            ret.push_back((*j).second);
        }
    }
    return ret;
}

const MSTLLogicControl::TLSLogicVariants &
MSTLLogicControl::get(const std::string &id) const throw(InvalidArgument)
{
    std::map<std::string, TLSLogicVariants>::const_iterator i = myLogics.find(id);
    if (i==myLogics.end()) {
        throw InvalidArgument("The tls '" + id + "' is not known.");
    }
    return (*i).second;
}


MSTrafficLightLogic * const
MSTLLogicControl::get(const std::string &id, const std::string &subid) const
{
    std::map<std::string, TLSLogicVariants>::const_iterator i = myLogics.find(id);
    if (i==myLogics.end()) {
        return 0;
    }
    const std::map<std::string, MSTrafficLightLogic *> &vars = (*i).second.ltVariants;
    std::map<std::string, MSTrafficLightLogic *>::const_iterator j = vars.find(subid);
    if (j==vars.end()) {
        return 0;
    }
    return (*j).second;
}


std::vector<std::string>
MSTLLogicControl::getAllTLIds() const
{
	std::vector<std::string> ret;

	std::map<std::string, TLSLogicVariants>::const_iterator i;
	for (i=myLogics.begin(); i!=myLogics.end(); ++i) {
		ret.push_back((*i).first);
	}

	return ret;
}


bool
MSTLLogicControl::add(const std::string &id, const std::string &subID,
                      MSTrafficLightLogic *logic, bool newDefault) throw(ProcessError)
{
    if (myLogics.find(id)==myLogics.end()) {
        TLSLogicVariants var;
        var.defaultTL = 0;
        myLogics[id] = var;
    }
    std::map<std::string, TLSLogicVariants>::iterator i = myLogics.find(id);
    TLSLogicVariants &tlmap = (*i).second;
    if (tlmap.ltVariants.find(subID)!=tlmap.ltVariants.end()) {
        return false;
    }
    // assert the links are set
    if (myNetWasLoaded) {
        // this one has not yet its links set
        if (tlmap.defaultTL==0) {
            throw ProcessError("No initial signal plan loaded for tls '" + id + "'.");
        }
        logic->adaptLinkInformationFrom(*(tlmap.defaultTL));
    }
    // add to the list of active
    if (tlmap.ltVariants.size()==0) {
        tlmap.defaultTL = logic;
        myActiveLogics.push_back(logic);
    } else if (newDefault) {
        std::vector<MSTrafficLightLogic*>::iterator i =
            find(myActiveLogics.begin(), myActiveLogics.end(), tlmap.defaultTL);
        assert(i!=myActiveLogics.end());
        *i = logic;
        tlmap.defaultTL = logic;
    }
    // add to the list of logic
    tlmap.ltVariants[subID] = logic;
    return true;
}


bool
MSTLLogicControl::knows(const std::string &id) const
{
    std::map<std::string, TLSLogicVariants>::const_iterator i = myLogics.find(id);
    if (i==myLogics.end()) {
        return false;
    }
    return true;
}


void
MSTLLogicControl::closeNetworkReading()
{
    for (map<std::string, TLSLogicVariants>::iterator i=myLogics.begin(); i!=myLogics.end(); ++i) {
        (*i).second.originalLinkStates = (*i).second.defaultTL->collectLinkStates();
    }
    myNetWasLoaded = true;
}


bool
MSTLLogicControl::isActive(const MSTrafficLightLogic *tl) const
{
    std::map<std::string, TLSLogicVariants>::const_iterator i = myLogics.find(tl->getID());
    if (i==myLogics.end()) {
        return false;
    }
    return (*i).second.defaultTL == tl;
}


MSTrafficLightLogic * const
MSTLLogicControl::getActive(const std::string &id) const
{
    std::map<std::string, TLSLogicVariants>::const_iterator i = myLogics.find(id);
    if (i==myLogics.end()) {
        return 0;
    }
    return (*i).second.defaultTL;
}


bool
MSTLLogicControl::switchTo(const std::string &id, const std::string &subid)
{
    // try to get the tls program definitions
    std::map<std::string, TLSLogicVariants>::iterator i = myLogics.find(id);
    // handle problems
    if (i==myLogics.end()) {
        throw ProcessError("Could not switch tls '" + id + "' to program '" + subid + "':\n No such tls exists.");
    }
    if ((*i).second.ltVariants.find(subid)==(*i).second.ltVariants.end()) {
        // we'll inform the user about the missing tls program
        throw ProcessError("Could not switch tls '" + id + "' to program '" + subid + "':\n No such program exists.");
    }
    // try to get the program to switch to
    MSTrafficLightLogic *touse = (*i).second.ltVariants[subid];
    // ... and the current program
    std::vector<MSTrafficLightLogic*>::iterator j =
        find(myActiveLogics.begin(), myActiveLogics.end(), (*i).second.defaultTL);
    // handle problems
    if (j==myActiveLogics.end()) {
        throw ProcessError("Could not switch tls '" + id + "' to program '" + subid + "':\n No such tls exists.");
    }
    // switch to the wished program
    // replace the current sub-program by the found wished in myActiveLogics
    *j = touse;
    // set the found wished sub-program as this tls' current one
    (*i).second.defaultTL = touse;
    // in the case we have switched to an off-state, we'll reset the links
    if (subid=="off") {
        touse->resetLinkStates((*i).second.originalLinkStates);
    }
    return true;
}


void
MSTLLogicControl::addWAUT(SUMOTime refTime, const std::string &id,
                          const std::string &startProg)
{
    // check whether the waut was already defined
    if (myWAUTs.find(id)!=myWAUTs.end()) {
        // report an error if so
        throw InvalidArgument("Waut '" + id + "' was already defined.");
    }
    WAUT *w = new WAUT;
    w->id = id;
    w->refTime = refTime;
    w->startProg = startProg;
    myWAUTs[id] = w;
}


void
MSTLLogicControl::addWAUTSwitch(const std::string &wautid,
                                SUMOTime when, const std::string &to)
{
    // try to get the waut
    if (myWAUTs.find(wautid)==myWAUTs.end()) {
        // report an error if the waut is not known
        throw InvalidArgument("Waut '" + wautid + "' was not yet defined.");
    }
    // build and save the waut switch definition
    WAUTSwitch s;
    s.to = to;
    s.when = when;
    myWAUTs[wautid]->switches.push_back(s);
    // activate it if it's the first one
    if (myWAUTs[wautid]->switches.size()==1) {
        SUMOTime begin = when;
        // check reference time
        if (myWAUTs[wautid]->refTime<=begin) {
            // add the reference time if it's within the same day
            begin += myWAUTs[wautid]->refTime;
        }
        if (myWAUTs[wautid]->refTime>begin) {
            // make a day period
            begin = begin - (86400-myWAUTs[wautid]->refTime);
        }
        begin = (86400 + begin);
        while (begin>86400) {
            begin -= 86400;
        }
        // activate
        MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(
            new SwitchInitCommand(*this, wautid),
            begin, MSEventControl::NO_CHANGE);
    }
}


void
MSTLLogicControl::addWAUTJunction(const std::string &wautid,
                                  const std::string &junc,
                                  const std::string &proc,
                                  bool synchron)
{
    // try to get the waut
    if (myWAUTs.find(wautid)==myWAUTs.end()) {
        // report an error if the waut is not known
        throw InvalidArgument("Waut '" + wautid + "' was not yet defined.");
    }
    // try to get the tls to switch
    if (myLogics.find(junc)==myLogics.end()) {
        // report an error if the tls is not known
        throw InvalidArgument("TLS '" + junc + "' to switch in WAUT '" + wautid + "' was not yet defined.");
    }
    WAUTJunction j;
    j.junction = junc;
    j.procedure = proc;
    j.synchron = synchron;
    myWAUTs[wautid]->junctions.push_back(j);
    // set the current program
    TLSLogicVariants &vars = myLogics.find(junc)->second;
    switchTo(vars.defaultTL->getID(), myWAUTs[wautid]->startProg);
}


SUMOTime
MSTLLogicControl::initWautSwitch(MSTLLogicControl::SwitchInitCommand &cmd)
{
    const std::string &wautid = cmd.getWAUTID();
    int &index = cmd.getIndex();
    WAUTSwitch s = myWAUTs[wautid]->switches[index];
    for (std::vector<WAUTJunction>::iterator i=myWAUTs[wautid]->junctions.begin(); i!=myWAUTs[wautid]->junctions.end(); ++i) {

        // get the current program and the one to instantiate
        TLSLogicVariants &vars = myLogics.find((*i).junction)->second;
        MSTrafficLightLogic *from = vars.defaultTL;
        if (vars.ltVariants.find(s.to)==vars.ltVariants.end()) {
            if (s.to=="off") {
                // build an off-tll if this switch indicates it
                if (!add((*i).junction, "off", new MSOffTrafficLightLogic(*this, (*i).junction), false)) {
                    // inform the user if this fails
                    throw ProcessError("Could not build an off-state for tls '" + (*i).junction + "'.");
                }
            } else {
                // inform the user about a missing logic
                throw ProcessError("Can not switch tls '" + (*i).junction + "' to program '" + s.to + "';\n The program is not known.");
            }
        }
        MSTrafficLightLogic *to = vars.ltVariants.find(s.to)->second;

        WAUTSwitchProcedure *proc = 0;
        if ((*i).procedure=="GSP") {
            proc = new WAUTSwitchProcedure_GSP(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        } else if ((*i).procedure=="Stretch") {
            proc = new WAUTSwitchProcedure_Stretch(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        } else {
            proc = new WAUTSwitchProcedure_JustSwitch(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        }

        WAUTSwitchProcess p;
        p.junction = (*i).junction;
        p.proc = proc;
        p.from = from;
        p.to = to;

        myCurrentlySwitched.push_back(p);
    }
    index++;
    if (index==(int) myWAUTs[wautid]->switches.size()) {
        return 0;
    }
    return myWAUTs[wautid]->switches[index].when - myWAUTs[wautid]->switches[index-1].when;
}


void
MSTLLogicControl::check2Switch(SUMOTime step)
{
    for (std::vector<WAUTSwitchProcess>::iterator i=myCurrentlySwitched.begin(); i!=myCurrentlySwitched.end();) {
        const WAUTSwitchProcess &proc = *i;
        if (proc.proc->trySwitch(step)) {
            delete proc.proc;
            switchTo((*i).to->getID(), (*i).to->getSubID());
            i = myCurrentlySwitched.erase(i);
        } else {
            ++i;
        }
    }
}


std::pair<SUMOTime, MSPhaseDefinition>
MSTLLogicControl::getPhaseDef(const std::string &tlid) const
{
    MSTrafficLightLogic *tl = getActive(tlid);
    return make_pair(
               MSNet::getInstance()->getCurrentTimeStep(),
               tl->getCurrentPhaseDef());
}



/****************************************************************************/

