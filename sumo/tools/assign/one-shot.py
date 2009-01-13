#!/usr/bin/env python
"""
@file    one-shot.py
@author  Daniel.Krajzewicz@dlr.de
@date    2008-03-10
@version $Id$

This script does multiple sumo runs with different rerouting intervals.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""
import os, sys, subprocess
from datetime import datetime
from optparse import OptionParser

class TeeFile:
    """A helper class which allows simultaneous writes to several files"""
    def __init__(self, *files):
        self.files = files
    def write(self, txt):
        """Writes the text to all files"""
        for fp in self.files:
            fp.write(txt)

def writeSUMOConf(step, options, files):
    fd = open("one_shot_" + str(step) + ".sumo.cfg", "w")
    fd.write("<configuration>\n")
    fd.write("   <files>\n")
    fd.write("      <net-file>" + options.net + "</net-file>\n")
    fd.write("      <route-files>" + files + "</route-files>\n")
    fd.write("      <vehroutes>vehroutes_" + str(step) + ".xml</vehroutes>\n")
    if not options.noEmissions:
        fd.write("      <emissions>emissions_" + str(step) + ".xml</emissions>\n")
    if not options.noTripinfo:
        fd.write("      <tripinfo>tripinfo_" + str(step) + ".xml</tripinfo>\n")
    fd.write("      <additional-files>dump_" + str(step) + ".add.xml" + options.additional)
    if options.additional!="":
        fd.write("," + options.additional)
    fd.write("</additional-files>\n")
    fd.write("   </files>\n")
    fd.write("   <process>\n")
    fd.write("      <begin>" + str(options.begin) + "</begin>\n")
    fd.write("      <end>" + str(options.end) + "</end>\n")
    fd.write("      <route-steps>" + str(options.routeSteps) + "</route-steps>\n")
    if options.mesosim:
        fd.write("      <mesosim>x</mesosim>\n")
    fd.write("      <device.routing.probability>1</device.routing.probability>\n")
    fd.write("      <device.routing.period>" + str(step) + "</device.routing.period>\n")
    fd.write("   </process>\n")
    fd.write("   <reports>\n")
    if options.verbose:
        fd.write("      <verbose>x</verbose>\n")
    if not options.withWarnings:
        fd.write("      <suppress-warnings>x</suppress-warnings>\n")
    fd.write("   </reports>\n")
    fd.write("</configuration>\n")
    fd.close()
    fd = open("dump_%s.add.xml" % step, "w")
    print >> fd, """<a>
    <meandata-edge id="dump_%s_%s" freq="%s" file="dump_%s_%s.xml" excludeEmpty="true"/>
</a>""" % (step, options.aggregation, options.aggregation, step, options.aggregation)
    fd.close()

optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-W", "--with-warnings", action="store_true", dest="withWarnings",
                     default=False, help="enables warnings")

optParser.add_option("-n", "--net-file", dest="net",
                     help="SUMO network (mandatory)", metavar="FILE")
optParser.add_option("-t", "--trips", dest="trips",
                     help="trips in step 0", metavar="FILE")

optParser.add_option("-b", "--begin", dest="begin",
                     type="int", default=0, help="Set simulation/routing begin")
optParser.add_option("-e", "--end", dest="end",
                     type="int", default=86400, help="Set simulation/routing end")
optParser.add_option("-R", "--route-steps", dest="routeSteps",
                     type="int", default=200, help="Set simulation route steps")
optParser.add_option("-a", "--aggregation", dest="aggregation",
                     type="int", default=900, help="Set main weights aggregation period")
optParser.add_option("-f", "--frequencies", dest="frequencies",
                     default="-1,1800,300,15", help="Set the frequencies to iterate over")

optParser.add_option("-E", "--disable-emissions", action="store_true", dest="noEmissions",
                     default=False, help="No emissions are written by the simulation")
optParser.add_option("-T", "--disable-tripinfos", action="store_true", dest="noTripinfo",
                     default=False, help="No tripinfos are written by the simulation")
optParser.add_option("-m", "--mesosim", action="store_true", dest="mesosim",
                     default=False, help="Whether mesosim shall be used")
optParser.add_option("-+", "--additional", dest="additional",
                     default="", help="Additional files")


optParser.add_option("-p", "--path", dest="path",
                     default=os.environ.get("SUMO", ""), help="Path to binaries")
(options, args) = optParser.parse_args()


if (sys.platform=="win32"):        
        sumoBinary = os.path.join(options.path, "sumo.exe")
else:
        sumoBinary = os.path.join(options.path, "sumo")
log = open("one_shot-log.txt", "w")
logQuiet = open("one_shot-log-quiet.txt", "w")
sys.stdout = TeeFile(sys.stdout, logQuiet)
sys.stderr = TeeFile(sys.stderr, logQuiet)
starttime = datetime.now()
for step in options.frequencies.split(","):
    step = int(step)
    btimeA = datetime.now()
    print "> Executing step " + str(step)

    # simulation
    print ">> Running simulation"
    btime = datetime.now()
    print ">>> Begin time %s" % btime
    writeSUMOConf(step, options, options.trips)
    if options.verbose:
        print "> Call: %s -c one_shot_%s.sumo.cfg" % (sumoBinary, step)
        subprocess.call("%s -c one_shot_%s.sumo.cfg" % (sumoBinary, step),
                        shell=True, stdout=TeeFile(sys.__stdout__, log),
                        stderr=TeeFile(sys.__stderr__, log))
    else:
        subprocess.call("%s -c one_shot_%s.sumo.cfg" % (sumoBinary, step),
                        shell=True, stdout=log, stderr=log)
    etime = datetime.now()
    print ">>> End time %s" % etime
    print ">>> Duration %s" % (etime-btime)
    print "<<"
    print "< Step %s ended (duration: %s)" % (step, datetime.now() - btimeA)
    print "------------------\n"
print "one-shot ended (duration: %s)" % (datetime.now() - starttime)

log.close()
logQuiet.close()
