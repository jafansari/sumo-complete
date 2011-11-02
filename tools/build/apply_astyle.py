#!/usr/bin/env python
"""
@file    apply_astyle.py
@author  Michael Behrisch
@date    2007
@version $Id$

Applies astyle with the proper settings used in SUMO on all
 files in src.

Copyright (C) 2008-2011 DLR (http://www.dlr.de/) and contributors
All rights reserved
"""

import os, os.path, sys

srcRoot = os.path.join(os.path.dirname(__file__), "../../src/")
for root, dirs, files in os.walk(srcRoot):
    for name in files:
        if name.endswith(".h") or name.endswith(".cpp"):
            os.system("astyle --style=java --unpad-paren --pad-header --pad-oper --add-brackets --indent-switches --align-pointer=type -n " + os.path.join(root, name))
        for ignoreDir in ['.svn', 'foreign']:
            if ignoreDir in dirs:
                dirs.remove(ignoreDir)
