package dava_framework_Launcher.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Launcher_LauncherMacOS : BuildType({
    uuid = "a7087a4a-4aee-4a5f-9872-4b8decee83b0"
    extId = "dava_framework_Launcher_LauncherMacOS"
    name = "Launcher_MacOS"

    artifactRules = "%pathToProjectApp%/Launcher*.zip"
    buildNumberPattern = "1.0.%build.counter%"

    params {
        param("add_definitions", "-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%")
        param("framework_name", "development")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("platform", "macos")
        param("ProjectName", "Launcher")
        param("UNITY_BUILD", "true")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")

        checkoutMode = CheckoutMode.ON_SERVER
    }

    steps {
        script {
            name = "generate project"
            workingDir = "%pathToProjectBuild%"
            scriptContent = """
                rm -rf %pathToProjectApp%/*.*
                rm -rf %pathToProjectBuild%/*.*
                python %dava_gen% %platform% %pathToProject% --add_definitions=%add_definitions%
            """.trimIndent()
        }
        script {
            name = "build project"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "python %dava_dir%/Bin/RepoTools/Scripts/dava_build_wrapper.py --config=Release --teamcityLog=true --pathToDava=%dava_dir% --pathToBuild=%pathToProjectBuild%"
        }
        script {
            name = "self test"
            workingDir = "%pathToProjectApp%"
            scriptContent = "./Launcher.app/Contents/MacOS/Launcher -s"
        }
        script {
            name = "pack app"
            workingDir = "%pathToProjectApp%"
            scriptContent = """
                rm -rf *.zip
                zip -r Launcher_mac.zip "Launcher.app"
            """.trimIndent()
        }
    }

    failureConditions {
        errorMessage = true
    }

    requirements {
        exists("env.macos")
    }
})
