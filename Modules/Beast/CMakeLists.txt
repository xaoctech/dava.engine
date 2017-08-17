set( NAME_MODULE Beast )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set(BeastRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(INCLUDES "${BeastRootDir}/Sources" 
             "${BeastRootDir}/Thirdparty/include" 
             "${DAVA_ROOT_DIR}/Programs" #tmp
             )


set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS -D__DAVAENGINE_BEAST__ )

set( STATIC_LIBRARIES_WIN32 "${BeastRootDir}/Thirdparty/lib/beast32.lib" )
set( STATIC_LIBRARIES_WIN64 "${BeastRootDir}/Thirdparty/lib/beast64.lib" )
set( DYNAMIC_LIBRARIES_WIN32 "${BeastRootDir}/Thirdparty/bin/beast32.dll" )
set( DYNAMIC_LIBRARIES_WIN64 "${BeastRootDir}/Thirdparty/bin/beast64.dll" )


setup_main_module()
