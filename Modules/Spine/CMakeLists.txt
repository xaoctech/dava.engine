
set( NAME_MODULE Spine )

set( MODULE_TYPE STATIC )

set( INCLUDES ${CMAKE_CURRENT_LIST_DIR}/Sources )

set( INCLUDES_PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Libs/include
                      ${CMAKE_CURRENT_LIST_DIR}/Sources
                      ${DAVA_ENGINE_DIR} )

#set( MODULE_INITIALIZATION_CODE SampleModule.h ) # set hider where found module
#set( MODULE_INITIALIZATION_NAMESPACE Test )      # set module namespace

set( HPP_FILES Sources/UI/Spine/*.h )
set( CPP_FILES Sources/UI/Spine/*.cpp )

set( LIBRARIES_SPINE ${CMAKE_CURRENT_LIST_DIR}/Libs/lib )

set( STATIC_LIBRARIES_ANDROID        ${LIBRARIES_FMOD}/android/spine.a )

set( STATIC_LIBRARIES_IOS            ${LIBRARIES_FMOD}/ios/spine.a )

set( STATIC_LIBRARIES_MACOS          ${LIBRARIES_FMOD}/mac/spine.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${LIBRARIES_FMOD}/win/x86/Release/spine.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${LIBRARIES_FMOD}/win/x86/Debug/spined.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${LIBRARIES_FMOD}/win/x64/Release/spine.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${LIBRARIES_FMOD}/win/x64/Debug/spined.lib )

set( STATIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_FMOD}/win10/$(Platform)/Release/spine.lib )

set( STATIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_FMOD}/win10/$(Platform)/Debug/spined.lib )

#set( EXCLUDE_FROM_ALL true )

setup_main_module()