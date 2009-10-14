#!/usr/bin/env python
import os, subprocess, sys, shutil

sumoDir = os.path.join(os.path.dirname(sys.argv[0]), "..")
sumoBinary = os.path.join(sumoDir, "bin", "sumo.exe")
guisimBinary = os.path.join(sumoDir, "bin", "guisim.exe")
if os.name == 'posix':
    sumoBinary = os.path.join(sumoDir, "src", "sumo")
    guisimBinary = os.path.join(sumoDir, "src", "sumo-guisim")
sumoBinary = os.environ.get("SUMO_BINARY", sumoBinary)
guisimBinary = os.environ.get("GUISIM_BINARY", guisimBinary)

#start sumo to generate config
subprocess.call(sumoBinary + " --message-log guisim.stdout --error-log guisim.stderr --save-configuration guisim.sumo.cfg " + " ".join(sys.argv[1:]), 
                shell=True, stdout=sys.stdout, stderr=sys.stderr)
ret = subprocess.call(guisimBinary + " -Q -c guisim.sumo.cfg", 
                shell=True, stdout=sys.stdout, stderr=sys.stderr)
if os.path.exists("guisim.stderr"):
    f = open("guisim.stderr")
    shutil.copyfileobj(f, sys.stderr)
    f.close()
sys.exit(ret)
