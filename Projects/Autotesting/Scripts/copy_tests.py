#!/usr/bin/python
#
#  autotesting_init.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import string;
import platform;
import shutil;
import subprocess;

arguments = sys.argv[1:]

index_Project = 0
index_AutotestingDestPath = 1

if 0 == len(arguments) or 2 != len(arguments):
	print 'Usage: ./autotesting_init.py [ProjectName] [AutotestingDestPath]'
	exit(1)

def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

autotestingSrcFolder = os.getcwd()
autotestingDestFolder = arguments[index_AutotestingDestPath]

autotestingActionsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Actions")
autotestingActionsDestFolder = os.path.realpath(autotestingDestFolder + "/Actions")

autotestingTestsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Tests")
autotestingTestsDestFolder = os.path.realpath(autotestingDestFolder + "/Tests")

if os.path.exists(autotestingActionsDestFolder):    
    shutil.rmtree(autotestingActionsDestFolder)
os.mkdir(autotestingActionsDestFolder)

if os.path.exists(autotestingTestsDestFolder):    
    shutil.rmtree(autotestingTestsDestFolder)
os.mkdir(autotestingTestsDestFolder)

print "copy " + autotestingActionsSrcFolder + " to " + autotestingActionsDestFolder
shutil.copytree(autotestingActionsSrcFolder, autotestingActionsDestFolder, ignore=ignored_svn_files)

print "copy " + autotestingTestsSrcFolder + " to " + autotestingTestsDestFolder
shutil.copytree(autotestingTestsSrcFolder, autotestingTestsDestFolder, ignore=ignored_svn_files)