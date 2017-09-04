set( NAME_MODULE NetworkCore )
set( MODULE_TYPE STATIC ) # INLINE, STATIC, DYNAMIC

set(HPP_FILES_RECURSE Sources/*.h)
set(CPP_FILES_RECURSE Sources/*.cpp)

set( NetworkCoreRootDir "${CMAKE_CURRENT_SOURCE_DIR}" )

set( INCLUDES "${NetworkCoreRootDir}/Sources" )
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR}
                      ${DAVA_INCLUDE_DIR} )

set( MODULE_INITIALIZATION_CODE Sources/NetworkCore/NetworkCoreModule.h )

setup_main_module()
