package dava_framework_NewBuilds_Tests_UnitTests.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_Tests_UnitTests_Win10ProjectTemplate : Template({
    uuid = "7b40df77-0a04-401b-87f1-655d3af471ee"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_Win10ProjectTemplate"
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

        checkoutMode = CheckoutMode.ON_SERVER
        checkoutDir = "%system.teamcity.buildConfName%"
    }

    steps {
        script {
            name = "clear"
            id = "RUNNER_1187"
            workingDir = "RepoTools/Scripts"
            scriptContent = "python delete_folder.py %system.teamcity.build.checkoutDir%/%pathToProject%"
        }
        script {
            name = "Generating project"
            id = "RUNNER_1188"
            workingDir = "%pathToProject%"
            scriptContent = """../../../Programs/WinStoreCMake/mk_project.bat %system.teamcity.build.checkoutDir%\%pathToCmakeList% %system.teamcity.build.checkoutDir%"""
        }
        script {
            name = "Building project for platform x86"
            id = "RUNNER_1189"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=x86 /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Building project for platform x64"
            id = "RUNNER_1190"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=x64 /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Building project for platform arm"
            id = "RUNNER_1191"
            workingDir = "%pathToProject%"
            scriptContent = "cmake --build . --config Release -- /p:Platform=arm /p:GenerateAppxPackageOnBuild=Never"
        }
        script {
            name = "Creating bundle"
            id = "RUNNER_1192"
            workingDir = "%pathToProject%"
            scriptContent = """
                rm -rf AppPackages
                rm -rf BundleArtifacts
                cmake --build . --config Release -- /p:AppxBundle=Always /p:AppxBundlePlatforms="x86|x64|ARM"
            """.trimIndent()
        }
        script {
            name = "Packing artefacts"
            id = "RUNNER_1193"
            workingDir = "%pathToProject%/AppPackages"
            scriptContent = """if "%packProject%" == "yes" (zip -0 -r %projectName%.zip %projectName%/)"""
        }
    }

    requirements {
        exists("env.windows10", "RQ_264")
    }
})
