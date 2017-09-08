set( MODULE_NAME EditorPhysics )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/EditorPhysics/*.h  )
set( CPP_FILES_RECURSE Sources/EditorPhysics/*.cpp )

set( CPP_FILES_EXECUTE Sources/EditorPhysics/InitEditorPhysics.cpp )

set( INCLUDES Sources )
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set( FIND_MODULE Physics)

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE           -D__DAVAENGINE_QT__ )

setup_main_module()
