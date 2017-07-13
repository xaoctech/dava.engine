if ( EDITOR_PHYSICS_FOUND )
    return ()
endif ()
set ( EDITOR_PHYSICS_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( EditorPhysics "${DAVA_MODULES_DIR}/EditorPhysics" )
