package dava_framework_Editors_SceneViewer

import dava_framework_Editors_SceneViewer.buildTypes.*
import dava_framework_Editors_SceneViewer.vcsRoots.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "cc531c20-e09b-40b5-adec-fb09ae3487ef"
    extId = "dava_framework_Editors_SceneViewer"
    parentId = "dava_framework_Editors"
    name = "SceneViewer"

    vcsRoot(dava_framework_Editors_SceneViewer_SceneViewerData1)
    vcsRoot(dava_framework_Editors_SceneViewer_SceneViewerData)

    buildType(dava_framework_Editors_SceneViewer_SceneViewerWin)
    buildType(dava_framework_Editors_SceneViewer_SceneViewerIOS)
    buildType(dava_framework_Editors_SceneViewer_SceneViewerMacOS)
    buildType(dava_framework_Editors_SceneViewer_SceneViewerAndroid_CrystaX_NDK)
    buildType(dava_framework_Editors_SceneViewer_SceneViewerAndroid)
    buildType(dava_framework_Editors_SceneViewer_SceneViewerWin10)

    template(dava_framework_Editors_SceneViewer_Win10ProjectTemplate)

    params {
        param("framework_name", "development")
    }
})
