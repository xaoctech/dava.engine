# Project Structure

## Basic files/folders naming requirements
- Files and folders are named using CamelCase. Some exceptions may apply (see below).
- File or folder related to a particular platform must be named using the following convention: FileName<Platform>. For example: `DeviceInfoWin32.cpp`. `Platform` can be on of the following:
  - Android
  - Mac
  - Ios
  - Win32
  - Win10
  - Linux

  - Posix
  - Windows
  - Apple

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
    |- Libs/
    |  |- Build/
    |  |- Include/
    |  |- Lib/
    |- ...

#### Libs/Build
    |- ...
    |- Build/
    |  |- liba/
    |  |  |- build.py
    |  |- libb/
    |  |  |- build.py
    |  |- ...
    |  |- libz/
    |  |  |- build.py
    |  |- build.py
    |- ...
    
#### Libs/Include
    |- ...
    |- Include/
    |  |- liba/
    |  |  |- a.h
    |  |- libb/
    |  |  |- b.h
    |  |- ...
    |  |- libz/
    |  |  |- z.h
    |- ...
    
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
    |  |  |- Platforms/
    |  |  |- Sources/
    |  |  |  |- Private/
    |  |  |  |- ModuleName.h
    |  |  |- README.md
    |  |- ...
    |- ...

#### Modules/X/Platforms/Android
    |- ...
    |- ModuleX
    |  |- ...
    |  |- Platforms/
    |  |  |- Android
    |  |  |  |- libs/
    |  |  |  |- src/
    |  |  |  |  |- main
    |  |  |  |  |  |- java
    |  |  |  |  |  |  |- com
    |  |  |  |  |  |  |  |- dava
    |  |  |  |  |  |  |  |  |- modules
    |  |  |  |  |  |  |  |  |  |- ModuleX
    |  |  |  |  |  |  |  |  |  |  | - ModuleX.java
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

## Appendix A: Full folders structure
    .
    |- Bin/
    |  |- bin1.exe
    |  |- bin1
    |  |- ...
    |  |- binN.exe
    |  |- binN
    |- Docs/
    |  |- Doxygen/
    |  |  |- doxyfile
    |  |- Codestyle.md
    |  |- FAQ.md
    |  |- ...
    |  |- Structure.md
    |- Libs/
    |  |- Build
    |  |  |- liba/
    |  |  |  |- build.py
    |  |  |- libb/
    |  |  |  |- build.py
    |  |  |- ...
    |  |  |- libz/
    |  |  |  |- build.py
    |  |  |- build.py
    |  |- Include
    |  |  |- liba/
    |  |  |  |- a.h
    |  |  |- libb/
    |  |  |  |- b.h
    |  |  |- ...
    |  |  |- libz/
    |  |  |  |- z.h
    |  |- Lib
    |- Modules/
    |  |- ModuleX/
    |  |  |- Docs/
    |  |  |- Libs/
    |  |  |- Platforms/
    |  |  |  |- Android
    |  |  |  |  |- libs/
    |  |  |  |  |- src/
    |  |  |  |  |  |- main
    |  |  |  |  |  |  |- java
    |  |  |  |  |  |  |  |- com
    |  |  |  |  |  |  |  |  |- dava
    |  |  |  |  |  |  |  |  |  |- modules
    |  |  |  |  |  |  |  |  |  |  |- ModuleX
    |  |  |  |  |  |  |  |  |  |  |  | - ModuleX.java
    |  |  |  |  |- AndroidManifest.xml
    |  |  |  |  |- build.gradle
    |  |  |  | - Ios
    |  |  |  |  | - Entitlements.plist
    |  |  |  |  | - ModuleX.entitlements
    |  |  |  |  | - ModuleX.plist
    |  |  |  |  | - ModuleX-Info.plist
    |  |  |  | - ...
    |  |  |- Sources/
    |  |  |  |- Private/
    |  |  |  |  | - ModuleXImpl.cpp
    |  |  |  |  | - ...
    |  |  |  |- ModuleX.h
    |  |  |- README.md
    |  |- ...
    |- Programs/
    |  |- ProgramA/
    |  |- ...
    |  |- ProgramZ/
    |  |  |- ...
    |- Sources/
    |- LICENSE
    |- README.md
