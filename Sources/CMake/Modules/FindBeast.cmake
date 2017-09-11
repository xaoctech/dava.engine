if ( BEAST_FOUND )
    return ()
endif ()
set ( BEAST_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory( Beast  "${DAVA_MODULES_DIR}/Beast" )