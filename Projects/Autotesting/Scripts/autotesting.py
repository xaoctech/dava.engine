#!/usr/bin/env python2.6
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
import subprocess;

arguments = sys.argv[1:]

if len(arguments) != 4:
    print 'Usage: ./autotesting.py [PlatformName] [ProjectName] [TargetName] [ConfigurationName]'
    exit(1)


print "*** DAVA Starting autotesting"

print "platform.system: " + platform.system()
print sys.argv

platformName = arguments[0]
projectName = arguments[1]
targetName = arguments[2]
configurationName = arguments[3]

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + projectName)
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")

executableName = ""
executableBuildPath = ""
executableRunPath = ""
if (platform.system() == "Windows"):
    executableName = targetName + ".exe"
    print "executableName: " +executableName
    
    if(platformName == "Windows"):
        #TODO: Windows
        print "prepare to run " + executableName + " on Windows"
        
        executableBuildPath = os.path.realpath(projectDir + "/" + configurationName + "/" + executableName)
        executableRunPath = os.path.realpath(projectDir + "/" + executableName)
        
        if os.path.exists(executableRunPath):    
            print "delete " + executableRunPath
            os.remove(executableRunPath)
        print "copy " + executableBuildPath + " to " + executableRunPath
        shutil.copy(executableBuildPath, executableRunPath)
    
    else:
        print "Error: wrong OS " + platformName

elif (platform.system() == "Darwin"):
    executableName = targetName + ".app"
    print "executableName: " +executableName
    
    if (platformName == "iOS"):
        #TODO: iOS
        print "prepare to run " + executableName + " on iOS"
        #TODO: copy executable to Autotesting
        executableBuildPath = os.path.realpath(projectDir + "/build/" + configurationName + "-iphoneos/" + executableName)
        executableRunPath = os.path.realpath(autotestingSrcFolder + "/" + executableName)
        if os.path.exists(executableRunPath):    
            print "delete " + executableRunPath
            shutil.rmtree(executableRunPath)
        print "copy " + executableBuildPath + " to " + executableRunPath
        shutil.copytree(executableBuildPath, executableRunPath)
        
        # resign and deploy
        print "cd " + autotestingSrcFolder
        os.chdir(autotestingSrcFolder)
        
        #sh floatsign.sh $2.app $3 $2.ipa
        ipaName = targetName + ".ipa"
        
        params = ["sh", "./packipa.sh", executableName, ipaName]
        print "create " + ipaName
        
        print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params)
    
        # ./transporter_chief.rb $2.ipa
        print "deploy "+ ipaName +" on device"
        params = ["./transporter_chief.rb", ipaName]
        print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params)

    elif (platformName == "MacOS"):
        #TODO: MacOS
        print "prepare to run " + executableName + " on MacOS"
        executableBuildPath = os.path.realpath(projectDir + "/build/" + configurationName + "/" + executableName)
        executableRunPath = executableBuildPath
    else:
        print "Error: wrong OS " + platformName

else:
    print "Error: unsupported platform.system: " + platform.system()    

os.chdir(projectDir)

testsFolder = os.path.realpath(projectDir + "/Data/Autotesting/Tests")
testFiles = os.listdir(testsFolder)

for testFile in testFiles:
    print "current test file is: " + testFile
    
    if (platform.system() == "Windows"):
        if(platformName == "Windows"):
            #TODO: Windows
            print "run " + executableName + " on Windows"
            os.system("start /WAIT " + executableName)
        else:
            print "Error: wrong OS " + platformName
    elif (platform.system() == "Darwin"):
        if (platformName == "iOS"):
            #TODO: iOS
            print "run " + executableName + " on iOS"
            
            os.chdir(autotestingSrcFolder)
            
            #instruments -w $4 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$2" -e UIASCRIPT testRun.js
            params = ["sh", "./runOnDevice.sh", targetName]
            print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
            subprocess.call(params)
        
        elif (platformName == "MacOS"):
            #TODO: MacOS
            print "run " + executableRunPath + " on MacOS"
            os.system("open -W " + executableRunPath)
        
        else:
            print "Error: wrong OS " + platformName
    else:
        print "Error: unsupported platform.system: " + platform.system()

print "TODO: report"

print "TODO: cleanup"    
if os.path.exists(autotestingDestFolder):    
    print "Delete " + autotestingDestFolder
#shutil.rmtree(autotestingDestFolder)

if (platformName == "iOS"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
#shutil.rmtree(executableRunPath)
elif (platformName == "Windows"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
        os.remove(executableRunPath)
   
print "*** DAVA Finished autotesting"