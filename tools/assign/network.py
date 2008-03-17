#!/usr/bin/env python
# This script is to retrive the network and the zone connectors data from XML files. 
# The network XML file is automatically generated by SUMO.

import os, random, string, sys, datetime
from xml.sax import saxutils, make_parser, handler
from elements import Predecessor, Vertex, Edge, Vehicle, Path

# Net class which stores the network (vertex and edge collection)
# All the algorithmic stuffs and the output generation are also
# inside this class. The members are either "private" or have get/add/remove
# methods.

class Net:
    def __init__(self):
        self._vertices = []
        self._edges = {}
        self._vehicles = []
        self._startVertices = []
        self._endVertices = []
        self._paths = {}

    def newVertex(self):
        v = Vertex(len(self._vertices))
        self._vertices.append(v)
        return v

    def addVehicle(self, label):
        t = Vehicle(label)
        self._vehicles.append(t)
        return t
    
    def getEdge(self, edgeLabel):
        return self._edges[edgeLabel]

    def addEdge(self, edgeObj):
        edgeObj.source.outEdges.add(edgeObj)
        edgeObj.target.inEdges.add(edgeObj)
        if edgeObj.kind == "real":
            self._edges[edgeObj.label] = edgeObj

    def addIsolatedRealEdge(self, edgeLabel):
        self.addEdge(Edge(edgeLabel, self.newVertex(), self.newVertex(),
                          "real"))
    
#    def calcPaths(self, startEdgeLabel):
    def calcPaths(self, verbose, NewRoutes, KPaths, startVertices, endVertices, matrixPshort):
        if verbose:
            foutkpath = file('kpaths.txt', 'w')
        start = -1
        for startVertex in startVertices:
            start += 1
            end = -1
            for vertex in self._vertices:
                vertex.preds = []
                vertex.wasUpdated = False
            startVertex.preds.append(Predecessor(None, None, 0))
            updatedVertices = [startVertex]
            if verbose:
                print 'updatedVertices:', updatedVertices

            while len(updatedVertices) > 0:
                vertex = updatedVertices.pop(0)
                vertex.wasUpdated = False
                for edge in vertex.outEdges:
                    if edge.target != startVertex and edge.target.update(KPaths, edge):
                        updatedVertices.append(edge.target)
    
            for endVertex in endVertices:
                end += 1
                if verbose:
                    print 'Number of the new Routes:', NewRoutes
                if str(startVertex) != str(endVertex) and matrixPshort[start][end] != 0.:
                    for startPred in endVertex.preds:
                        newpath = Path()
                        if not startVertex in self._paths:
                            self._paths[startVertex] = {}
                        if not endVertex in self._paths[startVertex]:
                            self._paths[startVertex][endVertex] = []
                        self._paths[startVertex][endVertex].append(newpath)
                        newpath.source = startVertex
                        newpath.target = endVertex
                        
                        pred = startPred
                        vertex = endVertex
                        while vertex != startVertex:
                            if pred.edge.kind == "real":
                                newpath.Edges.append(pred.edge)
                            vertex = pred.edge.source
                            pred = pred.pred
#                        print
                        newpath.Edges.reverse()    
                        if verbose:
                            foutkpath.write('\npathID:%s, source:%s, target:%s, Edges:' %(newpath.label, newpath.source, newpath.target))  
    
                        for edge in newpath.Edges:
                            if verbose:
                                foutkpath.write('%s, ' %(edge.label))
                            newpath.freepathtime += edge.freeflowtime
                            
                        newpath.actpathtime = newpath.freepathtime
                        if verbose:
                            foutkpath.write('Path cost:%s' %newpath.actpathtime) 
                    NewRoutes += 1
        if verbose:
            foutkpath.close()
        
        return NewRoutes
                        
    def printNet(self, foutnet):
        foutnet.write('Name\t Kind\t FrNode\t ToNode\t length\t MaxSpeed\t Lanes\t CR-Curve\t EstCap.\t Free-Flow TT\t Weight\t Connection\n')
        for edgeName, edgeObj in self._edges.iteritems():
            foutnet.write('%s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %d\n' 
            %(edgeName, edgeObj.kind, edgeObj.source, edgeObj.target, edgeObj.length, edgeObj.maxspeed, edgeObj.numberlane, edgeObj.CRcurve, edgeObj.estcapacity, edgeObj.freeflowtime, edgeObj.weight, edgeObj.connection))
     
