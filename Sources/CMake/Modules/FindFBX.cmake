if ( FBX_FOUND )
    return ()
endif ()
set ( FBX_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory( FBX  "${DAVA_MODULES_DIR}/FBX" )