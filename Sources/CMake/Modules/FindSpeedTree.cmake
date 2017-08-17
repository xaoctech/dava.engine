if ( SPEED_TREE_FOUND )
    return ()
endif ()
set ( SPEED_TREE_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( SpeedTree "${DAVA_MODULES_DIR}/SpeedTree" )
