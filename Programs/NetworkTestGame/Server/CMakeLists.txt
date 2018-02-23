cmake_minimum_required (VERSION 3.0)

project ( Server )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../../Sources/CMake/Modules/" )
include      ( CMake-common )

set ( DAVA_COMPONENTS DAVA_DISABLE_AUTOTESTS )

dava_add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
dava_add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
dava_add_definitions(-DDISABLE_NATIVE_WEBVIEW)

if (WITH_BREAKPAD)
    find_dava_module( CrashHandler )
endif()
find_dava_module( NetworkCore OPTIONS NETWORK_SERVER )
find_dava_module( Physics )
find_dava_module( NetworkPhysics )
#find_dava_module( PhysicsDebug )
find_dava_module( TestCharacterController )
find_package( DavaFramework REQUIRED COMPONENTS "${DAVA_COMPONENTS}" )

include_directories   ( "Sources"  "${CMAKE_CURRENT_LIST_DIR}/../Shared/Sources")

define_source                  ( SOURCE "Sources"  "${CMAKE_CURRENT_LIST_DIR}/../Shared/Sources")

set( MIX_APP_DATA               "Data = ${DAVA_ROOT_DIR}/Programs/Data"
                                "Data = ${CMAKE_CURRENT_LIST_DIR}/../Data" )

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/Platforms/Mac/Info.plist )

if (MACOS)
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/Platforms/Mac/MainMenu.xib )
endif()

if (WIN32)
    set( EXECUTABLE_FLAG WIN32 )
endif()

setup_main_executable()
convert_graphics()
