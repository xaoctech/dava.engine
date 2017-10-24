package dava_framework_Editors_ResourceEditorPublic

import dava_framework_Editors_ResourceEditorPublic.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "01ed0b6f-ec94-4e04-8649-bc17242c873f"
    extId = "dava_framework_Editors_ResourceEditorPublic"
    parentId = "dava_framework_Editors"
    name = "ResourceEditor_Public"

    buildType(dava_framework_Editors_ResourceEditorPublic_ResourceEditorWin)
    buildType(dava_framework_Editors_ResourceEditorPublic_ResourceEditorMac)
})
