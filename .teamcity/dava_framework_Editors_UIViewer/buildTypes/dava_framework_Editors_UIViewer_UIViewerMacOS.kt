package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*

object dava_framework_Editors_UIViewer_UIViewerMacOS : BuildType({
    template(dava_framework.buildTypes.dava_framework_TemplateDAVATools_mac)
    uuid = "012da42c-ea63-4136-ad86-2771bd5505a3"
    extId = "dava_framework_Editors_UIViewer_UIViewerMacOS"
    name = "UIViewer_MacOS"

    params {
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "UIViewer")
    }
})
