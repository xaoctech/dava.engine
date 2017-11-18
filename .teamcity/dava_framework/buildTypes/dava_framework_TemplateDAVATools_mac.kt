package dava_framework.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_TemplateDAVATools_mac : Template({
    uuid = "4a33ec0a-df60-483d-928d-546579da23c8"
    extId = "dava_framework_TemplateDAVATools_mac"
    name = "TemplateDAVATools_mac"

    artifactRules = "%pathToOutPackDir%/*.zip"

    params {
        param("add_definitions", "-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%")
        param("appID", "%ProjectName%")
        param("baseArchiveNameMac", "%ProjectName%_mac_")
        param("baseURLMac", "http://by1-davatool-01.corp.wargaming.local/dava.framework/mac/Tools/%branchID%/")
        param("branchID", "%teamcity.build.branch%")
        param("buildsPathMac", "~/mac/Tools/%branchID%")
        param("configPathMac", "~/mac/launcher/launcher_config.yaml")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/Build/dava_gen.py")
        text("LAUNCHER_ENABLE", "1", allowEmpty = true)
        param("limit", "15")
        param("pathToOutPackDir", "%system.teamcity.build.checkoutDir%/b_%ProjectName%_pack")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Tools/%ProjectName%")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("platform", "macos")
        param("ProjectName", "")
        param("runPathMac", "%ProjectName%.app")
        param("runPathWin", "%ProjectName%.exe")
        param("UNITY_BUILD", "true")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "clear"
            id = "RUNNER_1085"
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
            id = "RUNNER_132"
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python create_version_h.py --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number% --branch_info %teamcity.build.branch%"
        }
        script {
            name = "generate project"
            id = "RUNNER_133"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "python %dava_gen% %platform% %pathToProject% --add_definitions=%add_definitions%"
        }
        script {
            name = "build"
            id = "RUNNER_134"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "python %dava_dir%/Bin/RepoTools/Scripts/dava_build_wrapper.py --config=Release --teamcityLog=true --pathToDava=%dava_dir% --pathToBuild=%pathToProjectBuild%"
        }
        script {
            name = "PackApp"
            id = "RUNNER_135"
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python pack_app.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number%"
        }
    }

    requirements {
        exists("env.macos", "RQ_161")
    }
})
