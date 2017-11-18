package dava_framework_Editors_SceneViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*

object dava_framework_Editors_SceneViewer_SceneViewerWin : BuildType({
    template = "dava_framework_TemplateDavaTools_win"
    uuid = "825faa3c-e29e-46ff-8496-da1f5ebf3a2b"
    extId = "dava_framework_Editors_SceneViewer_SceneViewerWin"
    name = "SceneViewer_Win"

    params {
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "SceneViewer")
    }

    vcs {
        root(dava_framework_Editors_SceneViewer.vcsRoots.dava_framework_Editors_SceneViewer_SceneViewerData, "+:.=>./dava.framework/Programs/SceneViewer/Data/")

    }
})
