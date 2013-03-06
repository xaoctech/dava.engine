#!/usr/bin/python
#
#  copy_tests.py
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

print "copy_tests" 

arguments = sys.argv[1:]

if 0 == len(arguments) or 2 != len(arguments):
	print 'Usage: ./copy_tests.py [TestsSrcFolder] [AutotestingDestPath]'
	exit(1)

def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

def copy_replace_folder(srcFolder, destFolder):
    if os.path.exists(destFolder):   
        print "shutil.rmtree " + destFolder 
        shutil.rmtree(destFolder)
    print "copy " + srcFolder + " to " + destFolder
    shutil.copytree(srcFolder, destFolder, ignore=ignored_svn_files)

testsSrcFolder = arguments[0]
autotestingDestFolder = arguments[1]

autotestingSrcFolder = os.getcwd()

autotestingActionsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Actions")
autotestingActionsDestFolder = os.path.realpath(autotestingDestFolder + "/Actions")
copy_replace_folder(autotestingActionsSrcFolder, autotestingActionsDestFolder)

autotestingTestsSrcFolder = os.path.realpath(autotestingSrcFolder + testsSrcFolder)
autotestingTestsDestFolder = os.path.realpath(autotestingDestFolder + "/Tests")
copy_replace_folder(autotestingTestsSrcFolder, autotestingTestsDestFolder)

autotestingStringsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Strings")
autotestingStringsDestFolder = os.path.realpath(autotestingDestFolder + "/Strings")
copy_replace_folder(autotestingStringsSrcFolder, autotestingStringsDestFolder)



print "copy_tests done" 