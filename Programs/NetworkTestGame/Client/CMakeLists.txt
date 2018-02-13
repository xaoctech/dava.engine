cmake_minimum_required (VERSION 3.0)
cmake_minimum_required (VERSION 3.0)

project ( Client )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../../Sources/CMake/Modules/" )
include      ( CMake-common )

set ( DAVA_COMPONENTS DAVA_DISABLE_AUTOTESTS )

dava_add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
dava_add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
dava_add_definitions(-DDISABLE_NATIVE_WEBVIEW)
dava_add_definitions(-DUSER_VERSIONING_DEBUG_FEATURES)

find_dava_module( NetworkCore )
find_dava_module( Physics )
find_dava_module( NetworkPhysics )
find_dava_module( TestCharacterController )
find_package( DavaFramework REQUIRED COMPONENTS "${DAVA_COMPONENTS}" )

include_directories   ( "Sources" "${CMAKE_CURRENT_LIST_DIR}/../Shared/Sources")

define_source                  ( SOURCE "Sources" "${CMAKE_CURRENT_LIST_DIR}/../Shared/Sources")

set( MIX_APP_DATA "Data = ${DAVA_ROOT_DIR}/Programs/Data")

if( ANDROID )
	list( APPEND MIX_APP_DATA "Data = ${CMAKE_CURRENT_LIST_DIR}/../DataAndroid" )
else()
	list( APPEND MIX_APP_DATA "Data = ${CMAKE_CURRENT_LIST_DIR}/../Data" )
endif()

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/Platforms/Mac/Info.plist )
set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/Platforms/Ios/Info.plist )

#ios
if (IOS)
    set( CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "9KMD79CS7L" ) 
    set( CMAKE_XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "match Development net.wargaming.networktestgame" )
    set( CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "net.wargaming.networktestgame" )
    set( CMAKE_XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon" )
    set( CMAKE_XCODE_ATTRIBUTE_VERSIONING_SYSTEM "Apple Generic")
    set( CMAKE_XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1" )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/Platforms/Ios/Images.xcassets )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/Platforms/Ios/LaunchScreen.storyboard )
    file( COPY "${CMAKE_CURRENT_LIST_DIR}/Platforms/fastlane" DESTINATION ".")
endif()

if (MACOS)
    set( CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "net.wargaming.networktestgame.mac" )
    set( CMAKE_XCODE_ATTRIBUTE_VERSIONING_SYSTEM "Apple Generic" )
    set( CMAKE_XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1" )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/Platforms/Mac/MainMenu.xib )
    file( COPY "${CMAKE_CURRENT_LIST_DIR}/Platforms/fastlane" DESTINATION "." )
endif()

if (WIN32)
    set( EXECUTABLE_FLAG WIN32 )
endif()

setup_main_executable()
convert_graphics()


#remove executabled from Data
if (IOS)
ADD_CUSTOM_COMMAND( TARGET DATA_COPY_${PROJECT_NAME}  
                  COMMAND ${CMAKE_COMMAND} -E remove
                   ${MIX_APP_DIR}/Data/PVRTexToolCLI.exe
                   ${MIX_APP_DIR}/Data/PVRTexToolCLI
                )
endif()

