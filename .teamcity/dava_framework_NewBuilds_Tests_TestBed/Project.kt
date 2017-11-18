package dava_framework_NewBuilds_Tests_TestBed

import dava_framework_NewBuilds_Tests_TestBed.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "da8bf772-a9ef-430b-adc8-6ec1767bcd39"
    extId = "dava_framework_NewBuilds_Tests_TestBed"
    parentId = "dava_framework_NewBuilds_Tests"
    name = "TestBed"

    buildType(dava_framework_NewBuilds_Tests_TestBed_Android)
    buildType(dava_framework_NewBuilds_Tests_TestBed_MacOS)
    buildType(dava_framework_NewBuilds_Tests_TestBed_Ios)
    buildType(dava_framework_NewBuilds_Tests_TestBed_Win)
    buildType(dava_framework_NewBuilds_Tests_TestBed_TestBedWin10)

    template(dava_framework_NewBuilds_Tests_TestBed_Win10ProjectTemplate)

    params {
        param("framework_name", "development")
    }
})
