import glob, os, shutil, subprocess, time
def makeAndChangeDir(dirName):
    runID = 1
    fullName = "%s%03i" % (dirName, runID)
    while os.path.exists(fullName):
        runID += 1
        fullName = "%s%03i" % (dirName, runID)
    os.mkdir(fullName)
    os.chdir(fullName)
    return fullName

os.chdir("input")
netFile = "../input/" + glob.glob("*.net.xml")[0]
mtxNamesList = ",".join(["../input/" + item for item in glob.glob("*.fma")])
addFiles = ",".join(["../input/" + item for item in glob.glob("*.add.xml")])
pyAdds = ""
sumoAdds = ""
if addFiles:
    pyAdds = "-+ %s" % addFiles
    sumoAdds = "-a %s" % addFiles

succDir = makeAndChangeDir("../successive")
os.system("incrementalAssignment.py -d ../input/districts.xml -m %s -n %s -p ../parameter.txt -u ../CRcurve.txt" % (mtxNamesList, netFile))
shutil.copy("%s/routes.txt" % succDir, "../input/successive.rou.xml")
os.system("route2trips.py ../input/successive.rou.xml > ../input/successive.trips.xml")
duaDir = makeAndChangeDir("../dua")
duaProcess = subprocess.Popen("dua-iterate.py -C -n %s -t ../input/trips.trips.xml %s" % (netFile, pyAdds), shell=True)
clogDir = makeAndChangeDir("../clogit")
os.system("cLogit.py -d ../input/districts.xml -m %s -n %s -p ../clogit_parameter.txt -u ../CRcurve.txt" % (mtxNamesList, netFile))
while not os.path.exists("%s/trips_0.rou.xml" % duaDir):
    time.sleep(1)
shutil.copy("%s/trips_0.rou.xml" % duaDir, "../input/routes.rou.xml")
shotDir = makeAndChangeDir("../oneshot")
os.system("one-shot.py -n %s -t ../input/routes.rou.xml %s" % (netFile, pyAdds))

duaProcess.wait()
makeAndChangeDir("../routes")
for step in [0, 24, 49]:
    os.system("sumo --no-step-log -n %s -e 90000 -r %s/trips_%s.rou.xml --vehroute-output vehroutes_dua_%s.xml %s -l sumo_dua_%s.log" % (netFile, duaDir, step, step, sumoAdds, step))
    os.system("networkStatistics.py -n %s -d ../input/districts.xml -x vehroutes_dua_%s.xml -o networkStatistics_%s_%s.txt" % (netFile, step, os.path.basename(duaDir), step))
for step in [-1, 1800, 300, 15]:
    shutil.copy("%s/vehroutes_%s.xml" % (shotDir, step), "vehroutes_oneshot%s.xml" % step)
    os.system("networkStatistics.py -n %s -d ../input/districts.xml -x vehroutes_oneshot%s.xml -o networkStatistics_%s_%s.txt" % (netFile, step, os.path.basename(shotDir), step))
os.system("sumo --no-step-log -n %s -e 90000 -r %s/routes.txt --vehroute-output vehroutes_successive.xml %s -l sumo_successive.log" % (netFile, succDir, sumoAdds))
os.system("networkStatistics.py -n %s -d ../input/districts.xml -x vehroutes_successive.xml -o networkStatistics_%s.txt" % (netFile, os.path.basename(succDir)))
os.system("sumo --no-step-log -n %s -e 90000 -r %s/routes.txt --vehroute-output vehroutes_clogit.xml %s -l sumo_clogit.log" % (netFile, clogDir, sumoAdds))
os.system("networkStatistics.py -n %s -d ../input/districts.xml -x vehroutes_clogit.xml -o networkStatistics_%s.txt" % (netFile, os.path.basename(clogDir)))
os.chdir("..")
