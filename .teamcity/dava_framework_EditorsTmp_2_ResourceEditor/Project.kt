package dava_framework_EditorsTmp_2_ResourceEditor

import dava_framework_EditorsTmp_2_ResourceEditor.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "3a5cb24c-8f3f-4b3f-ad62-482886db22a7"
    extId = "dava_framework_EditorsTmp_2_ResourceEditor"
    parentId = "dava_framework_Editors"
    name = "ResourceEditor"

    buildType(dava_framework_EditorsTmp_2_ResourceEditor_ResourceEditorWin)
    buildType(dava_framework_EditorsTmp_2_ResourceEditor_ResourceEditorMac)
})
