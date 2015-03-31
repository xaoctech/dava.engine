#!/usr/bin/env python
#
#  convert_3d.py
#  DAVA SDK
#
#  Created by Yury Danilov on 6/14/12.
#  Copyright (c) 2012 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys
import os.path
import pickle
import zlib
import string
import sys
import platform
import shutil
import datetime

def unix_path(path):
    index = path.find(':')
    if ( index != -1):
        path = path[index + 1:]
    path = path.replace('\\', '/')
    return path

def log_time():
    print "convert_3d_FX.py time: " + str(datetime.datetime.now()).split('.')[0]

def main_convert_3d(): # it's function for external usage
    log_time()
    fx = ['FX', 'FXLow', 'FXHigh']

    currentDir = os.getcwd(); 
    dataDir =  os.path.realpath(currentDir + "/../Data/3d/")
    dataSourceDir = os.path.realpath(currentDir + "/3d/")
    qualityConfigPath = os.path.realpath(currentDir + "/../Data/quality.yaml")
    print "*** convert_3d_FX.py Launching command line 3D packer - data directory:" + dataDir
    
    gpuParams = []
    gpuParam = 'PowerVR_iOS'
    
    if 1 < len(sys.argv):
        gpuParam = sys.argv[1]

    if gpuParam != 'png':
        gpuParams.append('-gpu')
        gpuParams.append(gpuParam)
        print 'converting -gpu ' + gpuParam
        
    if 2 < len(sys.argv):
        if sys.argv[2] != '-teamcity':
            qualityParamIndex = 2
        else:
            qualityParamIndex = 3
            gpuParams.append('-teamcity')
            print 'converting with teamcity logs'
        if qualityParamIndex < len(sys.argv):
            qualityParam = sys.argv[qualityParamIndex]
            gpuParams.append('-quality')
            gpuParams.append(qualityParam)
            print 'converting with -quality ' + qualityParam
        else:
            print 'converting with default quality'
    else:
        print 'converting with default quality'
    
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
    executables = { "Darwin": "./ResourceEditor.app/Contents/MacOS/ResourceEditor", "Windows": "./ResourceEditor", "Microsoft": "./ResourceEditor" }
                                 
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

    #executable = './ResourceEditor.app/Contents/MacOS/TemplateProjectMacOS'
    #executable = "./ResourceEditor"
    os.chdir("../ResourceEditor/dava.framework/Tools/ResourceEditor/");

    executable = executables[platform.system()];
    print "ex: " + executable;

    # clean folders
    for i in range(0, len(fx)):
        os.spawnv(os.P_WAIT, executable, [executable, '-cleanfolder', '-folder', dataDir +"/" + fx[i], '-forceclose']);

    # export tanks
    for i in range(0, len(fx)):
        log_time()
        print "processdir " + fx[i]
        params = [executable, '-sceneexporter', '-export', '-indir', dataSourceDir, '-outdir', dataDir, '-processdir', fx[i], '-qualitycfgpath', qualityConfigPath, '-forceclose']
        params.extend(gpuParams)
        print "[%s]" % ", ".join(map(str, params))
        os.spawnv(os.P_WAIT, executable, params)
        log_time()
    
    # come back
    os.chdir(currentDir)
    log_time()

if __name__ == '__main__':
    main_convert_3d()