package dava_framework_Launcher.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Launcher_LauncherWin : BuildType({
    uuid = "f47b9439-16fa-4666-8e75-7735e0d02dcd"
    extId = "dava_framework_Launcher_LauncherWin"
    name = "Launcher_win"

    artifactRules = "%pathToProject%/Launcher.zip"
    buildNumberPattern = "1.0.%build.counter%"

    params {
        param("add_definitions", "-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%")
        param("framework_name", "development")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("platform", "windows")
        param("ProjectName", "Launcher")
        param("UNITY_BUILD", "true")
        checkbox("x64", "true", display = ParameterDisplay.PROMPT,
                  checked = "true", unchecked = "false")
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
                del /f /s /q /a %pathToProjectApp%
                del /f /s /q /a %pathToProjectBuild%
                
                if %x64% == true (
                  python %dava_gen% %platform% x64 %pathToProject% --add_definitions=%add_definitions%
                ) else (
                  python %dava_gen% %platform% %pathToProject% --add_definitions=%add_definitions%
                )
            """.trimIndent()
        }
        script {
            name = "build Launcher"
            workingDir = "%pathToProjectBuild%"
            scriptContent = """
                echo %config.cmake_bin% --build %pathToProjectBuild% --config Release
                %config.cmake_bin% --build %pathToProjectBuild% --config Release
            """.trimIndent()
        }
        script {
            name = "self test"
            workingDir = "%pathToProjectApp%"
            scriptContent = "Launcher.exe -s"
        }
        script {
            name = "packApp"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = "python zip.py --base_dir %pathToProjectApp% --archive_name %pathToProject%/Launcher.zip"
        }
    }

    failureConditions {
        errorMessage = true
    }

    requirements {
        exists("MSBuildTools4.0_x86_Path")
    }
})
