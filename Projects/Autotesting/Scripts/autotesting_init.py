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
import string;
import platform;
import shutil;
import random;

print "*** DAVA Initializing autotesting"

print "platform.system: " + platform.system()
print sys.argv

index_OS = 1;
index_Project = 2;

def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + sys.argv[index_Project])
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingConfigSrcPath = os.path.realpath(currentDir + "/../Data/Config.h")
autotestingConfigDestPath = os.path.realpath(frameworkDir + "/Sources/Internal/Autotesting/Config.h")
if os.path.exists(autotestingConfigDestPath):    
    print "delete " + autotestingConfigDestPath
    os.remove(autotestingConfigDestPath)
print "copy " + autotestingConfigSrcPath + " to " + autotestingConfigDestPath
shutil.copy(autotestingConfigSrcPath, autotestingConfigDestPath)

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
if (platform.system() == "Darwin"):
    if (sys.argv[index_OS] == "iOS"):
        print "copy iOS scripts from " + currentDir + " to " + autotestingSrcFolder
        
        scripts = ["/runOnDevice.sh", "/floatsign.sh", "/transporter_chief.rb", "/testRun.js"]
        for scriptName in scripts:
            scriptSrcPath = os.path.realpath(currentDir + scriptName)
            scriptDestPath = os.path.realpath(autotestingSrcFolder + scriptName)
            if os.path.exists(scriptDestPath):    
                print "delete " + scriptDestPath
                os.remove(scriptDestPath)
            print "copy " + scriptSrcPath + " to " + scriptDestPath
            shutil.copy(scriptSrcPath, scriptDestPath)

if os.path.exists(autotestingDestFolder):    
    print "Autotesting already exists - delete " + autotestingDestFolder
    shutil.rmtree(autotestingDestFolder)
print "copy " + autotestingSrcFolder + " to " + autotestingDestFolder
shutil.copytree(autotestingSrcFolder, autotestingDestFolder, ignore=ignored_svn_files)

randomNumber = random.randint(0, 100000)
print "randomNumber: " + str(randomNumber)
idFilePath = os.path.realpath(projectDir + "/Data/Autotesting/id.txt")
print "write to file " + idFilePath
file=open(idFilePath,'w')
file.write(str(randomNumber))
file.close()
   
print "*** DAVA Initialized autotesting"