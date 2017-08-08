if ( NETWORK_HELPERS_FOUND )
    return ()
endif ()
set ( NETWORK_HELPERS_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( NetworkHelpers "${DAVA_MODULES_DIR}/NetworkHelpers" )
