package dava_framework_NewBuilds_ToolSet

import dava_framework_NewBuilds_ToolSet.buildTypes.*
import dava_framework_NewBuilds_ToolSet.vcsRoots.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "4d14e6e8-b005-4d23-9ec8-62adc008b656"
    extId = "dava_framework_NewBuilds_ToolSet"
    parentId = "dava_framework"
    name = "ToolSet"

    vcsRoot(dava_framework_NewBuilds_ToolSet_ImageUnpacker)
    vcsRoot(dava_framework_NewBuilds_ToolSet_Runner)
    vcsRoot(dava_framework_NewBuilds_ToolSet_Tests)
    vcsRoot(dava_framework_NewBuilds_ToolSet_Lib)
    vcsRoot(dava_framework_NewBuilds_ToolSet_ResourceEditorConsoleTesting)
    vcsRoot(dava_framework_NewBuilds_ToolSet_StashDava)

    buildType(dava_framework_ToolSet_ToolSetAndroid)
    buildType(dava_framework_ToolSet_ToolSetIos)
    buildType(dava_framework_NewBuilds_ToolSet_ToolSetMac)
    buildType(dava_framework_NewBuilds_ToolSet_ToolSetWin)
    buildType(dava_framework_ToolSet_ToolSetWin10)
    buildType(dava_framework_ToolSet_ToolSetLinux)

    cleanup {
        artifacts(days = 7)
    }
})
