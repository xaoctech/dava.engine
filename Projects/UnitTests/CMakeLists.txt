cmake_minimum_required (VERSION 2.8)
project      ( UnitTest )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

find_package ( DavaFramework REQUIRED )

include      ( CMake-common )

define_source_folders ( GLOB_FOLDER "Classes" "iOSSpecific" GLOB_ERASE_FOLDERS "_build" )

include_directories   ( "Classes" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

add_definitions( -D_CRT_SECURE_NO_DEPRECATE )

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDAVA_DEBUG" )

if( IOS )

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/iOSSpecific/*.xib" )

  	list( APPEND RESOURCES_LIST ${XIB_LIST} )
    list( APPEND RESOURCES_LIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist )

endif()

generate_source_groups_project ()
                               
add_executable  ( UnitTest MACOSX_BUNDLE
    ${PROJECT_SOURCE_FILES} 
    ${RESOURCES_LIST}
)

target_link_libraries( UnitTest
    ${DAVA_LIBRARY}
    ${LIBRARIES}
)

if( IOS )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist 
                            RESOURCE "${XIB_LIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                          )

    get_target_property ( TARGET_LOC ${PROJECT_NAME} LOCATION )
    string ( REGEX REPLACE /Contents/MacOS/${PROJECT_NAME} "" TARGET_LOC ${TARGET_LOC} )
    add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Data ${TARGET_LOC}/Data
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/TemplateProjectiPhone-Info.plist ${CMAKE_BINARY_DIR}/CMakeFiles/UnitTest.dir/Info.plist
    )

endif()

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( UnitTest debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( UnitTest optimized ${FILE} )
endforeach ()





