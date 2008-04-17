# -*- coding: Latin-1 -*-
"""
@file    CalcTime.py
@author  Sascha.Krieg@dlr.de
@date    2008-04-17


Copyright (C) 2008 DLR/FS, Germany
All rights reserved
"""

from time import gmtime
from time import strftime
from time import strptime
from time import mktime
from time import localtime

format="%Y-%m-%d %H:%M:%S" 
formatTime="%H:%M:%S" 
# used simulation date in seconds
simDate=mktime((2007,7,18,0,0,0,2,199,1))


def main(): 
    print "start program"
    getSecsInTime()
    print "end"

     
def getSecsInTime(time=24543):
    """calculates the Hour:Minutes:Seconds of the given time in seconds."""
    print strftime(formatTime,gmtime(time))
    
def getDateFromDepart(time):
    """calculates the Date from the departtime"""
    return strftime(format,localtime(simDate+ int(time)))

def getTimeInSecs(date):
    """calculates the depart time in seconds of the given date (date should be same as simDate)"""
    return (int)(mktime(strptime(date,format))-simDate)

#start the program
#main()