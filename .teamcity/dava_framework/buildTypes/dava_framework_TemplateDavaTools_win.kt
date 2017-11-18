package dava_framework.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_TemplateDavaTools_win : Template({
    uuid = "2ec2bbc5-a9d5-4ad7-9dcb-63f7b7d5cafd"
    extId = "dava_framework_TemplateDavaTools_win"
    name = "TemplateDAVATools_win"

    artifactRules = "%pathToOutPackDir%/*.zip"

    params {
        param("add_definitions", "-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%")
        param("appID", "%ProjectName%")
        param("baseArchiveNameWin", "%ProjectName%_win_")
        param("baseURLWin", "http://by1-davatool-01.corp.wargaming.local/dava.framework/win/Tools/%branchID%/")
        param("branchID", "%teamcity.build.branch%")
        param("buildsPathWin", "//by1-davatool-01/win/Tools/%branchID%")
        param("configPathWin", "//by1-davatool-01/win/launcher/launcher_config.yaml")
        text("LAUNCHER_ENABLE", "1", allowEmpty = true)
        param("limit", "15")
        param("pathToOutPackDir", "%system.teamcity.build.checkoutDir%/b_%ProjectName%_pack")
        param("pathToProject", "%dava_dir%/Programs/%ProjectName%")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectBuild", "%pathToProject%/_build")
        param("platform", "windows")
        param("ProjectName", "")
        param("runPathWin", "%ProjectName%.exe")
        checkbox("UNITY_BUILD", "true",
                  checked = "true", unchecked = "false")
        checkbox("use_incredi_build", "false",
                  checked = "true", unchecked = "false")
        checkbox("x64", "true", display = ParameterDisplay.PROMPT,
                  checked = "true", unchecked = "false")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "Clear"
            id = "RUNNER_1083"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = """
                git clean -d -x -f
                
                python delete_folder.py %pathToOutPackDir%
                python delete_folder.py %pathToProjectApp%
                python delete_folder.py %pathToProjectBuild%
            """.trimIndent()
        }
        script {
            name = "create version.h"
            id = "RUNNER_20"
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python create_version_h.py --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number% --branch_info %teamcity.build.branch%"
        }
        script {
            name = "generate project"
            id = "RUNNER_82"
            workingDir = "%pathToProjectBuild%"
            scriptContent = """
                if %x64% == true (
                    python %dava_gen% %platform% x64 %pathToProject% --add_definitions=%add_definitions%
                ) else (
                    python %dava_gen% %platform% %pathToProject% --add_definitions=%add_definitions%
                )
            """.trimIndent()
        }
        script {
            name = "build"
            id = "RUNNER_106"
            workingDir = "%pathToProjectBuild%"
            scriptContent = """
                if "%use_incredi_build%" == "true" (
                    if "%VS_VERSION%" == "Visual Studio 12" (
                        BuildConsole %ProjectName%.sln /cfg="RelWithDebinfo|x64" /VsVersion="vc12"
                    ) else (
                        BuildConsole %ProjectName%.sln /cfg="RelWithDebinfo|x64" /VsVersion="vc15"
                    )
                )
                
                %config.cmake_bin% --build . --config RelWithDebinfo
            """.trimIndent()
        }
        script {
            name = "PackApp"
            id = "RUNNER_117"
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python pack_app.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number%"
        }
    }

    requirements {
        exists("MSBuildTools4.0_x86_Path", "RQ_228")
    }
})
