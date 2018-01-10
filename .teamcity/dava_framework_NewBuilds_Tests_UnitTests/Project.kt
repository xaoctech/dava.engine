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
    buildType(dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS)
    buildType(dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework)

    template(dava_framework_NewBuilds_Tests_UnitTests_Win10ProjectTemplate)

    params {
        param("framework_name", "development")
    }
})
