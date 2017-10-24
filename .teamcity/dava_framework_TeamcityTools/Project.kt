package dava_framework_TeamcityTools

import dava_framework_TeamcityTools.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "65debc0c-e66b-401c-aaa0-4fca3400a175"
    extId = "dava_framework_TeamcityTools"
    parentId = "dava_framework"
    name = "TeamcityTools"

    buildType(dava_framework_TeamcityTools_ChangeBuildDescription)
    buildType(dava_framework_WoTBlitzStash_MergeSvnTrunkToStashMaster)
    buildType(dava_framework_TeamcityTools_UpdatedDavaengineVersion)
    buildType(dava_framework_TeamcityTools_UpdateDependBuildStatus)
})
