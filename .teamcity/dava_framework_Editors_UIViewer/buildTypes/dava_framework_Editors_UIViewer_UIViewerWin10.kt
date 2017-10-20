package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Editors_UIViewer_UIViewerWin10 : BuildType({
    template(dava_framework_Editors_UIViewer.buildTypes.dava_framework_Editors_UIViewer_Win10ProjectTemplate)
    uuid = "612bac02-4d54-4c13-9c37-15f845c6d3df"
    extId = "dava_framework_Editors_UIViewer_UIViewerWin10"
    name = "UIViewer_Win10"

    artifactRules = "%pathToProject%/AppPackagesZip/*.zip"

    params {
        param("pathToCmakeList", """Programs\%projectName%""")
        param("pathToOutPackDir", "%pathToProject%/AppPackagesZip")
        param("pathToProjectApp", "%pathToProject%/AppPackages")
        param("projectName", "UIViewer")
        param("ProjectName", "UIViewer")
    }

    vcs {
        cleanCheckout = true
    }

    steps {
        script {
            name = "create version.h"
            id = "RUNNER_420"
            workingDir = "RepoTools/Scripts"
            scriptContent = "python create_version_h.py --dava_path %system.teamcity.build.checkoutDir% --build_number %build.number% --branch_info %teamcity.build.branch%"
        }
        script {
            name = "PackApp"
            id = "RUNNER_421"
            workingDir = "RepoTools/Scripts"
            scriptContent = "python pack_app.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir% --build_number %build.number%"
        }
        stepsOrder = arrayListOf("RUNNER_420", "RUNNER_1032", "RUNNER_1033", "RUNNER_1034", "RUNNER_1035", "RUNNER_1036", "RUNNER_1037", "RUNNER_421")
    }
    
    disableSettings("RUNNER_1037")
})
