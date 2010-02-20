#!/usr/bin/env python

import os,subprocess,sys,time
import xml.dom.minidom as dom
sys.path.append(os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), "../../../tools/traci"))
import traciControl
import time

sumoBinary = os.environ.get("SUMO_BINARY", os.path.join(os.path.dirname(sys.argv[0]), '..', '..', '..', 'bin', 'sumo'))
PORT = 8813


def runSingle(sumoEndTime, traciEndTime):
    doClose = True
    step = 0
    sumoProcess = subprocess.Popen("%s -c %s -e %s" % (sumoBinary, "sumo.sumo.cfg", str(sumoEndTime)), shell=True, stdout=sys.stdout)
    traciControl.initTraCI(PORT)
    while not step>traciEndTime:
        try:
            traciControl.cmdSimulationStep(1)
            vehs = traciControl.cmdGetVehicleVariable_idList("x")
            if vehs.index("horiz")<0 or len(vehs)>1:
                print "Something is false"
            step += 1
        except traciControl.FatalTraCIError:
            print "Closed by SUMO"
            doClose = False
            break
    if doClose:
        traciControl.cmdClose()
    print "Print ended at step %s" % step
    sys.stdout.flush()
    
#print "SUMO ends first"
#for i in range(0, 10):
#    print " Run %s" % i
#    runSingle(50, 100)

print "TraCI ends first"
for i in range(0, 10):
    print " Run %s" % i
    runSingle(101, 99)
