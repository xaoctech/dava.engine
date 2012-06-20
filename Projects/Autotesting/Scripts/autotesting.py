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

print "*** DAVA Starting autotesting"

print "platform.system: " + platform.system()
print sys.argv

index_OS = 1;
index_Project = 2;
index_Target = 3;
index_Configuration = 4;
index_Certificate = 5;
index_Device = 6;

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + sys.argv[index_Project])
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")

executableName = ""
executableBuildPath = ""
executableRunPath = ""
if (platform.system() == "Windows"):
    executableName = sys.argv[index_Target] + ".exe"
    print "executableName: " +executableName
    
    if(sys.argv[index_OS] == "Windows"):
        #TODO: Windows
        print "prepare to run " + executableName + " on Windows"
        
        executableBuildPath = os.path.realpath(projectDir + "/" + sys.argv[index_Configuration] + "/" + executableName)
        executableRunPath = os.path.realpath(projectDir + "/" + executableName)
        
        if os.path.exists(executableRunPath):    
            print "delete " + executableRunPath
            os.remove(executableRunPath)
        print "copy " + executableBuildPath + " to " + executableRunPath
        shutil.copy(executableBuildPath, executableRunPath)
    
    else:
        print "Error: wrong OS " + sys.argv[index_OS]

elif (platform.system() == "Darwin"):
    executableName = sys.argv[index_Target] + ".app"
    print "executableName: " +executableName
    
    if (sys.argv[index_OS] == "iOS"):
        #TODO: iOS
        print "prepare to run " + executableName + " on iOS"
        #TODO: copy executable to Autotesting
        executableBuildPath = os.path.realpath(projectDir + "/build/" + sys.argv[index_Configuration] + "-iphoneos/" + executableName)
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
        ipaName = sys.argv[index_Target] + ".ipa"
        #params = ["sh", "./floatsign.sh", executableName, sys.argv[index_Certificate], ipaName]
        #print "sign with " + sys.argv[index_Certificate] + " and create " + ipaName
        
        params = ["sh", "./packipa.sh", executableName, ipaName]
        print "create " + ipaName
        
        print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params)
        
        # ./transporter_chief.rb $2.ipa
        print "deploy "+ ipaName +" on device"
        params = ["./transporter_chief.rb", ipaName]
        print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params)

    elif (sys.argv[index_OS] == "MacOS"):
        #TODO: MacOS
        print "prepare to run " + executableName + " on MacOS"
        executableBuildPath = os.path.realpath(projectDir + "/build/" + sys.argv[index_Configuration] + "/" + executableName)
        executableRunPath = executableBuildPath
    else:
        print "Error: wrong OS " + sys.argv[index_OS]

else:
    print "Error: unsupported platform.system: " + platform.system()    

os.chdir(projectDir)

testsFolder = os.path.realpath(projectDir + "/Data/Autotesting/Tests")
testFiles = os.listdir(testsFolder)

for testFile in testFiles:
    print "current test file is: " + testFile
    
    if (platform.system() == "Windows"):
        if(sys.argv[index_OS] == "Windows"):
            #TODO: Windows
            print "run " + executableName + " on Windows"
            os.system("start /WAIT " + executableName)
        else:
            print "Error: wrong OS " + sys.argv[index_OS]
    elif (platform.system() == "Darwin"):
        if (sys.argv[index_OS] == "iOS"):
            #TODO: iOS
            print "run " + executableName + " on iOS"
            
            os.chdir(autotestingSrcFolder)
            
            #instruments -w $4 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$2" -e UIASCRIPT testRun.js
            params = ["sh", "./runOnDevice.sh", sys.argv[index_Target], sys.argv[index_Device]]
            print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
            subprocess.call(params)
        
        elif (sys.argv[index_OS] == "MacOS"):
            #TODO: MacOS
            print "run " + executableRunPath + " on MacOS"
            os.system("open -W " + executableRunPath)
        
        else:
            print "Error: wrong OS " + sys.argv[index_OS]
    else:
        print "Error: unsupported platform.system: " + platform.system()

print "TODO: report"

print "TODO: cleanup"    
if os.path.exists(autotestingDestFolder):    
    print "Delete " + autotestingDestFolder
#shutil.rmtree(autotestingDestFolder)

if (sys.argv[index_OS] == "iOS"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
#shutil.rmtree(executableRunPath)
elif (sys.argv[index_OS] == "Windows"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
        os.remove(executableRunPath)
   
print "*** DAVA Finished autotesting"