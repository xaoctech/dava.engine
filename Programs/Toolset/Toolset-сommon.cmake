include ( CMake-common )

##############

set( JOIN_PROJECT_NAME 1 )
set( DAVA_MEGASOLUTION      1 )
set( IGNORE_FILE_TREE_CHECK 1 )

if( NOT DEPLOY_DIR )
    set( DEPLOY_DIR_MACOS             ${CMAKE_BINARY_DIR}/app )
    set( DEPLOY_DIR_WIN               ${CMAKE_BINARY_DIR}/app )
else()
    set( DEPLOY_DIR_MACOS             ${DEPLOY_DIR} )
    set( DEPLOY_DIR_WIN               ${DEPLOY_DIR} )
endif()

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( PROGRAMM_DIR "${DAVA_ROOT_DIR}/Programs" )

set( DEFAULT_DEPEND_DIRS "Sources/Internal;Modules;Sources/CMake" )

##############

macro ( prepare_tools )

    if( CHECK_DEPENDS_FOLDERS ) 

        find_package( PythonInterp   )

        set( TARGET_FOLDERS_LIST )
        foreach( TARGET_NAME ${SINGLE_TOOLS_LIST} ${PACKAGE_TOOLS_LIST} )
            string(REPLACE "${DAVA_ROOT_DIR}/" "" DEPEND_DIRS_${TARGET_NAME} "${DEPEND_DIRS_${TARGET_NAME}}" )
            if( TARGET_FOLDERS_LIST )
                set(  TARGET_FOLDERS_LIST  "'${TARGET_NAME}':'${DEPEND_DIRS_${TARGET_NAME}}',${TARGET_FOLDERS_LIST}")
            else()
                set(  TARGET_FOLDERS_LIST  "'${TARGET_NAME}':'${DEPEND_DIRS_${TARGET_NAME}}'")            
            endif()
        endforeach()

        set( TARGET_FOLDERS_LIST "--target_folders_list ${TARGET_FOLDERS_LIST}")

        string(REPLACE ";" "+" TARGET_FOLDERS_LIST "${TARGET_FOLDERS_LIST}" )

        set( TEAMCITY_URL     "--teamcity_url ${TEAMCITY_URL}")
        set( STASH_URL        "--stash_url ${STASH_URL}")

        set( TEAMCITY_LOGIN   "--teamcity_login ${TEAMCITY_LOGIN}")
        set( TEAMCITY_PASS    "--teamcity_password ${TEAMCITY_PASS}")
        set( STASH_LOGIN      "--stash_login ${STASH_LOGIN}")
        set( STASH_PASS       "--stash_password ${STASH_PASS}") 

        set( FRAMEWORK_BRANCH "--framework_branch ${FRAMEWORK_BRANCH}" )
   
        set( ARGS_DEPEND    ${TEAMCITY_URL}
                            ${STASH_URL}
                            ${TEAMCITY_LOGIN}
                            ${TEAMCITY_PASS}
                            ${STASH_LOGIN}
                            ${STASH_PASS}
                            ${FRAMEWORK_BRANCH}
                            ${TARGET_FOLDERS_LIST} )

        string(REPLACE " " ";" ARGS_DEPEND "${ARGS_DEPEND}" )


        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} "${DAVA_ROOT_DIR}/RepoTools/Teamcity/check_depends_of_targets_on_folders.py" ${ARGS_DEPEND}
            OUTPUT_VARIABLE BUILD_TARGETS
        )

        string(REPLACE "\n" "" BUILD_TARGETS "${BUILD_TARGETS}")

    endif()
    
