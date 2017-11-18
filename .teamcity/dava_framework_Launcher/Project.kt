package dava_framework_Launcher

import dava_framework_Launcher.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "e753eed1-288e-4196-8dd4-d28172d42a21"
    extId = "dava_framework_Launcher"
    parentId = "dava_framework_Editors"
    name = "Launcher"

    buildType(dava_framework_Launcher_LauncherMacOS)
    buildType(dava_framework_Launcher_LauncherWin)
})
