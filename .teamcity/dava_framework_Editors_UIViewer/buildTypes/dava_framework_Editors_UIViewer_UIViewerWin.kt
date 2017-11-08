package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*

object dava_framework_Editors_UIViewer_UIViewerWin : BuildType({
    template = "dava_framework_TemplateDavaTools_win"
    uuid = "2562e5e6-64b3-481a-bc5c-a303dbf7b961"
    extId = "dava_framework_Editors_UIViewer_UIViewerWin"
    name = "UIViewer_Win"

    params {
        param("ProjectName", "UIViewer")
    }
})
