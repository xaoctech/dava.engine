package dava_framework_NewBuilds_Tests_UnitTests

import dava_framework_NewBuilds_Tests_UnitTests.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project
import jetbrains.buildServer.configs.kotlin.v10.ProjectFeature
import jetbrains.buildServer.configs.kotlin.v10.ProjectFeature.*

object Project : Project({
    uuid = "d086ae7f-99e5-47e9-abcd-9dee21ad4948"
    extId = "dava_framework_NewBuilds_Tests_UnitTests"
    parentId = "dava_framework_NewBuilds_Tests"
    name = "UnitTests"

    buildType(dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UnitTestsLinux)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_FormatTestTmp)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin32)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework_2)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework)

    template(dava_framework_NewBuilds_Tests_UnitTests_Win10ProjectTemplate)

    params {
        param("framework_name", "development")
    }

    features {
        feature {
            id = "PROJECT_EXT_59"
            type = "project-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "Build Duration (all stages)",
                    "sourceBuildTypeId": "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid_old",
                    "key": "BuildDuration"
                  }
                ]
            """.trimIndent())
            param("format", "duration")
            param("hideFilters", "")
            param("title", "Build Duration UnitTests_Android")
            param("defaultFilters", "")
            param("seriesTitle", "Build Duration")
        }
        feature {
            id = "PROJECT_EXT_60"
            type = "project-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "Build Duration (all stages)",
                    "sourceBuildTypeId": "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2",
                    "key": "BuildDuration"
                  }
                ]
            """.trimIndent())
            param("format", "duration")
            param("title", "Build Duration UnitTests_iOS")
            param("seriesTitle", "Time")
        }
        feature {
            id = "PROJECT_EXT_61"
            type = "project-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "Build Duration (all stages)",
                    "sourceBuildTypeId": "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102",
                    "key": "BuildDuration"
                  }
                ]
            """.trimIndent())
            param("format", "duration")
            param("title", "Build Duration UnitTests_Win10")
            param("seriesTitle", "Time")
        }
        feature {
            id = "PROJECT_EXT_62"
            type = "project-graphs"
            param("series", """
                [
                  {
                    "type": "valueType",
                    "title": "Build Duration (all stages)",
                    "sourceBuildTypeId": "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin32",
                    "key": "BuildDuration"
                  }
                ]
            """.trimIndent())
            param("format", "duration")
            param("title", "Build Duration UnitTests_Win32")
            param("seriesTitle", "Time")
        }
    }
})
