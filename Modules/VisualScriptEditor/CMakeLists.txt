set( MODULE_NAME VisualScriptEditor )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/VisualScriptEditor/*.h   Thirdparty/*.hpp)
set( CPP_FILES_RECURSE Sources/VisualScriptEditor/Private/*.cpp Thirdparty/*.cpp)

# setup module initialization
set( CPP_FILES_EXECUTE Sources/VisualScriptEditor/InitVisualScriptEditor.cpp )

set( INCLUDES Sources )
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR} Thirdparty/include)

# qt resources
set(QT_RES_FILES "Resources/*.qrc" "Thirdparty/resources/*.qrc")

# ignore list for qt resources
set( UNITY_IGNORE_LIST              "qrc_VisualScriptEditorResources.cpp" )
#				    "qrc_neresources.cpp" )

# set private defines
set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE 	  -D__DAVAENGINE_QT__ )

# find modules
set( FIND_PACKAGE QT5)
set( FIND_MODULE VisualScript)

# setup node editor thirdparty
SET(DEFINITIONS -DNODE_EDITOR_STATIC)


setup_main_module()
