"""
@file    getPaths.py
@author  Yun-Pang.Wang@dlr.de
@date    2007-02-01
@version $Id$

This script is find the new paths for all OD pairs.
The Dijkstra algorithm is applied for searching the shortest paths.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""

import os, random, string, sys
from dijkstra import dijkstra
from elements import Vertex, Edge, Path, pathNum

def findNewPath(startVertices, endVertices, net, newRoutes, matrixPshort):
    newRoutes = 0
    start = -1
    for startVertex in startVertices:
        start += 1
        end = -1
        D,P = dijkstra(startVertex)
        for endVertex in endVertices:
            end += 1
            endnode = endVertex
            if matrixPshort[start][end] > 0. and str(startVertex) != str(endVertex): 
                tempPath = []
                helpPath = []
                pathcost = 0.0
                while 1:
                    tempPath.append(endnode)
                    if endnode == startVertex: 
                        break
                    endnode = P[endnode]
                tempPath.reverse()
                for i in range(0, len(tempPath)):   
                    if tempPath[i] != endVertex:
                        node = tempPath[i]
                        for edge in node.outEdges:
                            if str(tempPath[i]) != str(tempPath[i+1]) and str(edge.source) == str(tempPath[i]) and str(edge.target) == str(tempPath[i+1]):
                                helpPath.append(edge)
                    else:
                        pathcost = D[endVertex]/3600.

                ODPaths = net._paths[startVertex][endVertex]
                NewPath = True
                nopath = False                    
                for path in ODPaths:
                    sameEdgeCount = 0
                    sametraveltime = 0.                                           
                    if len(path.Edges) == len(helpPath):
                        for i in range (0, len(helpPath)):
                            if str(helpPath[i]) == str(path.Edges[i]):
                                sameEdgeCount += 1
                                sametraveltime += helpPath[i].actualtime
                    if sameEdgeCount == len(path.Edges):
                        samePath = path
                        NewPath = False
                        break
                    elif (sameEdgeCount-len(path.Edges)) <= 5 and abs(sametraveltime/3600. - pastcost) < 0.05:
                        nopath = True
                        break

                if NewPath:
                    newpath = Path()
                    ODPaths.append(newpath)
                    newpath.source = startVertex
                    newpath.target = endVertex
                    newpath.actpathtime = pathcost
                    newpath.Edges = helpPath
                    for edge in newpath.Edges:
                        newpath.freepathtime += edge.freeflowtime
                    newRoutes += 1
                elif not NewPath:
                    samePath.actpathtime = pathcost

    return newRoutes