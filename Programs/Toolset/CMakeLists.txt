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
include ( Toolset-common )


############ SINGLE TOOLS

add_tool_single     ( UnitTests      NO_UNITY_BUILD )
add_tool_single     ( TestBed         )
add_tool_single     ( PerfomanceTests )
add_tool_single     ( Launcher        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

if ( WIN32 )
    add_tool_single ( UWPRunner )
endif ()


############ PACKAGE TOOLS

add_tool_package     ( SceneViewer     DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( UIViewer        DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( ResourceEditor  DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )
add_tool_package     ( QuickEd         DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

add_tool_package     ( ResourceArchiver DEPENDS ResourceEditor )
add_tool_package     ( ResourcePacker   DEPENDS ResourceEditor )
add_tool_package     ( ResourcePatcher  DEPENDS ResourceEditor )

add_tool_package     ( AssetCacheServer )
add_tool_package     ( Dep )

add_tool_package     ( ImageUnpacker    )

if (NOT DEPLOY OR CACHE_CLIENT)    
    add_tool_package     ( AssetCacheClient )
endif ()

###

prepare_tools()


