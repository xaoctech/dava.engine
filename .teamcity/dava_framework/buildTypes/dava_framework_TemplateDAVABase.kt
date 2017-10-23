package dava_framework.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_TemplateDAVABase : Template({
    uuid = "c77d0745-1b12-4a78-85e8-ebe1f1addf92"
    extId = "dava_framework_TemplateDAVABase"
    name = "TemplateDAVABase"

    artifactRules = """
        %pathToProjectApp%/*.zip
        %pathToProjectBuild%/*.zip
    """.trimIndent()

    params {
        param("add_definitions", "-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%")
        param("appID", "%ProjectName%")
        param("baseArchiveNameWin", "%ProjectName%_win_")
        param("baseURLWin", "http://by1-davatool-01.corp.wargaming.local/dava.framework/win/Tools/%branchID%/")
        param("branchID", "%teamcity.build.branch%")
        param("buildsPathWin", "//by1-davatool-01/win/Tools/%branchID%")
        param("configPathWin", "//by1-davatool-01/win/launcher/launcher_config.yaml")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/Build/dava_gen.py")
        text("LAUNCHER_ENABLE", "1", allowEmpty = true)
        param("limit", "15")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Tools/%ProjectName%")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("platform", "windows")
        param("ProjectName", "")
        param("runPathWin", "%ProjectName%.exe")
        param("UNITY_BUILD", "true")
        checkbox("x64", "true", display = ParameterDisplay.PROMPT,
                  checked = "true", unchecked = "false")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "generate project"
            id = "RUNNER_469"
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
            name = "build"
            id = "RUNNER_471"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "cmake --build . --config Release"
        }
    }
})
