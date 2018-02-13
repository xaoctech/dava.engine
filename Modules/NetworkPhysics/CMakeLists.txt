set(MODULE_NAME NetworkPhysics )

set(MODULE_TYPE STATIC)

set(DEFINITIONS "-D__DAVAENGINE_PHYSICS_ENABLED__")

set(HPP_FILES_RECURSE Sources/NetworkPhysics/*.h)
set(CPP_FILES_RECURSE Sources/NetworkPhysics/*.cpp)

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" )
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(MODULE_INITIALIZATION true )

setup_main_module()
