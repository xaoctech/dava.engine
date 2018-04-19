# -*- coding: utf-8 -*-
import argparse
import os
import platform
import shutil
import subprocess
import sys

def getCurrentFolder():
    currentFile = os.path.realpath(__file__)
    currentFolder = os.path.dirname(currentFile)
    return currentFolder

def getArtifactsFolder(options):
    artifactsFolder = options.dir
    if artifactsFolder is None:
        currentFolder = getCurrentFolder()
        artifactsFolder = os.path.join(currentFolder, 'Data')
    return artifactsFolder

def cleanOldArtifacts(options):
    currentFolder = getCurrentFolder()

    artifactsFolder = getArtifactsFolder(options)
    clean = options.clean 
    if clean is True and os.path.isdir(artifactsFolder) is True:
        shutil.rmtree(artifactsFolder)

def downloadArtifacts( options, gpu ):
    currentFolder = getCurrentFolder()

    cmdToolFolder = os.path.join(currentFolder, '..', '..', '..', '..', '..', 'resourcesystem', 'res-cmd-line')
    os.chdir(cmdToolFolder)

    # create platformed pathnames
    platformName = platform.system()
    if platformName == 'Windows':
        python_path = os.path.join(cmdToolFolder, 'venv', 'Scripts', 'python.exe')
    elif platformName == 'Darwin':
        python_path = os.path.join(cmdToolFolder, 'venv', 'bin', 'python3')
    elif platformName == 'Linux':
        python_path = os.path.join(cmdToolFolder, 'venv', 'bin', 'python3')
    else:
        print ('Wrong platform: ', platformName)
        return 1
    python_path = os.path.abspath(python_path)

    # download artifacts
    cmdTool = os.path.abspath(os.path.join(cmdToolFolder, 'src', 'main.py'))

    artifactsFolder = getArtifactsFolder(options)

    arguments = [python_path, cmdTool, 'artifacts']
    arguments.append('--tags='+gpu)
    arguments.append('--dir='+artifactsFolder)
    config = options.config
    if config is not None:
        arguments.append('--config='+config)

    if platformName == 'Windows':
        return subprocess.check_call(arguments)
    elif platformName == 'Darwin':
        return subprocess.check_call(arguments, env={"LC_ALL": "en_US.utf-8"})
    elif platformName == 'Linux':
        return subprocess.check_call(arguments, env={"LC_ALL": "en_US.utf-8"})

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--gpu', default='dx11', help='Select one GPU from list: PowerVR_iOS, PowerVR_Android, tegra, adreno, mali, dx11' )
    parser.add_argument( '--clean', default=False, help='Clean artifacts folder', action='store_true' )
    parser.add_argument( '--dir', help='Path to artifacts folder' )
    parser.add_argument( '--config', default='master_config.txt', help='Path to main config' )

    options = parser.parse_args()
    cleanOldArtifacts(options)

    allGPUs = ['PowerVR_iOS', 'PowerVR_Android', 'tegra', 'adreno', 'mali', 'dx11']
    gpu = options.gpu
    if gpu in allGPUs:
        ret = downloadArtifacts(options, gpu)
        sys.exit(ret)
    else:
        print ('Wrong gpu: ', gpu, '. Could be one from: ', str(allGPUs))
        sys.exit(1)

if __name__ == '__main__':
    main()