#######

    set( DEFINITIONS            )

    set( DEPLOY_DIR             ${CMAKE_BINARY_DIR}/app_other )
    set( MIX_APP_DIR            )
    set( DEPLOY_DIR_EXECUTABLE  )
    set( QT_POST_DEPLOY         )

    reset_property ( MIX_APP_DATA )

    __add_tools( SINGLE_TOOLS_LIST )

    set( POSTPONED_MIX_DATA 1 )
    set( QT_POST_DEPLOY 0 )

    set( DEPLOY_DIR_LIBS_MACOS        ${DEPLOY_DIR_MACOS}/Libs )
    set( DEPLOY_DIR_DATA_MACOS        ${DEPLOY_DIR_MACOS} )
    set( DEPLOY_DIR_EXECUTABLE_MACOS  ${DEPLOY_DIR_MACOS} )
    set( ADDED_LD_RUNPATHES           "@executable_path/../../Libs @executable_path/../../../Libs" )


    set( DEPLOY_DIR_LIBS_WIN          )
    set( DEPLOY_DIR_DATA_WIN          )
    set( DEPLOY_DIR_EXECUTABLE_WIN    )

    set( DEPLOY_DIR             ${DEPLOY_DIR_${DAVA_PLATFORM_CURRENT}})
    set( DEPLOY_DIR_LIBS        ${DEPLOY_DIR_LIBS_${DAVA_PLATFORM_CURRENT}})
    set( DEPLOY_DIR_DATA        ${DEPLOY_DIR_DATA_${DAVA_PLATFORM_CURRENT}})
    set( DEPLOY_DIR_EXECUTABLE  ${DEPLOY_DIR_EXECUTABLE_${DAVA_PLATFORM_CURRENT}})
    set( MIX_APP_DIR            ${CMAKE_BINARY_DIR}/MixResources )

    reset_property ( MIX_APP_DATA )

    __add_tools( PACKAGE_TOOLS_LIST )

    processing_mix_data()
    processing_mix_data_dependencies( "${PACKAGE_TOOLS_LIST}" )

    generation_ToolList_json( "${GENERATED_PACKAGE_TOOLS_LIST}" ) 

    dump_module_log()

    if( NO_GENERATED_TOOLS )
         message( STATUS "These applications are not generated because there were no changes in the dependent folders:" )
         foreach( TARGET_NAME ${NO_GENERATED_TOOLS} )
              message( STATUS "  ${TARGET_NAME}" )
         endforeach()
         message("")

    endif()

endmacro()

macro ( add_tool_single TARGET_NAME )
    cmake_parse_arguments ( ARG "NO_UNITY_BUILD"  "ROOT_DIR;CUSTOM_DEPLOY_DIR;DEPLOY_DEFINE;DEPENDS" "" ${ARGN} )

    list( APPEND SINGLE_TOOLS_LIST ${TARGET_NAME} )

    set( ROOT_DIR_${TARGET_NAME}           ${ARG_ROOT_DIR} )
    set( CUSTOM_DEPLOY_DIR_${TARGET_NAME}  ${ARG_CUSTOM_DEPLOY_DIR} )
    set( DEPLOY_DEFINE_${TARGET_NAME}      ${ARG_DEPLOY_DEFINE} )
    set( NO_UNITY_BUILD_${TARGET_NAME}     ${ARG_NO_UNITY_BUILD} )
    set( DEPENDS_${TARGET_NAME}            ${ARG_DEPENDS} )

    if( ARG_ROOT_DIR )
        set( DEPEND_DIRS_${TARGET_NAME}  ${DEFAULT_DEPEND_DIRS} ${ARG_ROOT_DIR} )
    else()
        set( DEPEND_DIRS_${TARGET_NAME}  ${DEFAULT_DEPEND_DIRS}  ${PROGRAMM_DIR}/${TARGET_NAME} )
    endif()
   
endmacro ()

macro ( add_tool_package TARGET_NAME )
    cmake_parse_arguments ( ARG "NO_UNITY_BUILD"  "ROOT_DIR;CUSTOM_DEPLOY_DIR;DEPLOY_DEFINE;DEPENDS" "" ${ARGN} )

    list( APPEND PACKAGE_TOOLS_LIST ${TARGET_NAME} )

    set( ROOT_DIR_${TARGET_NAME}           ${ARG_ROOT_DIR} )
    set( CUSTOM_DEPLOY_DIR_${TARGET_NAME}  ${ARG_CUSTOM_DEPLOY_DIR} )
    set( DEPLOY_DEFINE_${TARGET_NAME}      ${ARG_DEPLOY_DEFINE} )
    set( NO_UNITY_BUILD_${TARGET_NAME}     ${ARG_NO_UNITY_BUILD} )
    set( DEPENDS_${TARGET_NAME}            ${ARG_DEPENDS} )

    if( ARG_ROOT_DIR )
        set( DEPEND_DIRS_${TARGET_NAME}  ${DEFAULT_DEPEND_DIRS} ${ARG_ROOT_DIR} )
    else()
        set( DEPEND_DIRS_${TARGET_NAME}  ${DEFAULT_DEPEND_DIRS} ${PROGRAMM_DIR}/${TARGET_NAME} )
    endif()
    
endmacro ()

