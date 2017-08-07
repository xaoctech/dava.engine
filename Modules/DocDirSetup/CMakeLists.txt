set( NAME_MODULE DocDirSetup )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/DocDirSetup/*.h )
set( CPP_FILES_RECURSE Sources/DocDirSetup/*.cpp )
set( CPP_FILES_APPLE Sources/DocDirSetup/*.mm  )

set( INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" )
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

setup_main_module()
