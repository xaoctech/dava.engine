if ( MEMORY_PROFILER_SERVICE_FOUND )
    return ()
endif ()
set ( MEMORY_PROFILER_SERVICE_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "MemoryProfilerService" )

add_module_subdirectory ( MemoryProfilerService "${DAVA_MODULES_DIR}/MemoryProfilerService" )