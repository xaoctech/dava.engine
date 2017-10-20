package dava_framework_EditorsTmp_2_QuickEd

import dava_framework_EditorsTmp_2_QuickEd.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "d45ffa1e-1736-4129-8abb-64c843bd48f6"
    extId = "dava_framework_EditorsTmp_2_QuickEd"
    parentId = "dava_framework_Editors"
    name = "QuickEd"

    buildType(dava_framework_EditorsTmp_2_QuickEdWin)
    buildType(dava_framework_EditorsTmp_2_QuickEd_QuickEdMac)
})
