package dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar

import dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.buildTypes.*
import dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.vcsRoots.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project
import jetbrains.buildServer.configs.kotlin.v10.ProjectFeature
import jetbrains.buildServer.configs.kotlin.v10.ProjectFeature.*

object Project : Project({
    uuid = "8a46efcc-3ab1-4599-a7c7-a41242360019"
    extId = "dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar"
    parentId = "dava_framework_NewBuilds_Tests"
    name = "PerfomanceTests"

    vcsRoot(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_DavaFrameworkPerformanceTes)

    buildType(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid)
    buildType(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerformanceTestsIOS)
    buildType(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid_old)

    features {
        feature {
            id = "PROJECT_EXT_1"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "GlobalPerformanceTest_Min_fps_branch_9_from_device_samsungSM-T325",
                    "key": "GlobalPerformanceTest_Min_fps_branch_9_from_device_samsungSM-T325"
                  },
                  {
                    "type": "valueType",
                    "title": "GlobalPerformanceTest_Max_fps_branch_9_from_device_samsungSM-T325",
                    "key": "GlobalPerformanceTest_Max_fps_branch_9_from_device_samsungSM-T325"
                  },
                  {
                    "type": "valueType",
                    "title": "GlobalPerformanceTest_Average_fps_branch_9_from_device_samsungSM-T325",
                    "key": "GlobalPerformanceTest_Average_fps_branch_9_from_device_samsungSM-T325"
                  }
                ]
            """.trimIndent())
            param("format", "text")
            param("hideFilters", "")
            param("title", "Fps")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_10"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Average_frame_delta_branch_development_samsungSM-T325",
                    "key": "AsiaPerformanceTest_Average_frame_delta_branch_development_samsungSM-T325"
                  }
                ]
            """.trimIndent())
            param("hideFilters", "")
            param("title", "New chart title")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_11"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Average_fps_branch_258_from_device_samsungSM-T325",
                    "key": "AsiaPerformanceTest_Average_fps_branch_258_from_device_samsungSM-T325"
                  }
                ]
            """.trimIndent())
            param("hideFilters", "")
            param("title", "New chart title")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_12"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Max_frame_delta_branch_434_from_samsungSM-T325",
                    "key": "AsiaPerformanceTest_Max_frame_delta_branch_434_from_samsungSM-T325"
                  }
                ]
            """.trimIndent())
            param("hideFilters", "")
            param("title", "New chart title")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_13"
            type = "buildtype-graphs.order"
            param("order", "PROJECT_EXT_1,PROJECT_EXT_6,PROJECT_EXT_7,PROJECT_EXT_8,PROJECT_EXT_9,PROJECT_EXT_10,PROJECT_EXT_11,PROJECT_EXT_12")
        }
        feature {
            id = "PROJECT_EXT_6"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "GlobalPerformanceTest_Max_memory_usage_branch_9_from_device_samsungSM-T325",
                    "key": "GlobalPerformanceTest_Max_memory_usage_branch_9_from_device_samsungSM-T325"
                  }
                ]
            """.trimIndent())
            param("format", "size")
            param("hideFilters", "")
            param("title", "Memory")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_7"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Average_fps_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Average_fps_branch_development_Apple_inc_iPad_6_WiFi"
                  },
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Max_fps_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Max_fps_branch_development_Apple_inc_iPad_6_WiFi"
                  },
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Min_fps_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Min_fps_branch_development_Apple_inc_iPad_6_WiFi"
                  }
                ]
            """.trimIndent())
            param("hideFilters", "")
            param("title", "FPS chart")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_8"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Average_frame_delta_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Average_frame_delta_branch_development_Apple_inc_iPad_6_WiFi"
                  },
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Max_frame_delta_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Max_frame_delta_branch_development_Apple_inc_iPad_6_WiFi"
                  },
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Min_frame_delta_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Min_frame_delta_branch_development_Apple_inc_iPad_6_WiFi"
                  }
                ]
            """.trimIndent())
            param("format", "text")
            param("hideFilters", "")
            param("title", "Frame time chart")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
        feature {
            id = "PROJECT_EXT_9"
            type = "buildtype-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "AsiaPerformanceTest_Max_memory_usage_branch_development_Apple_inc_iPad_6_WiFi",
                    "key": "AsiaPerformanceTest_Max_memory_usage_branch_development_Apple_inc_iPad_6_WiFi"
                  }
                ]
            """.trimIndent())
            param("format", "size")
            param("hideFilters", "")
            param("title", "Memory chart")
            param("defaultFilters", "")
            param("seriesTitle", "Serie")
        }
    }
})
