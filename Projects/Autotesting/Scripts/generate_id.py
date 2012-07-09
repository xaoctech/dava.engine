#!/usr/bin/env python2.6
#
#  autotesting_generate_id.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import string;
#import random;
import time;
import datetime;

print "generate id"

index_Project = 1;
index_DestFolder = 2;

#testsId = random.randint(0, 100000)
testsId = int(round(time.time()))
print "testsId: " + str(testsId)

currentDate = datetime.datetime.today()
currentYear = currentDate.year
currentMonth = currentDate.month
currentDay = currentDate.day
currentDateStr = str(currentYear) + "{0:02d}".format(currentMonth) + "{0:02d}".format(currentDay)
print "testsDate: " + currentDateStr

idFilePath = os.path.realpath(sys.argv[index_DestFolder] + "/id.txt")
print "write to file " + idFilePath
file=open(idFilePath,'w')
file.write(str(testsId))
file.write("\n")
file.write(currentDateStr)
file.write("\n")
file.write(sys.argv[index_Project])
file.close()
   
print "generated id"