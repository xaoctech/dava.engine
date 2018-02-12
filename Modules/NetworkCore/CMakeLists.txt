set( MODULE_NAME NetworkCore )
set( MODULE_TYPE STATIC ) # INLINE, STATIC, DYNAMIC

set( HPP_FILES_RECURSE Sources/*.h Sources/*.hpp)
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES Sources )
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR}
                      ${DAVA_INCLUDE_DIR}
                      "${PhysicsRootDir}/Sources"
                      "${PhysicsRootDir}/Libs/include/"
                      "${PhysicsRootDir}/Libs/include/physx"
                      "${PhysicsRootDir}/Libs/include/PxShared")  

module_options( NETWORK_SERVER )

set( EXCLUDE_FROM_ALL true )
set( FIND_MODULE Physics )

set( LIBRARIES_NETWORK                   ${CMAKE_CURRENT_LIST_DIR}/Libs)
set( INCLUDES                            ${CMAKE_CURRENT_LIST_DIR}/Sources
                                         ${LIBRARIES_NETWORK}/include )

set( STATIC_LIBRARIES_LINUX              ${LIBRARIES_NETWORK}/linux/libenet.a)
set( STATIC_LIBRARIES_MACOS              ${LIBRARIES_NETWORK}/MacOS/libenet.a)
set( STATIC_LIBRARIES_IOS                ${LIBRARIES_NETWORK}/iOS/libenet.a)
set( STATIC_LIBRARIES_ANDROID            ${LIBRARIES_NETWORK}/Android/${ANDROID_NDK_ABI_NAME}/libenet.a)
set( STATIC_LIBRARIES_WIN32_RELEASE      ${LIBRARIES_NETWORK}/Win32/x86/Release/enet.lib)
set( STATIC_LIBRARIES_WIN32_DEBUG        ${LIBRARIES_NETWORK}/Win32/x86/Debug/enet.lib)
set( STATIC_LIBRARIES_WIN64_RELEASE      ${LIBRARIES_NETWORK}/Win32/x64/Release/enet.lib)
set( STATIC_LIBRARIES_WIN64_DEBUG        ${LIBRARIES_NETWORK}/Win32/x64/Debug/enet.lib)
set( STATIC_LIBRARIES_WINUAP_RELEASE     ${LIBRARIES_NETWORK}/win10/$(Platform)/Release/enet.lib)
set( STATIC_LIBRARIES_WINUAP_DEBUG       ${LIBRARIES_NETWORK}/win10/$(Platform)/Debug/enet.lib)

set( MODULE_INITIALIZATION true )

if( NETWORK_SERVER  )
    list( APPEND DEFINITIONS -DSERVER )
endif()

setup_main_module()
