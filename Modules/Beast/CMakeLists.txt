set( NAME_MODULE Beast )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/Beast/*.h )
set( CPP_FILES_RECURSE Sources/Beast/*.cpp )

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" )
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

if (WIN32)

    if (DEPLOY AND NOT DAVA_MEGASOLUTION )
       set( DEPLOY_ROOT ${DEPLOY_DIR} )
       set( DEPLOY_ROOT ${DEPLOY_ROOT}/${PROJECT_NAME} )
       set( DEPLOY_DIR ${DEPLOY_ROOT}/dava.framework/Programs/${PROJECT_NAME} )
       set( DEPLOY_BEAST_DIR ${DEPLOY_DIR} )
    endif()

    dava_add_definitions( -D__DAVAENGINE_BEAST__ )

    set( BEAST_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR} )
    set( BEAST_WRAPPER "${BEAST_DIR}/wrapper" )

    if( X64_MODE )
        set( BEAST_LIB "${BEAST_DIR}/beast/lib/beast64.lib" )
        set( BEAST_DLL "${BEAST_DIR}/beast/bin/beast64.dll")
    else ()
         set( BEAST_LIB "${BEAST_DIR}/beast/lib/beast32.lib" )
         set( BEAST_DLL "${BEAST_DIR}/beast/bin/beast32.dll")
    endif()

    set( LIBRARIES  ${BEAST_LIB} )

    DEPLOY_SCRIPT ( COPY_WIN32  ${BEAST_DLL}
                    COPY_WIN64  ${BEAST_DLL}
                    COPY_DIR    ${DEPLOY_DIR})


    if (DEPLOY)
        if( NOT DEPLOY)
            set( DEPLOY_BEAST_DIR ${MIX_APP_DIR} )
        endif()

        if( NOT DEPLOY_BEAST_DIR )
            set( DEPLOY_BEAST_DIR ${DEPLOY_DIR} )
        endif()

        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}  POST_BUILD
             COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Teamcity/postDeploy.py"
                     "${DEPLOY_BEAST_DIR}"
                     "${BEAST_DIR}"
            )
    endif()

endif()



setup_main_module()