# The class for parsing the XML input file (network file). The data parsed is
# written into the net. All members are "private".
class NetDetectorFlowReader(handler.ContentHandler):
    def __init__(self, net):
        self._net = net
        self._edgeString = ''
        self._edge = ''
        self._maxSpeed = 0
        self._laneNumber = 0
        self._length = 0
        self._edgeObj = None

    def startElement(self, name, attrs):
        if name == 'edges':
            self._edgeString = ' '
        elif name == 'edge' and (not attrs.has_key('function') or attrs['function'] != 'internal'):
            self._edge = attrs['id']
            self._edgeObj = self._net.getEdge(self._edge)
            self._edgeObj.source.label = attrs['from']
            self._edgeObj.target.label = attrs['to']
            self._maxSpeed = 0
            self._laneNumber = 0
            self._length = 0
        elif name == 'cedge' and self._edge != '':
            fromEdge = self._net.getEdge(self._edge)
            toEdge = self._net.getEdge(attrs['id'])
            newEdge = Edge(self._edge+"_"+attrs['id'], fromEdge.target, toEdge.source)
            self._net.addEdge(newEdge)
            fromEdge.finalizer = attrs['id']
        elif name == 'lane' and self._edge != '':
            self._maxSpeed = max(self._maxSpeed, float(attrs['maxspeed']))
            self._laneNumber = self._laneNumber + 1
            self._length = float(attrs['length'])
      
    def characters(self, content):
        if self._edgeString != '':
            self._edgeString += content

    def endElement(self, name):
        if name == 'edges':
            for edge in self._edgeString.split():
                self._net.addIsolatedRealEdge(edge)
            self._edgeString = ''
        elif name == 'edge':
            self._edgeObj.init(self._maxSpeed, self._length, self._laneNumber)
            self._edge = ''

# The class for parsing the XML input file (zone connectors ). The data parsed is
# written into the net. All members are "private".
class ZoneConnectionReader(handler.ContentHandler):
    def __init__(self, net):
        self._net = net
        self._StartDTIn = None
        self._StartDTOut = None
        self.I = 100

    def startElement(self, name, attrs):
        if name == 'district':
            self._StartDTIn = self._net.newVertex()
            self._StartDTIn.label = attrs['id']
            self._StartDTOut = self._net.newVertex()
            self._StartDTOut.label = self._StartDTIn.label
            self._net._startVertices.append(self._StartDTIn)
            self._net._endVertices.append(self._StartDTOut)

        elif name == 'dsink':
            sourcelink = self._net.getEdge(attrs['id'])
            self.I += 1
            conlink = self._StartDTOut.label + str(self.I)
            newEdge = Edge(conlink, sourcelink.target, self._StartDTOut, "real")
            self._net.addEdge(newEdge)
            newEdge.weight = attrs['weight']
            newEdge.connection = 1
        elif name == 'dsource':
            sinklink = self._net.getEdge(attrs['id'])
            self.I += 1
            conlink = self._StartDTIn.label + str(self.I)
            newEdge = Edge(conlink, self._StartDTIn, sinklink.source, "real")
            self._net.addEdge(newEdge)
            newEdge.weight = attrs['weight']
            newEdge.connection = 2

# The class for parsing the XML input file (vehicle information ). This class is used in the GlobalMOE.py for
# calculating the gloabal network performances, e.g. avg. travel time and avg. travel speed.
class VehInformationReader(handler.ContentHandler):
    def __init__(self, net):
        self._net = net
        self._Vehicle = None
        self._routeString = ''

    def startElement(self, name, attrs):
        if name == 'route':
            self._routeString = ''
        if name == 'vehicle':
#            for key in attrs.getNames():
#                print key, "=", attrs[key], "  "
#            print
            self._Vehicle = self._net.addVehicle(str(attrs['id']))
            self._Vehicle.depart = float(attrs['emittedAt'])
            self._Vehicle.arrival = float(attrs['endedAt'])
#            print self._Vehicle
#            self._net.vehicles.append(self._Vehicle)
    
    def characters(self, content):
        if self._Vehicle:
            self._routeString += content
   
    def endElement(self, name):
        if name == 'vehicle':
            self._Vehicle.route = self._routeString.split()
            self._routeString = ''
            self._Vehicle = None