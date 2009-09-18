#!/usr/bin/env python
"""
@file    route_1htoDay.py
@author  Daniel.Krajzewicz@dlr.de
@date    11.09.2009
@version $Id: route_1htoDay 6858 2009-02-25 13:27:57Z behrisch $

Uses "route_departOffset.py" for building 24 route files which describe
 a whole day assuming the given route files describes an hour.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""

import os

if len(sys.argv) < 4:
    print "Usage: route_1htoDay.py <INPUT_FILE>"
    sys.exit()
for i in range(0, 24):
    out = sys.argv[1]
    out = out[:out.find(".")] + str(i) + out[out.find("."):]
    os.system("route_departOffset.py " + sys.argv[1] + " " + out + " " + str(i*3600))
