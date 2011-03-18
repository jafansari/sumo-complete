#!/usr/bin/env python

import os, subprocess, sys, time

sumoHome = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '..', '..', '..', '..'))
sys.path.append(os.path.join(sumoHome, "tools"))
import traci

if sys.argv[1]=="sumo":
    sumoBinary = os.environ.get("SUMO_BINARY", os.path.join(sumoHome, 'bin', 'sumo'))
    addOption = ""
else:
    sumoBinary = os.environ.get("GUISIM_BINARY", os.path.join(sumoHome, 'bin', 'sumo-gui'))
    addOption = "-Q"
PORT = 8813
DELTA_T = 1000

def runSingle(sumoEndTime, traciEndTime):
    fdi = open("sumo.sumo.cfg")
    fdo = open("used.sumo.cfg", "w")
    for line in fdi:
        line = line.replace("%end%", str(sumoEndTime))
        fdo.write(line)
    fdi.close()
    fdo.close()
    doClose = True
    step = 0
    sumoProcess = subprocess.Popen("%s -c used.sumo.cfg %s" % (sumoBinary, addOption), shell=True, stdout=sys.stdout)
    traci.init(PORT)
    while not step>traciEndTime:
        traci.simulationStep(DELTA_T)
        vehs = traci.vehicle.getIDList()
        if vehs.index("horiz")<0 or len(vehs)>1:
            print "Something is false"
        step += 1
    print "Print ended at step %s" % (traci.simulation.getCurrentTime()/DELTA_T)
    traci.close()
    sys.stdout.flush()
    
print "----------- SUMO ends first -----------"
sys.stdout.flush()
for i in range(0, 10):
    print " Run %s" % i
    runSingle(50, 99)

print "----------- TraCI ends first -----------"
sys.stdout.flush()
for i in range(0, 10):
    print " Run %s" % i
    runSingle(101, 99)
