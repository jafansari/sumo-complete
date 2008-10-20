"""
@file    vehicleControl.py
@author  Michael.Behrisch@dlr.de
@date    2008-07-21
@version $Id$

Control the CityMobil parking lot via TraCI.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""
import subprocess, random
from optparse import OptionParser

import statistics
from constants import *
from traciControl import initTraCI, simStep, stopObject, changeTarget, close

class Manager:
    def personArrived(self, vehicleID, edge, target):
        raise NotImplementedError
    def cyberCarArrived(self, vehicleID, edge, step):
        raise NotImplementedError

class Status:

    def __init__(self, edge, pos):
        self.edge = edge
        self.pos = pos
        self.parking = False
        self.target = None
        self.targetPos = None
        self.slot = None

class Setting:
    step = 0
    manager = None 
    verbose = False
    cyber = False

setting = Setting()
occupancy = {}
vehicleStatus = {}
persons = {}
waiting = {}

def init(manager):
    optParser = OptionParser()
    optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                         default=False, help="tell me what you are doing")
    optParser.add_option("-g", "--gui", action="store_true", dest="gui",
                         default=False, help="run with GUI")
    optParser.add_option("-c", "--cyber", action="store_true", dest="cyber",
                         default=False, help="use small cybercars instead of big busses")
    optParser.add_option("-d", "--demand", type="int", dest="demand",
                         default=15, help="period with which the persons are emitted")
    (options, args) = optParser.parse_args()
    sumoExe = SUMO
    if options.gui:
        sumoExe = SUMOGUI
    sumoConfig = PREFIX + ".sumo.cfg"
    if options.cyber:
        sumoConfig = PREFIX + "_cyber.sumo.cfg"
    sumoProcess = subprocess.Popen("%s -c %s" % (sumoExe, sumoConfig), shell=True)
    initTraCI(PORT)
    setting.manager = manager
    setting.verbose = options.verbose
    setting.cyber = options.cyber
    try:
        while setting.step < 100 or statistics.personsRunning > 0:
            doStep()
    finally:
        close()
    statistics.evaluate()

def getCapacity():
    if setting.cyber:
        return CYBER_CAPACITY
    return BUS_CAPACITY

def stopAt(vehicleID, edge, pos=None):
    if setting.verbose:
        print "stopAt", vehicleID, edge
    if pos == None:
        pos = ROW_DIST-15.
        if edge.endswith("out"):
            pos = 90.
    changeTarget(edge, vehicleID)
    stopObject(edge, vehicleID, pos)
    vehicleStatus[vehicleID].target = edge
    vehicleStatus[vehicleID].targetPos = pos

def leaveStop(vehicleID, newTarget=None, delay=0.):
    v = vehicleStatus[vehicleID]
    if newTarget:
        changeTarget(newTarget, vehicleID)
    stopObject(v.target, vehicleID, v.targetPos, delay)
    v.target = None
    v.targetPos = None
    v.parking = False

def _rerouteCar(vehicleID):
    slotEdge = ""
    for rowIdx in range(DOUBLE_ROWS):
        for idx in range(SLOTS_PER_ROW):
            for dir in ["l", "r"]:
                slotEdge = "slot%s-%s%s" % (rowIdx, idx, dir)
                if not slotEdge in occupancy:
                    occupancy[slotEdge] = vehicleID
                    stopAt(vehicleID, slotEdge, SLOT_LENGTH-1.)
                    return

def _reroutePersons(edge):
    if edge in persons:
        for person in persons[edge]:
            if not vehicleStatus[person].slot:
                row = int(edge[4])
                targetEdge = "footmain%sto%s" % (row, row+1)
                stopObject(edge.replace("slot", "-foot"), person, 1., 0.)
                stopAt(person, targetEdge)
                vehicleStatus[person].parking = False
                vehicleStatus[person].slot = edge

def _checkInitialPositions(vehicleID, edge, pos):
    if vehicleID in vehicleStatus:
        vehicleStatus[vehicleID].edge = edge
        vehicleStatus[vehicleID].pos = pos
    else:
        vehicleStatus[vehicleID] = Status(edge, pos)
        if edge == "mainin":
            _rerouteCar(vehicleID)
        elif edge == "cyberin":
            stopAt(vehicleID, "cyber0to1")
        elif edge == "footfairin":
            stopAt(vehicleID, "footmainout")
        elif "foot" in edge:
            stopObject("-"+edge, vehicleID)
            parkEdge = edge.replace("foot", "slot")
            if not parkEdge in persons:
                persons[parkEdge] = []
            persons[parkEdge].append(vehicleID)
            vehicleStatus[vehicleID].parking = True
        elif edge.startswith("slot"):
            stopAt(vehicleID, edge, SLOT_LENGTH-1.)
            occupancy[edge] = vehicleID
    _reroutePersons(edge)

def doStep():
    setting.step += 1
    if setting.verbose:
        print "step", setting.step
    moveNodes = simStep(setting.step)
    for vehicleID, edge, pos in moveNodes:
        _checkInitialPositions(vehicleID, edge, pos)
        if edge == vehicleStatus[vehicleID].target and not vehicleStatus[vehicleID].parking:
            if edge.startswith("footmain"):
                vehicleStatus[vehicleID].parking = True
                target = "footmainout"
                if edge == "footmainout":
                    row = random.randrange(0, DOUBLE_ROWS)
                    target = "footmain%sto%s" % (row, row+1)
                statistics.personArrived(vehicleID, edge, target, setting.step)
                setting.manager.personArrived(vehicleID, edge, target)
            if edge.startswith("cyber"):
                vehicleStatus[vehicleID].parking = True
                setting.manager.cyberCarArrived(vehicleID, edge, setting.step)
