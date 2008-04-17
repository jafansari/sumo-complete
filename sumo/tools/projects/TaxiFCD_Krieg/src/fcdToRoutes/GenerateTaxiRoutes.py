# -*- coding: Latin-1 -*-
"""
@file    GenerateTaxiRoutes.py
@author  Sascha.Krieg@dlr.de
@date    2008-04-07

Creates an Route-File, which contains for each Taxi the route, from an FCD-File.

Copyright (C) 2008 DLR/FS, Germany
All rights reserved
"""

from util.CalcTime import getTimeInSecs
import util.Path as path

#global vars
taxis=[]
routes=[]
vlsEdges=[]
taxiIdDict={} #contains for each Taxi the actual "TaxiId" based on the number of single routes which are created for them
fcdDict={}

    
def readVLS_Edges():    
    """Reads the net file and returns a list of all edges""" 
    global vlsEdges
           
    inputFile=open(path.net,'r')
    for line in inputFile:
        if line.find(" <edges")!=-1:
            #         delete edges tag at start and end    
            words=line[line.find(">")+1:line.find("</")].split(" ")            
            break
    inputFile.close()
    vlsEdges=words


def getTaxiId(taxi):       
    return "%s_%s" %(taxi,taxiIdDict.setdefault(taxi,0))
    

def readFCD(): 
    """Reads the FCD and creates a list of Taxis and for each a list of routes"""
       
    inputFile=open(path.fcd,'r')
    for line in inputFile:
        words= line.split("\t")
        #add route
        taxiId=getTaxiId(words[4])              
        if taxiId in taxis:           
            if words[1] in vlsEdges:
                routes[taxis.index(taxiId)].append(words[1])
            else:
                taxiIdDict[words[4]]+=1                
        elif words[1] in vlsEdges: #if the edge is in the VLS-Area a new route is created 
            taxis.append(taxiId)
            #                 departTime               
            routes.append([getTimeInSecs(words[0]),words[1]])
           
    inputFile.close() 
    print len(taxis) 
    
    
def readFCDComplete():
    """Reads the FCD-File and creates a list of Id's with a belonging List of Data tuples."""
    readVLS_Edges()
    
    inputFile=open(path.fcd,'r')
    for line in inputFile:
        words= line.split("\t")
        #add route
        taxiId=getTaxiId(words[4])              
        if taxiId in taxis:           
            if words[1] in vlsEdges:
                #routes[taxis.index(taxiId)].append(words[1])
                fcdDict[taxiId].append((getTimeInSecs(words[0]),words[1],words[2]))
            else:
                taxiIdDict[words[4]]+=1                
        elif words[1] in vlsEdges: #if the edge is in the VLS-Area a new route is created 
            taxis.append(taxiId)
            #                 departTime               
            #routes.append([(int)(mktime(strptime(words[0],format))-simDate),words[1]])
            fcdDict[taxiId]=[(getTimeInSecs(words[0]),words[1],words[2])]
           
    inputFile.close()
    return fcdDict
    
        
def writeRoutes():
    """Writes the collected values in a Sumo-Routes-File"""
    outputFile=open(path.routes,'w')
    
    outputFile.write("<routes>\n")
    # known for like used in java
    for i in xrange(len(taxis)):              
        if len(routes[i])>3: 
            outputFile.write("\t<vehicle id=\""+taxis[i]+"\" type=\"taxi\" depart=\""+ str(routes[i].pop(0))+"\" color=\"1,0,0\">\n")
            outputFile.write("\t\t<route>")
            for edge in routes[i]:
                  outputFile.write(edge+" ")
            outputFile.seek(-1,1) #delete the space between the last edge and </route>
            outputFile.write("</route>\n")
            outputFile.write("\t</vehicle>\n")
       
    outputFile.write("</routes>")
    outputFile.close()
    
    
