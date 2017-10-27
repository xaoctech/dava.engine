## Command line parameters
#
# -DCHECK_DEPENDS_FOLDERS = generate only tools in which there were changes [true/false]
# -DTEAMCITY_URL   = teamcity url   
# -DSTASH_URL      = stash url
# -DTEAMCITY_LOGIN = teamcity user login
# -DTEAMCITY_PASS  = teamcity user password
# -DSTASH_LOGIN    = stash user login
# -DSTASH_PASS     = stash user password
#
# -DIMAGE_UNPACKER = add to toolset ImageUnpacker [true/false]
# -DCACHE_CLIENT   = add to toolset AssetCacheClient
#
# -DDEPLOY         = deploy build [true/false]
# -DUNITY_BUILD    = unity build  [true/false]
#
##

cmake_minimum_required (VERSION 3.0)

project( ToolSet )

set     ( CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include ( Toolset-сommon )


############ SINGLE TOOLS

add_tool_single     ( UnitTests         NO_UNITY_BUILD )

add_tool_single     ( TestBed           PLATFORMS IGNORE_LINUX  )

add_tool_single     ( PerfomanceTests   PLATFORMS IGNORE_LINUX )

add_tool_single     ( Launcher          PLATFORMS MACOS WIN        
                                        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_single     ( UWPRunner         PLATFORMS WIN )
 

############ PACKAGE TOOLS

add_tool_package     ( SceneViewer      PLATFORMS IGNORE_LINUX 
                                        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( UIViewer         PLATFORMS MACOS WIN
                                        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( ResourceEditor   PLATFORMS MACOS WIN
                                        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( QuickEd          PLATFORMS MACOS WIN
                                        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( ResourceArchiver PLATFORMS MACOS WIN
                                        DEPENDS ResourceEditor )

add_tool_package     ( ResourcePacker   PLATFORMS MACOS WIN
                                        DEPENDS ResourceEditor )

add_tool_package     ( ResourcePatcher  PLATFORMS MACOS WIN
                                        DEPENDS ResourceEditor )

add_tool_package     ( AssetCacheServer PLATFORMS MACOS WIN )


add_tool_package     ( ImageUnpacker    PLATFORMS MACOS WIN )

if (NOT DEPLOY OR CACHE_CLIENT)    
    add_tool_package ( AssetCacheClient PLATFORMS MACOS WIN )
endif ()

###
#deprecated
#add_tool_package    ( Dep              PLATFORMS MACOS WIN )


prepare_tools()


