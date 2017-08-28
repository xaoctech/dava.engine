set( NAME_MODULE EditorPhysics )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/EditorPhysics/*.h  )
set( CPP_FILES_RECURSE Sources/EditorPhysics/*.cpp )

set( CPP_FILES_EXECUTE Sources/EditorPhysics/InitEditorPhysics.cpp )

set( INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" )
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )

setup_main_module()
