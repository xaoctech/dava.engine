cmake_minimum_required (VERSION 3.0)

set(MODULE_NAME TerrainConverter)
set(MODULE_TYPE PLUGIN)

set(FIND_PACKAGE GMock QT5 DavaFramework)
set(FIND_MODULE DavaTools)

list( APPEND DAVA_COMPONENTS Sound )
list( APPEND DAVA_COMPONENTS DAVA_PLATFORM_QT )
list( APPEND DAVA_COMPONENTS DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME )
list( APPEND DAVA_COMPONENTS DAVA_DISABLE_AUTOTESTS )
list( APPEND DAVA_COMPONENTS DAVA_USE_RENDERSTATS )

set(PACKAGE_COMPONENTS_DavaFramework ${DAVA_COMPONENTS})

set(FIND_MODULE QtHelpers TArc QtTools REPlatform)

set(SOURCES_DIR "${CMAKE_CURRENT_LIST_DIR}/Sources/TerrainConverter")
set(LIBS_DIR "${CMAKE_CURRENT_LIST_DIR}/Libs")

set(INCLUDES_PRIVATE 
                     "${LIBS_DIR}/Includes"
                     "${LIBS_DIR}/Includes/zip"
                     "${SOURCES_DIR}"
                     "${SOURCES_DIR}/Private"
                     "${SOURCES_DIR}/Private/physics2"
                     "${DAVA_ENGINE_DIR}"
                     "${DAVA_THIRD_PARTY_ROOT_PATH}/zlib"
                     "${DAVA_THIRD_PARTY_ROOT_PATH}/bullet")

set(HPP_FILES_RECURSE Sources/TerrainConverter/*.hpp Sources/TerrainConverter/*.h)
set(CPP_FILES_RECURSE Sources/TerrainConverter/*.cpp Sources/TerrainConverter/*.c)

set(WIN_DEBUG_LIBS "${LIBS_DIR}/Win32/Debug/cstdmf.lib")
set(WIN_RELEASE_LIBS "${LIBS_DIR}/Win32/Release/cstdmf.lib")

set(MACOS_LIBS "${LIBS_DIR}/MacOS/libcstdmf_macosx.a"
                              "${LIBS_DIR}/MacOS/libminizip.a")

set(STATIC_LIBRARIES_WIN64_RELEASE "${WIN_RELEASE_LIBS}")
set(STATIC_LIBRARIES_WIN64_DEBUG "${WIN_DEBUG_LIBS}")

set(STATIC_LIBRARIES_MACOS_RELEASE "${MACOS_LIBS}")
set(STATIC_LIBRARIES_MACOS_DEBUG "${MACOS_LIBS}")

if (OUT_DIR)
  set(PLUGIN_OUT_DIR ${OUT_DIR})
endif()

setup_main_module()