macro ( __add_tools TOOLS_LIST_NAME )

    set( TOOLS_LIST ${${TOOLS_LIST_NAME}} )

    set( GENERATED_${TOOLS_LIST_NAME} )

    set( UNITY_BUILD_OLD_VALUE  ${UNITY_BUILD} )

    foreach( TARGET_NAME ${TOOLS_LIST} )

        set( ARG_ROOT_DIR          ${ROOT_DIR_${TARGET_NAME}} )
        set( ARG_CUSTOM_DEPLOY_DIR ${CUSTOM_DEPLOY_DIR_${TARGET_NAME}} )
        set( ARG_DEPLOY_DEFINE     ${DEPLOY_DEFINE_${TARGET_NAME}} )
        set( ARG_NO_UNITY_BUILD    ${NO_UNITY_BUILD_${TARGET_NAME}} )

        set( GENERATE true )

        if( CHECK_DEPENDS_FOLDERS AND ( NOT "ALL_BUILD" STREQUAL "${BUILD_TARGETS}" ) ) 
            list (FIND BUILD_TARGETS   ${TARGET_NAME} _index)
            if ( ${_index} EQUAL -1)
                set( GENERATE false )
                foreach( ITEM ${DEPENDS_${TARGET_NAME}} )
                    list (FIND BUILD_TARGETS   ${ITEM} _index)
                    if( NOT ${_index} EQUAL -1 )
                        set( GENERATE true )
                        break()
                    endif()
                endforeach()
            endif()
        endif()

        if( GENERATE )
            list( APPEND GENERATED_${TOOLS_LIST_NAME} ${TARGET_NAME} )
      
            if( ARG_DEPLOY_DEFINE AND DEPLOY ) 
                get_property( DEFINITIONS_OLD GLOBAL PROPERTY DEFINITIONS )
                append_property( DEFINITIONS ${ARG_DEPLOY_DEFINE} )
            endif()

            set( OLD_DEPLOY_DIR ${DEPLOY_DIR} )
            if( ARG_CUSTOM_DEPLOY_DIR AND WIN32 )
                set( DEPLOY_DIR ${ARG_CUSTOM_DEPLOY_DIR}/${TARGET_NAME} )
            endif()  

            if( ARG_NO_UNITY_BUILD )
                set( UNITY_BUILD false )
            endif()
             
            message(STATUS "GENERATE [ ${TARGET_NAME} ] UNITY_BUILD [ ${UNITY_BUILD} ] ")

            if( ARG_ROOT_DIR )
                add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
            else()
                add_subdirectory      ( "${PROGRAMM_DIR}/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
            endif() 

            if( ARG_DEPLOY_DEFINE AND DEPLOY ) 
                set_property( GLOBAL PROPERTY DEFINITIONS "${DEFINITIONS_OLD}" )
            endif()

            set( DEPLOY_DIR ${OLD_DEPLOY_DIR} )
        else()
            list( APPEND NO_GENERATED_TOOLS ${TARGET_NAME} )

        endif()

        set( UNITY_BUILD  ${UNITY_BUILD_OLD_VALUE} )
    endforeach()

endmacro ()

macro ( generation_ToolList_json TARGET_LIST )
    set( TOOL_LIST_FILE  ${CMAKE_BINARY_DIR}/Info.json )

    file(WRITE  ${TOOL_LIST_FILE} "{\n" )
    file(APPEND ${TOOL_LIST_FILE} "   \"Applications\": [\n" )

    set( TARGET_LIST "${TARGET_LIST}")
    list( LENGTH TARGET_LIST TARGET_LIST_COUNT )
    math( EXPR TARGET_LIST_COUNT "${TARGET_LIST_COUNT} - 1" )

    if ( ${TARGET_LIST_COUNT} GREATER 0)
        foreach ( index RANGE ${TARGET_LIST_COUNT} )
            list ( GET TARGET_LIST ${index} ITEM )
            file( APPEND ${TOOL_LIST_FILE} "      {\n" )
            file( APPEND ${TOOL_LIST_FILE} "          \"name\": \"${ITEM}\"\n" )
            if (${index} EQUAL ${TARGET_LIST_COUNT})
                file( APPEND ${TOOL_LIST_FILE} "      }\n" )
            else()
                file( APPEND ${TOOL_LIST_FILE} "      },\n" )
            endif()
        endforeach()
    endif()

    file(APPEND ${TOOL_LIST_FILE} "   ]\n" )
    file( APPEND ${TOOL_LIST_FILE} "}\n" )

endmacro ()

