include (CMake-common)

# general code for downloading per GPU
function(download_resources_impl GPU META_FOLDER MAIN_CONFIG)
    message(STATUS "download_resources: ${GPU} | ${META_FOLDER} | ${MAIN_CONFIG}")

    # find python
    find_package(PythonInterp)
    if(NOT PYTHONINTERP_FOUND)
        set(PYTHON_EXECUTABLE python)
    endif()

    # enumerate files from meta
    get_filename_component(META_ROOT_DIR ${META_FOLDER} DIRECTORY)
    file(GLOB_RECURSE RES_FILES_RECURSE ${META_FOLDER}/*.res)
    foreach(RES_FILE ${RES_FILES_RECURSE})

        # Get the directory of the source file
        get_filename_component(RESOURCE_DIR ${RES_FILE} DIRECTORY)

        # Remove common directory prefix to make the group
        string(REPLACE ${META_ROOT_DIR} "" GROUP ${RESOURCE_DIR})
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        source_group(${GROUP} FILES ${RES_FILE})
    endforeach()

    # download resources
    get_filename_component(RESOURCES_CMD_TOOL "${CMAKE_CURRENT_LIST_DIR}/../Scripts/Resources/build_resources.py" ABSOLUTE)
    get_filename_component(ARTIFACTS_FOLDER "${CMAKE_CURRENT_LIST_DIR}/../Data" ABSOLUTE)

    set(UPDATE_STEP_NAME ${PROJECT_NAME}_UPDATE_RESOURCES)
    add_custom_target(${UPDATE_STEP_NAME}
                      ALL
                      COMMAND ${PYTHON_EXECUTABLE} "${RESOURCES_CMD_TOOL}" "--gpu=${GPU}" "--config=${MAIN_CONFIG}" "--dir=${ARTIFACTS_FOLDER}"
                      SOURCES ${RES_FILES_RECURSE}
                      DEPENDS ${RES_FILES_RECURSE}
                      VERBATIM
    )

    add_dependencies (${PROJECT_NAME} ${UPDATE_STEP_NAME})
endfunction (download_resources_impl)

# download resources for project
function (download_resources)
    get_filename_component(RESOURCES_META_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/../DataSource/meta" ABSOLUTE)
    set (RESOURCES_CONFIG master_config.txt)

    if (IOS)
        download_resources_impl("PowerVR_iOS" ${RESOURCES_META_FOLDER} ${RESOURCES_CONFIG})
    elseif(ANDROID)
        download_resources_impl("mali" ${RESOURCES_META_FOLDER} ${RESOURCES_CONFIG})
    else() # desktop
        download_resources_impl("dx11" ${RESOURCES_META_FOLDER} ${RESOURCES_CONFIG})
    endif()
endfunction (download_resources)

#copy resources as is from datasrouce/3d to save old behaviour
function (copy_resources)

    get_filename_component(DATA_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/../Data/" ABSOLUTE)
    get_filename_component(DATA_SOURCE_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/../DataSource/3d" ABSOLUTE)

    if (NOT ANDROID)
        file(COPY "${DATA_SOURCE_FOLDER}" DESTINATION "${DATA_FOLDER}")
    endif()
endfunction (copy_resources)

