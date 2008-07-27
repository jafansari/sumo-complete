#!/usr/bin/env python
"""
@file    vehicleControl.py
@author  Michael.Behrisch@dlr.de
@date    2008-07-21
@version $Id$

Control the CityMobil parking lot via TraCI.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""
import subprocess, numpy
from optparse import OptionParser

from constants import *
from traciControl import initTraCI, simStep, stopObject, changeTarget, close

class Status:

    def __init__(self, edge):
        self.edge = edge
        self.parking = False
        self.load = 0

def reroute(occupancy, vehicleID):
    slotEdge = ""
    for rowIdx, row in enumerate(occupancy):
        for idx, slot in enumerate(row):
            if slot == -1:
                row[idx] = vehicleID
                dir = "l"
                if rowIdx % 2 == 1:
                    dir = "r"
                slotEdge = "slot%s-%s%s" % (rowIdx/2, idx, dir)
                break
        if slotEdge:
            break
    changeTarget(slotEdge, vehicleID)
    stopObject(slotEdge, vehicleID, SLOT_LENGTH-1.)

def reroutePerson(edge, vehicleID):
    row = int(edge[5])
    targetEdge = "footmain%sto%s" % (row, row+1)
    if row == DOUBLE_ROWS-1:
        targetEdge = "footmainout"
    stopObject(edge, vehicleID, 1., 0.)
    changeTarget(targetEdge, vehicleID)
    stopObject(targetEdge, vehicleID, ROW_DIST-10.)

def main():
    sumoExe = SUMO
    if options.gui:
        sumoExe = SUMOGUI
    sumoProcess = subprocess.Popen("%s -c %s.sumo.cfg" % (sumoExe, PREFIX), shell=True)
    initTraCI(PORT)
    occupancy = -numpy.ones((2*DOUBLE_ROWS, SLOTS_PER_ROW), int)
    vehicleStatus = {}
    persons = {}
    waiting = {}
    
    for step in range(1, 2000):
        if options.verbose:
            print "step", step
        moveNodes = simStep(step)
        for vehicleID, edge, pos in moveNodes:
            if vehicleID in vehicleStatus:
                vehicleStatus[vehicleID].edge = edge
            else:
                vehicleStatus[vehicleID] = Status(edge)
                if edge == "mainin":
                    reroute(occupancy, vehicleID)
                elif "foot" in edge:
                    stopObject("-"+edge, vehicleID)
                    parkEdge = edge.replace("foot", "slot")
                    if not parkEdge in persons:
                        persons[parkEdge] = []
                    persons[parkEdge].append(vehicleID)
                    vehicleStatus[vehicleID].parking = True
            if options.verbose:
                print vehicleID, edge
            if edge in persons and pos >= SLOT_LENGTH-1.5:
                if options.verbose:
                    print "destReached", vehicleID, pos
                remaining = []
                for person in persons[edge]:
                    if vehicleStatus[person].parking:
                        reroutePerson(edge.replace("slot", "-foot"), person)
                        vehicleStatus[person].parking = False
                    else:
                        remaining.append(person)
                persons[edge] = remaining
            if edge.startswith("footmain"):
                if not vehicleStatus[vehicleID].parking:
                    if not edge in waiting:
                        waiting[edge] = []
                    waiting[edge].append(vehicleID)
                    vehicleStatus[vehicleID].parking = True
            if edge.startswith("cyber"):
                footEdge = edge.replace("cyber", "footmain")
                if not vehicleStatus[vehicleID].parking:
                    if vehicleStatus[vehicleID].load < CYBER_CAPACITY:                
                        if footEdge in waiting and waiting[footEdge]:
                            stopObject(edge, vehicleID, ROW_DIST-5.)
                            vehicleStatus[vehicleID].parking = True
                else:
                    if pos >= ROW_DIST - 10:
                        while waiting[footEdge] and vehicleStatus[vehicleID].load < CYBER_CAPACITY:
                            person = waiting[footEdge].pop(0)
                            stopObject(footEdge, person, ROW_DIST-10., 0.)
                            vehicleStatus[vehicleID].load += 1
                        stopObject(edge, vehicleID, ROW_DIST-5., 0.)
                        vehicleStatus[vehicleID].parking = False
    close()

optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-g", "--gui", action="store_true", dest="gui",
                     default=False, help="run with GUI")
(options, args) = optParser.parse_args()
main()