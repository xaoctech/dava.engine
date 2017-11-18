package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_Editors_UIViewer_Win10ProjectTemplate : Template({
    uuid = "5c30e8bc-bb60-41cf-a725-1c52a981409b"
    extId = "dava_framework_Editors_UIViewer_Win10ProjectTemplate"
    name = "Win10_Project_Template"

    artifactRules = "%pathToProject%/AppPackages/%projectName%.zip"

    params {
        checkbox("packProject", "yes",
                  checked = "yes", unchecked = "no")
        param("pathToBin", """%system.teamcity.build.checkoutDir%\Tools\Bin""")
        param("pathToCmakeList", """Projects\%projectName%""")
        param("pathToProject", """%pathToCmakeList%\Release""")
        param("projectName", "%system.teamcity.buildConfName%")
    }

    vcs {
        root("dava_DavaFrameworkStash")

        checkoutMode = CheckoutMode.ON_AGENT
        checkoutDir = "%system.teamcity.buildConfName%"
    }

    steps {
        script {
            name = "Generating project"
            id = "RUNNER_1032"
            workingDir = "%pathToProject%"
            scriptContent = """../../../Programs/WinStoreCMake/mk_project.bat %system.teamcity.build.checkoutDir%\%pathToCmakeList% %system.teamcity.build.checkoutDir% UnityBuild"""
        }
        script {
            name = "Building project for platform x86"
            id = "RUNNER_1033"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=x86 /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Building project for platform x64"
            id = "RUNNER_1034"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=x64 /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Building project for platform arm"
            id = "RUNNER_1035"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=arm /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Creating bundle"
            id = "RUNNER_1036"
            workingDir = "%pathToProject%"
            scriptContent = """
                rm -rf AppPackages
                rm -rf BundleArtifacts
                cmake --build . --config Release -- /p:AppxBundle=Always /p:AppxBundlePlatforms="x86|x64|ARM"
            """.trimIndent()
        }
        script {
            name = "Packing artefacts"
            id = "RUNNER_1037"
            workingDir = "%pathToProject%/AppPackages"
            scriptContent = """if "%packProject%" == "yes" (zip -0 -r %projectName%.zip %projectName%/)"""
        }
    }

    triggers {
        vcs {
            id = "vcsTrigger"
            enabled = false
        }
    }

    requirements {
        exists("env.windows10", "RQ_115")
    }
})
