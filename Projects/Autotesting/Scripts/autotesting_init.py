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

index_OS = 0
index_Project = 1

if 0 == len(arguments):
	print 'Usage: ./autotesting_init.py [PlatformName] [ProjectName]'
	exit(1)

print "*** DAVA Initializing autotesting"

print "platform.system: " + platform.system()



def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + arguments[index_Project])
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

if 2 == len(arguments):
    autotestingConfigSrcPath = os.path.realpath(currentDir + "/../Data/Config.h")
    autotestingConfigDestPath = os.path.realpath(frameworkDir + "/Sources/Internal/Autotesting/Config.h")
    if os.path.exists(autotestingConfigDestPath):    
        print "delete " + autotestingConfigDestPath
        os.remove(autotestingConfigDestPath)
    print "copy " + autotestingConfigSrcPath + " to " + autotestingConfigDestPath
    shutil.copy(autotestingConfigSrcPath, autotestingConfigDestPath)

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
scripts = ["/generate_id.py"]

if (platform.system() == "Darwin"):
    if (arguments[index_OS] == "iOS"):
        scripts.append("/runOnDevice.sh")
        scripts.append("/floatsign.sh")
        scripts.append("/packipa.sh")
        scripts.append("/transporter_chief.rb")
        scripts.append("/testRun.js")

print "copy iOS scripts from " + currentDir + " to " + autotestingSrcFolder
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

os.mkdir(autotestingDestFolder)

autotestingActionsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Actions")
autotestingActionsDestFolder = os.path.realpath(autotestingDestFolder + "/Actions")

autotestingTestsSrcFolder = os.path.realpath(autotestingSrcFolder + "/Tests")
autotestingTestsDestFolder = os.path.realpath(autotestingDestFolder + "/Tests")

print "copy " + autotestingActionsSrcFolder + " to " + autotestingActionsDestFolder
shutil.copytree(autotestingActionsSrcFolder, autotestingActionsDestFolder, ignore=ignored_svn_files)

print "copy " + autotestingTestsSrcFolder + " to " + autotestingTestsDestFolder
shutil.copytree(autotestingTestsSrcFolder, autotestingTestsDestFolder, ignore=ignored_svn_files)

os.chdir(autotestingSrcFolder)

params = ["python", "./generate_id.py", arguments[index_Project], autotestingDestFolder]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

os.chdir(currentDir)
   
print "*** DAVA Initialized autotesting"