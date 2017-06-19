# Project Structure

An overview how the DAVAEngine project is structured.

## Basic naming requirements
- All top-level folders are named with a capital letter
- We support a several platforms. If the directory or file is related to a particular platform, then this relation must be expressed in the file or directory naming in the following form: `[Prefix]<Platform>`, where `Prefix` is any user-defined world (can be empty) and `Platform` is one of the following:
  - Android
  - Mac
  - Ios
  - Win32
  - Win10
- We support several processor architectures. If the directory is related to a particular architecture, then this relation must be expressed in directory naming in the following form: `[Prefix]<architecture>`, where `Prefix` is any user-defined world (can be empty) and `architecture` is one of the following:
  - x86
  - x64
  - arm
  - arm64
- More..

## Top-level directory layout
rfregerg

    .
    |- Bin/
    |- Docs/
    |- Libs/
    |- Modules/
    |- Programs/
    |- Sources/
    |- LICENSE
    |- README.md

## Bin

## Doc
It is used to store some reference data about the project, such documentation, best practive guids, codestyle, etc.
    
    |- ...
    |- Docs/
    |  |- Doxygen/
    |  |- Codestyle.md
    |  |- Structure.md
    |  |- FAQ.md
    |  |- ...
    |- ...

## Libs
    |- ...
    |- Libs
    |  |- Build
    |  |- Include
    |  |- Lib
    |- ...

#### Libs/Build
#### Libs/Include
#### Libs/Lib

## Sources

#### Sources/Internal
#### Sources/External

## Modules
    |- ...
    |- Modules
    |  |- ModuleA
    |  |- ModuleB
    |  |- ...
    |  |- ModuleZ
    |- ...

#### Modules/X

    |- ...
    |- Modules/
    |  |- ModuleX/
    |  |  |- Docs/
    |  |  |- Libs/
    |  |  |- Platform/
    |  |  |- Sources/
    |  |  |  |- Private/
    |  |  |  |- ModuleName.h
    |  |  |- README.md
    |  |- ...
    |- ...

#### Modules/X/Platform/Android
    |- ...
    |- ModuleX
    |  |- ...
    |  |- Platforms/
    |  |  |- Android
    |  |  |  |- libs/
    |  |  |  |- src/
    |  |  |  |- AndroidManifest.xml
    |  |  |  |- build.gradle
    |  |  |- 

#### Modules/X/Platform/Ios

## Programs
    |- ...
    |- Programs
    |  |- ProgramA
    |  |- ProgramB
    |  |- ...
    |  |- ProgramZ
    |- ...

#### Programs/X

## User Projects

