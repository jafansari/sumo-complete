#!/usr/bin/env python
"""
A module for building a demand.

@file    demand.py
@author  Daniel Krajzewicz
@date    2013-10-10
@version $Id: demand.py 14731 2013-09-20 18:56:22Z behrisch $

SUMO, Simulation of Urban MObility; see http://sumo-sim.org/
Copyright (C) 2013 DLR (http://www.dlr.de/) and contributors

This file is part of SUMO.
SUMO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
"""
import random
import sumolib          
import os
import subprocess
import math



class LinearChange:
  def __init__(self, beginFlow, endFlow, beginTime, endTime):
    self.beginFlow = beginFlow /3600.
    self.endFlow = endFlow /3600.
    self.beginTime = beginTime
    self.endTime = endTime
  def depart(self, t):
    return random.random()<(self.beginFlow+(self.endFlow-self.beginFlow)/(self.endTime-self.beginTime)*(t-self.beginTime))

class WaveComposition:
  def __init__(self, offset, curves):
    self.offset = offset
    self.curves = curves
  def depart(self, t):
    v = self.offset
    for c in self.curves:
      dt = t-c[3]
      v = v + c[0]*math.sin(2*math.pi*dt*c[2]) + c[1]*math.cos(2*math.pi*dt*c[2])
    v = v/3600.
    return random.random()<v


class Vehicle:
  def __init__(self, id, depart, fromEdge, toEdge, vType):
    self.id = id
    self.depart = depart
    self.fromEdge = fromEdge
    self.toEdge = toEdge
    self.vType = vType

class Stream:
  def __init__(self, sid, validFrom, validUntil, numberModel, departEdgeModel, arrivalEdgeModel, vTypeModel):
    self.sid = sid
    self._numberModel = numberModel
    self._departEdgeModel = departEdgeModel
    self._arrivalEdgeModel = arrivalEdgeModel
    self._vTypeModel = vTypeModel
    self._validFrom = validFrom
    self._validUntil = validUntil

  def getVehicleDepartures(self, b, e):
    if self._validFrom!=None and self._validUntil!=None and (e<self._validFrom or b>self._validUntil):
      return []
    ret = []
    for i in range(b, e):
      if self._validFrom!=None and self._validUntil!=None and (i<self._validFrom or i>self._validUntil):
        continue
      if isinstance(self._numberModel, int) or isinstance(self._numberModel, float): 
        if random.random()<float(self._numberModel)/3600.:
          ret.append(i)
      elif self._numberModel.depart(i):
        ret.append(i)
    return ret
         

  def getFrom(self, what, i, number):
    if isinstance(what, str): return what
    if isinstance(what, int): return what
    if isinstance(what, float): return what
    if isinstance(what, list): return what[i%len(what)]
    if isinstance(what, dict):
      r = random.random()
      s = 0 
      for k in what:
        s = s + k
        if s>r: return what[k]
      return None
    return what.get()
          
  def toVehicles(self, b, e):
    vehicles = []
    departures = self.getVehicleDepartures(b, e)
    number = len(departures)
    for i,d in enumerate(departures):
        fromEdge = self.getFrom(self._departEdgeModel, i, number)
        toEdge = self.getFrom(self._arrivalEdgeModel, i, number)
        vType = self.getFrom(self._vTypeModel, i, number)
        sid = self.sid   
        if sid==None:
          sid = fromEdge + "_to_" + toEdge + "_" + str(i)
        vehicles.append(Vehicle(sid+"#"+str(i), int(d), fromEdge, toEdge, vType))  
    return vehicles




class Demand:
  def __init__(self):
    self.streams = []

  def addStream(self, s):
    self.streams.append(s)
  
  def build(self, b, e, netName, routesName="input_routes.rou.xml"):
    vehicles = []
    for s in self.streams:
      vehicles.extend(s.toVehicles(b, e))
    fdo = open("input_trips.rou.xml", "w")
    fdo.write("<routes>\n")
    for v in sorted(vehicles, key=lambda veh: veh.depart):
      fdo.write('    <trip id="%s" depart="%s" from="%s" to="%s" type="%s"/>\n' % (v.id, v.depart, v.fromEdge, v.toEdge, v.vType))
    fdo.write("</routes>")
    fdo.close()

    duarouter = sumolib.checkBinary("duarouter")
    retCode = subprocess.call([duarouter, "-v", "-n", netName,  "-t", "input_trips.rou.xml", "-o", routesName])
    os.remove(nodesFile)
    os.remove(edgesFile)
    os.remove(connectionsFile)
