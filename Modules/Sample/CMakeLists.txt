
components_option( DAVA_TEST_COMPONENT )

set( NAME_MODULE Sample )                     # set name to module 

set( MODULE_TYPE STATIC )                     # set module type [INLINE, STATIC, DYNAMIC]

set( INCLUDES         Sources)                # set search path headers for all project
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )    # set search path headers inside module

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( MODULE_INITIALIZATION true )
set( NOT_USE_PARENT_DEFINITIONS true )

if( DAVA_TEST_COMPONENT )
    #message( STATUS "DAVA_TEST_COMPONENT  on")
else()
    #message( STATUS "DAVA_TEST_COMPONENT  off")
endif()

setup_main_module()                           # induce macros of module formation