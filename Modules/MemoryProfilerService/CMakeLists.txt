set(NAME_MODULE MemoryProfilerService)

set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp *.mm)

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" "Sources/MemoryProfilerService")

set(INCLUDES_PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR})

set(FIND_PACKAGE NetworkHelpers)

setup_main_module()