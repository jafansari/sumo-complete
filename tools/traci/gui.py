# -*- coding: utf-8 -*-
"""
@file    gui.py
@author  Michael.Behrisch@dlr.de
@date    2011-03-09
@version $Id$

Python implementation of the TraCI interface.

Copyright (C) 2011 DLR (http://www.dlr.de/) and contributors
All rights reserved
"""
import struct, traci
import traci.constants as tc

DEFAULT_VIEW = 'View #0'
RETURN_VALUE_FUNC = {tc.ID_LIST:           traci.Storage.readStringList,
                     tc.VAR_VIEW_ZOOM:     traci.Storage.readDouble,
                     tc.VAR_VIEW_OFFSET:   lambda(result): result.read("!dd"),
                     tc.VAR_VIEW_SCHEMA:   traci.Storage.readString,
                     tc.VAR_VIEW_BOUNDARY: lambda(result): (result.read("!dd"), result.read("!dd"))}
subscriptionResults = {}

def _getUniversal(varID, viewID):
    result = traci._sendReadOneStringCmd(tc.CMD_GET_GUI_VARIABLE, varID, viewID)
    return RETURN_VALUE_FUNC[varID](result)

def getIDList():
    return _getUniversal(tc.ID_LIST, "")

def getZoom(viewID=DEFAULT_VIEW):
    return _getUniversal(tc.VAR_VIEW_ZOOM, viewID)

def getOffset(viewID=DEFAULT_VIEW):
    return _getUniversal(tc.VAR_VIEW_OFFSET, viewID)

def getSchema(viewID=DEFAULT_VIEW):
    return _getUniversal(tc.VAR_VIEW_SCHEMA, viewID)

def getBoundary(viewID=DEFAULT_VIEW):
    return _getUniversal(tc.VAR_VIEW_BOUNDARY, viewID)


def subscribe(viewID, varIDs=(tc.VAR_VIEW_OFFSET,), begin=0, end=2**31-1):
    _resetSubscriptionResults()
    traci._subscribe(tc.CMD_SUBSCRIBE_GUI_VARIABLE, begin, end, viewID, varIDs)

def _resetSubscriptionResults():
    subscriptionResults.clear()

def _addSubscriptionResult(viewID, varID, data):
    if viewID not in subscriptionResults:
        subscriptionResults[viewID] = {}
    subscriptionResults[viewID][varID] = RETURN_VALUE_FUNC[varID](data)

def getSubscriptionResults(viewID=None):
    if viewID == None:
        return subscriptionResults
    return subscriptionResults.get(viewID, None)


def setZoom(viewID, zoom):
    traci._sendDoubleCmd(tc.CMD_SET_GUI_VARIABLE, tc.VAR_VIEW_ZOOM, viewID, zoom)

def setOffset(viewID, x, y):
    traci._beginMessage(tc.CMD_SET_GUI_VARIABLE, tc.VAR_VIEW_OFFSET, viewID, 1+4+4)
    traci._message.string += struct.pack("!Bff", tc.POSITION_2D, x, y)
    traci._sendExact()

def setSchema(viewID, schemeName):
    traci._sendStringCmd(tc.CMD_SET_GUI_VARIABLE, tc.VAR_VIEW_SCHEMA, viewID, schemeName)

def setBoundary(viewID, xmin, ymin, xmax, ymax):
    traci._beginMessage(tc.CMD_SET_GUI_VARIABLE, tc.VAR_VIEW_BOUNDARY, viewID, 1+4+4+4+4)
    traci._message.string += struct.pack("!Bffff", tc.TYPE_BOUNDINGBOX, xmin, ymin, xmax, ymax)
    traci._sendExact()

def screenshot(viewID, filename):
    traci._sendStringCmd(tc.CMD_SET_GUI_VARIABLE, tc.VAR_SCREENSHOT, viewID, filename)

def trackVehicle(viewID, vehID):
    traci._sendStringCmd(tc.CMD_SET_GUI_VARIABLE, tc.VAR_TRACK_VEHICLE, viewID, vehID)
