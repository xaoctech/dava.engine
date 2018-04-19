
# General
General info about ResourceSystem at dava.engine you can find at Docs/ResourceSystem.md


# Network Test Game


## Folders Structure

* Data/
    * 3d/ - generated folder for resources. 
* DataAndroid/ - temporary folder for android. Will be deleted after moving to ResourceSystem
* DataSource/
    * 3d/ - temporary folder for android. Will be deleted after moving to ResourceSystem
    * cache/ - folder with cached info. Used by ResourceSystem. Should be ignored by git
    * conf/ - configs for ResourceSystem. Project-level config should be added into git. Local-level config should be ignored by git.
    * meta/ - folder with resource id. Should be added into git.
    * sources/ - non converted sources. Only non-compressed and non-exported resources. All files 
    * master_config.txt - list of main resources that are required by game. ()


## How to build app with ResourceSystem

Enable USE\_RESOURCE\_SYSTEM option for downloading resources from ResourceSystem  
> cmake ... -DUSE\_RESOURCE\_SYSTEM=true

CMake will create build step "${PROJECT\_NAME}\_UPDATE\_RESOURCES" that will download resoures from server 


## How to work with resources

res-cmd-line/src/main.py is the tool to work with ResourceSystem. Use "help" option to learn more about tool

* Place resource file(*.sc2, *.tex) into DataSoure/sources/3d/ to put it into ResourceSystem. 
* Update master_config.txt with new path in case this file should be opened from code
* Don't place *.pvr and *.dds files into source folder. These files will be created on server
* Place resource file(*.sc2, *.tex) into DataSoure/3d/ if this folder exsits

