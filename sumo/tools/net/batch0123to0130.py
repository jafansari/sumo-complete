#!/usr/bin/python
"""
@file    batch0103to0110.py
@author  Daniel.Krajzewicz@dlr.de
@date    2007
@version $Id: batch0103to0110.py 9525 2011-01-04 21:22:52Z behrisch $

Applies the transformation on all nets in the given folder or 
 - if no folder is given - in the base folder (../..).

Copyright (C) 2009-2011 DLR (http://www.dlr.de/) and contributors
All rights reserved
"""

import os, os.path, sys

r = "../../"
if len(sys.argv)>1:
    r = sys.argv[1]
srcRoot = os.path.join(os.path.dirname(sys.argv[0]), r)
for root, dirs, files in os.walk(srcRoot):
    for name in files:
        if name.endswith(".net.xml") or name=="net.netconvert" or name=="net.netgen":
            p = os.path.join(root, name)
            print "Patching " + p + "..."
            os.system("0123to0130.py " + p)
            os.remove(p)
            os.rename(p+".chg", p)
        for ignoreDir in ['.svn', 'foreign']:
            if ignoreDir in dirs:
                dirs.remove(ignoreDir)
