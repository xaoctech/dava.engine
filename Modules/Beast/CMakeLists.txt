set( MODULE_NAME Beast )
set( MODULE_TYPE STATIC )

set( PLATFORMS_SUPPORTED WIN )                

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES Sources
              Thirdparty/include
             )

set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS -D__DAVAENGINE_BEAST__ )

set( BeastThirdparty "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty")
set( BeastRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set( STATIC_LIBRARIES_WIN32  "${BeastThirdparty}/lib/beast32.lib" )
set( STATIC_LIBRARIES_WIN64  "${BeastThirdparty}/lib/beast64.lib" )
set( DYNAMIC_LIBRARIES_WIN32 "${BeastThirdparty}/bin/beast32.dll" )
set( DYNAMIC_LIBRARIES_WIN64 "${BeastThirdparty}/bin/beast64.dll" )


                 
set( MIX_APP_DATA  "Data.beast/bin = ${BeastThirdparty}/bin/ernsttool.xml"
                   "Data.beast/bin = ${BeastThirdparty}/bin/ernst-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/ernsttool-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/filtertool-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/oslc-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/rendertool.xml"
                   "Data.beast/bin = ${BeastThirdparty}/bin/rendertool-32.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/rendertool-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/rendertoolcontroller-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/thebeast-64.exe"
                   "Data.beast/bin = ${BeastThirdparty}/bin/uvtool-64.exe"

                   "Data.beast/config = ${BeastRootDir}/Config"
                   "Data.beast/shaders = ${BeastThirdparty}/shaders"
                   )




setup_main_module()
