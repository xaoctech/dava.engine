if (NETWORK_PHYSICS_FOUND)
    return ()
endif ()

set (NETWORK_PHYSICS_FOUND true)

include (GlobalVariables)

add_module_subdirectory( NetworkPhysics  "${DAVA_MODULES_DIR}/NetworkPhysics" )
