package dava_framework_Editors_SceneViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*

object dava_framework_Editors_SceneViewer_SceneViewerMacOS : BuildType({
    template = "dava_framework_TemplateDAVATools_mac"
    uuid = "e2995825-34d4-41d4-ae4b-1fc495f19fda"
    extId = "dava_framework_Editors_SceneViewer_SceneViewerMacOS"
    name = "SceneViewer_MacOS"

    params {
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "SceneViewer")
    }

    vcs {
        root(dava_framework_Editors_SceneViewer.vcsRoots.dava_framework_Editors_SceneViewer_SceneViewerData, "+:.=>./dava.framework/Programs/SceneViewer/Data/")

    }
})
