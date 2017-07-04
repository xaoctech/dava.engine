if ( LOGGER_SERVICE_FOUND )
    return ()
endif ()
set ( LOGGER_SERVICE_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "LoggerService" )

add_module_subdirectory ( LoggerService "${DAVA_MODULES_DIR}/LoggerService" )