if ( LOGGER_SERVICE_FOUND )
    return ()
endif ()
set ( LOGGER_SERVICE_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "LoggerService" )

add_subdirectory ( "${DAVA_MODULES_DIR}/NetworkServices/LoggerService" ${CMAKE_CURRENT_BINARY_DIR}/NetworkServices/LoggerService )