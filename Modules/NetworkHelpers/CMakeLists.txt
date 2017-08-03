set(NAME_MODULE NetworkHelpers)

set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE Sources/NetworkHelpers/*.h)
set(CPP_FILES_RECURSE Sources/NetworkHelpers/Private/*.cpp)

set(INCLUDES ${CMAKE_CURRENT_LIST_DIR} "Sources")
set(INCLUDES_PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR} ${DAVA_INCLUDE_DIR})
set(NOT_USE_PARENT_DEFINITIONS true)

setup_main_module()
