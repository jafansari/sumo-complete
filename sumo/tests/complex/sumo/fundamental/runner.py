#!/usr/bin/env python

import os,subprocess,sys,time,shutil
sumoHome = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
if "SUMO_HOME" in os.environ:
    sumoHome = os.environ["SUMO_HOME"]
sys.path.append(os.path.join(sumoHome, "tools"))
import sumolib

sumoBinary = os.environ.get("SUMO_BINARY", os.path.join(sumoHome, 'bin', 'sumo'))
netconvertBinary = os.environ.get("NETCONVERT_BINARY", os.path.join(sumoHome, 'bin', 'netconvert'))

def call(command, log):
    print >> log, "-" * 79
    print >> log, command
    log.flush()
    print command
    retCode = subprocess.call(command, stdout=sys.stdout, stderr=sys.stderr)
    if retCode != 0:
        print >> sys.stderr, "Execution of %s failed. Look into %s for details." % (command, log.name)
        sys.exit(retCode) 

USE_ZERO_OFFSET = False

DENSITYSTEP = .01
VEHICLENUMBER = 1000
VEHICLELENGTH = 7.5
RELAXATIONSTEPS = 1000
MEASURESTEPS = 3600
log = open("logfile.txt", "w")

density = DENSITYSTEP
fdo = open("results.csv", "w")
while density<=1.:
    print ">>> Building the network (at density %s)" % (density)
    length = VEHICLENUMBER * VEHICLELENGTH / density
    fd = open("input_edges.edg.xml", "w")
    print >> fd, '<edges>'
    print >> fd, '    <edge id="a" from="a" to="b" numLanes="1" speed="36." length="%s" shape="1,0 50000,0 50000,50000 0,50000 0,1"/>' % (length-.1)
    print >> fd, '    <edge id="b" from="b" to="a" numLanes="1" speed="36." length=".1"/>'
    print >> fd, '</edges>'
    fd.close()
    call([netconvertBinary, "-c", "netconvert.netc.cfg"], log)

    print ">>> Building the routes (at density %s)" % (density)
    vehicleOffset = (length - .1) / float(VEHICLENUMBER)
    fd = open("input_routes.rou.xml", "w")
    print >> fd, '<routes>'
    print >> fd, '    <vType id="t1" accel="0.8" decel="4.5" sigma="0.5" length="%s" maxSpeed="36"/>' % (VEHICLELENGTH)
    print >> fd, '    <route id="r1" multi_ref="x" edges="a b a b a b a"/>'
    pos = 0
    for i in range(0, VEHICLENUMBER):
        print >> fd, '    <vehicle id="v.%s" depart="0" departSpeed="0" departPos="%s" route="r1" type="t1"/>' % (i, pos)
        if USE_ZERO_OFFSET:
            pos = pos + VEHICLELENGTH
        else:
            pos = pos + vehicleOffset
    print >> fd, '</routes>'
    fd.close()


    print ">>> Simulating (at density %s)" % (density)
    call([sumoBinary, "-c", "sumo.sumo.cfg", "-v"], log)

    shutil.copy("aggregated.xml", "backup/" + str(density) + "_aggregated.xml")
    shutil.copy("detector.xml", "backup/" + str(density) + "_detector.xml")
    dump = sumolib.output.dump.readDump("aggregated.xml", ["occupancy"])
    il = sumolib.output.inductionloop.readInductLoop("detector.xml", ["flow"])
    print >> fdo, "%s;%s;%s" % (density, dump.get("occupancy")[-1]["a_0"], il.get("flow")[-1]["det"])

    density = density + DENSITYSTEP
fdo.close()
