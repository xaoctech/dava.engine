cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set( NAME_MODULE TerrainConverter )                         # set name to plugin 

set( MODULE_TYPE PLUGIN )                               # set module type [INLINE, STATIC, PLUGIN]

list(APPEND DAVA_COMPONENTS "PhysicsModule")
list(APPEND DAVA_COMPONENTS "PhysicsDebugModule")
list(APPEND DAVA_COMPONENTS DAVA_PLATFORM_QT)
list(APPEND DAVA_COMPONENTS DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME)
list(APPEND DAVA_COMPONENTS DAVA_DISABLE_AUTOTESTS)
list(APPEND DAVA_COMPONENTS DAVA_USE_RENDERSTATS)

find_package( DavaTools REQUIRED )
find_package( QT5 REQUIRED )
find_package( GMock REQUIRED )
find_package( QtHelpers REQUIRED )
find_package( DavaFramework REQUIRED COMPONENTS ${DAVA_COMPONENTS})
find_package( QtTools REQUIRED )
find_package( TArc REQUIRED )

set( INCLUDES "." )                                     # set search path headers for all project
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )              # set search path headers inside module

set( HPP_FILES_RECURSE *.h )                                    # 
set( CPP_FILES_RECURSE *.cpp )                          # set where are sources

set( PLUGIN_OUT_DIR "d:/dev/dava.framework/Programs/ResourceEditor/_build/Debug/PlugIns/"  ) #plugin out dir
setup_main_module()                                     # induce macros of module formation
