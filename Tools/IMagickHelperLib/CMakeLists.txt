cmake_minimum_required ( VERSION 3.0 )

project      ( IMagickHelper )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common         )
include      ( FindDavaFramework    )
include      ( GenerateExportHeader )

set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

add_definitions ( 
    -DSTATIC_MAGICK 
    -D_MAGICKLIB_
    -D_CONSOLE
    -D_SCL_SECURE_NO_WARNING
    -D_CRT_SECURE_NO_DEPRECATE 
    -D_CRT_NON_CONFORMING_SWPRINTFS
)

set ( LIBRARIES_DIR  "D:/AddSources" )

if( MACOS ) 
    set( IMAGE_MAGICK ImageMagick-6.7.4 )

elseif( WIN32 )
    set( IMAGE_MAGICK ImageMagick-6.8.9 )

    set ( IMAGE_MAGICK_INCLUDES_PATH  
    "${LIBRARIES_DIR}/${IMAGE_MAGICK}"
    "${LIBRARIES_DIR}/${IMAGE_MAGICK}/Magick++/lib" )

    set ( IMAGE_MAGICK_LIBRARIES_PATH  
    "${LIBRARIES_DIR}/${IMAGE_MAGICK}/VisualMagick/lib" )  

    file ( GLOB IMAGE_MAGICK_LIBRARIES_RELEASE  "${IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
    file ( GLOB IMAGE_MAGICK_LIBRARIES_DEBUG    "${IMAGE_MAGICK_LIBRARIES_PATH}/CORE_DB_*.lib" )


endif  ()

include_directories (.)
include_directories ( ${IMAGE_MAGICK_INCLUDES_PATH} ) 
include_directories ( ${DAVA_INCLUDE_DIR} ) 

file ( GLOB LIB_SOURCES "*.h" "*.cpp" )

add_library( ${PROJECT_NAME} SHARED  ${LIB_SOURCES} )

#generate_export_header( ${PROJECT_NAME} )
#file( COPY  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h DESTINATION  ${CMAKE_BINARY_DIR} )
 
set_property( TARGET ${PROJECT_NAME} PROPERTY FOLDER "libraries")

if ( MSVC )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"")    

endif()

target_link_libraries( ${PROJECT_NAME} ${LIBRARIES}  ${DAVA_LIBRARY} )

foreach ( FILE ${IMAGE_MAGICK_LIBRARIES_DEBUG} ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${IMAGE_MAGICK_LIBRARIES_RELEASE} ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()


