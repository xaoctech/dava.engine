package dava_framework_EditorsTmp_TmpResourcePacker

import dava_framework_EditorsTmp_TmpResourcePacker.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "676b352e-54a2-4391-ab0a-bacb21718c7d"
    extId = "dava_framework_EditorsTmp_TmpResourcePacker"
    parentId = "dava_framework_Editors"
    name = "ResourcePacker"

    buildType(dava_framework_EditorsTmp_TmpResourcePacker_AutotestsMac)
    buildType(dava_framework_EditorsTmp_TmpResourcePacker_AutotestsWin)
})
