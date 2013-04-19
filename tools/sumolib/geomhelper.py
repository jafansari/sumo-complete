"""
@file    geomhelper.py
@author  Daniel Krajzewicz
@date    2013-02-25
@version $Id$

Some helper functions for geometrical computations.

SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
Copyright (C) 2008-2013 DLR (http://www.dlr.de/) and contributors
All rights reserved
"""
import math

def distance(p1, p2):
  dx = p1[0]-p2[0]
  dy = p1[1]-p2[1]
  return math.sqrt(dx*dx + dy*dy)


def minDistPositionPto2P(p, p1, p2, perpendicular=False):
  l = distance(p1, p2)
  u = (((p[0] - p1[0]) * (p2[0] - p1[0])) + ((p[1] - p1[1]) * (p2[1] - p1[1]))) / (l * l)
  if u<0.0 or u>1:
    if perpendicular:
      return -1.
    if u<0:
      return 0
    return l
  return u*l

def minDistPositionPtoV(p, s):
  o = 0
  for i in range(0, len(s)-1):
    q = minDistPositionPto2P(p, s[i], s[i+1], True)
    if q!=-1:
      return o+q
    o = o + distance(s[i], s[i+1])
  return -1


def distancePto2P(p, p1, p2, perpendicular=False):
  l = distance(p1, p2)
  u = (((p[0] - p1[0]) * (p2[0] - p1[0])) + ((p[1] - p1[1]) * (p2[1] - p1[1]))) / (l * l)
  if u<0.0 or u>1:
    if perpendicular:
      return -1.
    if u<0:
      return distance(p1, p)
    return distance(p2, p)
  intersection = (p1[0] + u*(p2[0]-p1[0]), p1[1] + u*(p2[1]-p1[1]))
  return distance(p, intersection)

def distancePtoV(p, s):
  for i in range(0, len(s)-1):
    q = distancePto2P(p, s[i], s[i+1], True)
    if q!=-1:
      return q
  return -1


