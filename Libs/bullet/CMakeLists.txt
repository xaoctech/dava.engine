cmake_minimum_required(VERSION 3.0)
project(bullet)

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES
    "${PROJECT_SOURCE_DIR}/BulletCollision/*.cpp" "${PROJECT_SOURCE_DIR}/BulletCollision/*.h"
    "${PROJECT_SOURCE_DIR}/BulletDynamics/*.cpp" "${PROJECT_SOURCE_DIR}/BulletDynamics/*.h"
    "${PROJECT_SOURCE_DIR}/LinearMath/*.cpp" "${PROJECT_SOURCE_DIR}/LinearMath/*.h"
    "${PROJECT_SOURCE_DIR}/vectormath/*.cpp" "${PROJECT_SOURCE_DIR}/vectormath/*.h"
    "${PROJECT_SOURCE_DIR}/*.h"
)

if (MSVC)
    foreach(FILE ${SRC_FILES})
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
        if ("${PARENT_DIR}" STREQUAL "")
            source_group("${TOP_GROUP}" FILES "${FILE}")
        else()
            string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
            source_group("${TOP_GROUP}\\${GROUP}" FILES "${FILE}")
        endif()
    endforeach()
endif()

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
