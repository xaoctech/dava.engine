package dava_framework_Editors

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project
import jetbrains.buildServer.configs.kotlin.v10.projectFeatures.VersionedSettings
import jetbrains.buildServer.configs.kotlin.v10.projectFeatures.VersionedSettings.*
import jetbrains.buildServer.configs.kotlin.v10.projectFeatures.versionedSettings

object Project : Project({
    uuid = "bff0f279-91fa-4cb6-93d7-707318b6836b"
    extId = "dava_framework_Editors"
    parentId = "dava_framework"
    name = "Single Tools"

    features {
        versionedSettings {
            id = "PROJECT_EXT_18"
            mode = VersionedSettings.Mode.DISABLED
        }
    }
})
