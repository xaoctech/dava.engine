if ( MEMORY_PROFILER_SERVICE_FOUND )
    return ()
endif ()
set ( MEMORY_PROFILER_SERVICE_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "MemoryProfilerService" )

add_subdirectory ( "${DAVA_MODULES_DIR}/NetworkServices/MemoryProfilerService" ${CMAKE_CURRENT_BINARY_DIR}/NetworkServices/MemoryProfilerService )