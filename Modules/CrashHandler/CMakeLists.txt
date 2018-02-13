set(MODULE_NAME CrashHandler)
set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE Sources/*.h)
set(CPP_FILES_RECURSE Sources/*.cpp)

set(PLATFORMS_SUPPORTED LINUX)
set(DEFINITIONS "-D__CRASHHANDLER_ENABLED__")

set(INCLUDES Sources)
set(INCLUDES_PRIVATE
        "${DAVA_ENGINE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources"
        "${CMAKE_CURRENT_SOURCE_DIR}/Libs/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Libs/Include/breakpad"
)

set(STATIC_LIBRARIES_LINUX "${CMAKE_CURRENT_LIST_DIR}/Libs/Linux/libbreakpad_client.a")

setup_main_module()
