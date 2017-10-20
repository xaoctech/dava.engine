package dava_framework_Editors_UIViewer

import dava_framework_Editors_UIViewer.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "c3d4fffe-c1ac-47cb-bcca-2f2db77aa5b5"
    extId = "dava_framework_Editors_UIViewer"
    parentId = "dava_framework_Editors"
    name = "UIViewer"

    buildType(dava_framework_Editors_UIViewer_UIViewerAndroid)
    buildType(dava_framework_Editors_UIViewer_UIViewerMacOS)
    buildType(dava_framework_Editors_UIViewer_UIViewerAndroidCrystaXNdk)
    buildType(dava_framework_Editors_UIViewer_UIViewerWin10)
    buildType(dava_framework_Editors_UIViewer_UIViewerIOS)
    buildType(dava_framework_Editors_UIViewer_UIViewerWin)

    template(dava_framework_Editors_UIViewer_Win10ProjectTemplate)

    params {
        param("framework_name", "development")
    }
})
