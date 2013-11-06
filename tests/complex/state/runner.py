#!/usr/bin/env python
import os,subprocess,sys

idx = sys.argv.index(":")
saveParams = sys.argv[1:idx]
loadParams = sys.argv[idx+1:]
if '--mesosim' in loadParams:
    saveParams.append('--mesosim')

# need to add runner.py again in options.complex.meso to ensure it is the last  entry
saveParams = [p for p in saveParams if not 'runner.py' in p]
loadParams = [p for p in loadParams if not 'runner.py' in p]

#print "save:", saveParams
#print "load:", loadParams

sumoBinary = os.environ.get("SUMO_BINARY", os.path.join(os.path.dirname(sys.argv[0]), '..', '..', '..', 'bin', 'sumo'))
#print "sumoBinary", sumoBinary
subprocess.call([sumoBinary]+saveParams,
                shell=(os.name=="nt"), stdout=sys.stdout, stderr=sys.stderr)
subprocess.call([sumoBinary]+loadParams,
                shell=(os.name=="nt"), stdout=sys.stdout, stderr=sys.stderr)
