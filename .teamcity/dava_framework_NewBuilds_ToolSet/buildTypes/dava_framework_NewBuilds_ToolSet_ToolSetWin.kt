package dava_framework_NewBuilds_ToolSet.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.commitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_NewBuilds_ToolSet_ToolSetWin : BuildType({
    template(dava_framework.buildTypes.dava_framework_TemplateDavaTools_win)
    uuid = "191a53b3-3a4a-4c0d-8aec-6bebec5048c0"
    extId = "dava_framework_NewBuilds_ToolSet_ToolSetWin"
    name = "ToolSet_win"

    artifactRules = """
        %pathToOutPackDir%/*.zip
        %pathToProjectBuild%/MODULES_LOG.txt
        %pathToProjectBuild%/Info.json
    """.trimIndent()

    params {
        param("add_definitions", "-DDAVA_MEMORY_PROFILER=0,-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=1,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%,-DIGNORE_FILE_TREE_CHECK=1,-DCHECK_DEPENDENT_FOLDERS=1,-DTEAMCITY_URL=https://teamcity2.wargaming.net,-DSTASH_URL=https://stash.wargaming.net,-DTEAMCITY_LOGIN=%teamcity_restapi_login%,-DTEAMCITY_PASS=%teamcity_restapi_password%,-DSTASH_LOGIN=%stash_restapi_login%,-DSTASH_PASS=%stash_restapi_password%,-DFRAMEWORK_BRANCH=%teamcity.build.branch%")
        param("appID", "%ProjectName%")
        param("baseArchiveNameWin", "%ProjectName%_win_")
        param("baseURLWin", "http://by1-davatool-01.corp.wargaming.local/dava.framework/win/Tools/%branchID%/")
        text("beast_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
        param("branchID", "%teamcity.build.branch%")
        param("buildsPathWin", "//by1-davatool-01/win/Tools/%branchID%")
        param("configPathWin", "//by1-davatool-01/win/launcher/launcher_config.yaml")
        param("env.build_failed", "true")
        param("env.from_commit", "0")
        text("LAUNCHER_ENABLE", "0", allowEmpty = true)
        param("limit", "15")
        param("pathToOutPackDir", "%system.teamcity.build.checkoutDir%/b_%ProjectName%_pack")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/Toolset")
        param("pathToProjectApp", "%pathToProjectBuild%/app")
        param("pathToProjectApp_other", "%pathToProjectBuild%/app_other")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("platform", "windows")
        param("ProjectName", "ToolSet")
        param("runPathWin", "%ProjectName%.exe")
        text("speedtree_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
        checkbox("TEST", "false",
                  checked = "true", unchecked = "false")
        checkbox("UNITY_BUILD", "true",
                  checked = "true", unchecked = "false")
        checkbox("use_incredi_build", "true",
                  checked = "true", unchecked = "false")
        checkbox("x64", "true", display = ParameterDisplay.PROMPT,
                  checked = "true", unchecked = "false")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")
        root("dava_framework_DavaResourceeditorBeastBranch", "+:.=>/dava.resourceeditor.beast")
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools", "+:Teamcity => Teamcity")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "get stash commit"
            workingDir = "dava.framework"
            scriptContent = "python %system.teamcity.build.checkoutDir%/Teamcity/get_pull_requests_commit.py --branch %teamcity.build.branch%"
        }
        script {
            name = "report commit status INPROGRESS"
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
        }
        script {
            name = "Clear"
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
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python create_version_h.py --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number% --branch_info %teamcity.build.branch%"
        }
        script {
            name = "generate project"
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
            workingDir = "%dava_scripts_dir%"
            scriptContent = "python pack_app.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number%"
        }
        script {
            name = "UnitTest"
            workingDir = "%pathToProjectBuild%/app_other"
            scriptContent = "python start_tests.py"
        }
        script {
            name = "report commit status SUCCESSFUL"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
    }

    triggers {
        vcs {
            branchFilter = "+:<default>"
        }
    }

    failureConditions {
        executionTimeoutMin = 120
        errorMessage = true
    }

    features {
        feature {
            type = "teamcity.stash.status"
            enabled = false
            param("stash_host", "https://stash.wargaming.net")
            param("stash_only_latest", "true")
            param("stash_username", "dava_teamcity")
            param("stash_vcsignorecsv", "dava.framework_wgtf_stash")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxx38986f37ccea38c0775d03cbe80d301b")
        }
        commitStatusPublisher {
            enabled = false
            vcsRootExtId = "dava_DavaFrameworkStash"
            publisher = bitbucketServer {
                url = "https://stash.wargaming.net"
                userName = "dava_teamcity"
                password = "zxx38986f37ccea38c0775d03cbe80d301b"
            }
        }
    }

    requirements {
        doesNotEqual("system.agent.name", "by1-badava-win-16", "RQ_52")
        exists("MSBuildTools4.0_x86_Path")
    }
    
    disableSettings("RQ_52", "RUNNER_9")
})
