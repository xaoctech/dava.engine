#!/usr/bin/env python2.5
#
#  autotesting.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import pickle;
import zlib;
import string;
import sys;
import platform;
import shutil;
import random;

print "*** DAVA Launching autotesting"

framework_path = { "Darwin": "./../../../dava.framework", "Windows": "./../../../dava.framework", "Microsoft": "./../../../dava.framework" }

print "platform.system: " + platform.system()
print sys.argv

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + sys.argv[1])
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
if os.path.exists(autotestingDestFolder):    
    print "Autotesting already exists - delete " + autotestingDestFolder
    shutil.rmtree(autotestingDestFolder)
print "copy " + autotestingSrcFolder + " to " + autotestingDestFolder
shutil.copytree(autotestingSrcFolder, autotestingDestFolder)

randomNumber = random.randint(0, 100000)
print "randomNumber: " + str(randomNumber)
idFilePath = os.path.realpath(projectDir + "/Data/Autotesting/id.txt")
print "write to file " + idFilePath
file=open(idFilePath,'w')
file.write(str(randomNumber))
file.close()

testsFolder = os.path.realpath(projectDir + "/Data/Autotesting/Tests")
testFiles = os.listdir(testsFolder)

executableName = ""
executableBuildPath = ""
executableRunPath = ""
#TODO: different name depending on OS
if (platform.system() == "Windows"):
    #TODO: Windows
    executableName = sys.argv[2] + ".exe"
    print "executableName: " +executableName
    
    buildFolder = "/" + sys.argv[3] + "/"

    executableBuildPath = os.path.realpath(projectDir + buildFolder + executableName)
    executableRunPath = os.path.realpath(projectDir + "/" + executableName)
    
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
        os.remove(executableRunPath)
    print "copy " + executableBuildPath + " to " + executableRunPath
    shutil.copy(executableBuildPath, executableRunPath)
elif (platform.system() == "Darwin"):
    #TODO: iOS
    executableName = sys.argv[3] + "/" + sys.argv[2] + ".app"
    print "executableName: " +executableName
else:
    print "Error: unsupported platform.system: " + platform.system()    

os.chdir(projectDir)
for testFile in testFiles:
    print "current file is: " + testFile
   
    if (platform.system() == "Windows"):
        print "run " + executableRunPath
        
        params = [executableRunPath]
        os.system("start /WAIT "+executableName)
    elif (platform.system() == "Darwin"):
        #TODO: iOS
        print "TODO: run on MacOS or deploy and run on iOS " + executableName
        #os.spawnv(os.P_WAIT, "./ResourcePacker", params)
    else:
        print "Error: unsupported platform.system: " + platform.system()
    
print "TODO: report"

print "TODO: cleanup"    
if os.path.exists(autotestingDestFolder):    
    print "Delete " + autotestingDestFolder
    shutil.rmtree(autotestingDestFolder)
if os.path.exists(executableRunPath):    
    print "delete " + executableRunPath
    os.remove(executableRunPath)    
print "*** DAVA Finished autotesting"