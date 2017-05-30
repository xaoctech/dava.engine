
set( NAME_MODULE SpineModule )

set( MODULE_TYPE STATIC )

set( INCLUDES ${CMAKE_CURRENT_LIST_DIR}/Sources )

set( INCLUDES_PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Libs/include
                      ${CMAKE_CURRENT_LIST_DIR}/Sources
                      ${DAVA_ENGINE_DIR} )

set( MODULE_INITIALIZATION_CODE Sources/SpineModule.h )
#set( MODULE_INITIALIZATION_NAMESPACE Spine )

set( HPP_FILES_RECURSE *.h )
set( CPP_FILES_RECURSE *.cpp )

set( LIBRARIES_SPINE ${CMAKE_CURRENT_LIST_DIR}/Libs )

set( STATIC_LIBRARIES_ANDROID        ${LIBRARIES_SPINE}/Android/spine.a )

set( STATIC_LIBRARIES_IOS            ${LIBRARIES_SPINE}/Ios/spine.a )

set( STATIC_LIBRARIES_MACOS          ${LIBRARIES_SPINE}/Mac/spine.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${LIBRARIES_SPINE}/Win32/x86/Release/spine.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${LIBRARIES_SPINE}/Win32/x86/Debug/spined.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${LIBRARIES_SPINE}/Win32/x64/Release/spine.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${LIBRARIES_SPINE}/Win32/x64/Debug/spined.lib )

set( STATIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_SPINE}/Win10/$(Platform)/Release/spine.lib )

set( STATIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_SPINE}/Win10/$(Platform)/Debug/spined.lib )

set( EXCLUDE_FROM_ALL true )

setup_main_module()