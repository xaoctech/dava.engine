cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( Launcher )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )

include               ( CMake-common )

find_package          ( QT5 REQUIRED  COMPONENTS Network )
find_package          ( QtHelpers  REQUIRED )

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)             
include_directories   ( "Classes" )

if ( WIN32 )
    set( LIB_RELEASE  "msvcrt.lib" )
    set( LIB_DEBUG    "msvcrtd.lib" )
endif()

file                  ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui           ( UI_APP_HDRS  ${UI_LIST} )
                   
file                  ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources     ( RCC_APP_SRCS  ${RCC_LIST} )

set                   ( Generated  ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source ( SOURCE    "Classes" "DataQt" )

if ( MACOS )
	set( APP_DATA		  ${DAVA_BIN_DIR}/7za)
endif ()

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/icon.icns  )

set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS}  Launcher.rc )

set( LIBRARIES_RELEASE    ${LIB_RELEASE} )
set( LIBRARIES_DEBUG      ${LIB_DEBUG} )
ASSERT(QT_ACTUAL_PATH "Qt path was not resolved")
set( ADDED_BINARY_DIR     ${QT_ACTUAL_PATH}/bin )

setup_main_executable()

if( WIN32 )
ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME} POST_BUILD
#   OutDir is a MSVC macro variable
    COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BIN_DIR}/7z.exe $(OutDir)
    COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BIN_DIR}/7z.dll $(OutDir)
)    
endif ()

if (WIN32 AND DEPLOY AND NOT DAVA_MEGASOLUTION )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-console-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-datetime-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-debug-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-errorhandling-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-file-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-file-l1-2-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-file-l2-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-handle-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-heap-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-interlocked-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-libraryloader-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-localization-l1-2-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-memory-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-namedpipe-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-processenvironment-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-processthreads-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-processthreads-l1-1-1.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-profile-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-rtlsupport-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-string-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-synch-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-synch-l1-2-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-sysinfo-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-timezone-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-core-util-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-conio-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-convert-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-environment-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-filesystem-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-heap-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-locale-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-math-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-multibyte-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-private-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-process-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-runtime-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-stdio-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-string-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-time-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/api-ms-win-crt-utility-l1-1-0.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/msvcp140.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/ucrtbase.dll $(OutDir)
        COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_BINARY_WIN_DIR_RELEASE}/vcruntime140.dll $(OutDir)
    )

    add_custom_target( "createPackageInfo" ALL
        COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Teamcity/createPackageInfo.py"
            "${DEPLOY_DIR}"
        DEPENDS ${PROJECT_NAME}
    )
endif()


