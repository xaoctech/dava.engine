if ( DOC_DIR_SETUP_FOUND )
    return ()
endif ()
set ( DOC_DIR_SETUP_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( DocDirSetup "${DAVA_MODULES_DIR}/DocDirSetup" )
