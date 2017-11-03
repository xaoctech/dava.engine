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
        param("add_definitions", "-DDAVA_MEMORY_PROFILER=0,-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%,-DIGNORE_FILE_TREE_CHECK=true")
        text("beast_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
        param("env.build_failed", "true")
        param("env.from_commit", "0")
        param("LAUNCHER_ENABLE", "0")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/Toolset")
        param("pathToProjectApp_other", "%pathToProjectBuild%/app_other")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_%ProjectName%")
        param("ProjectName", "ToolSet")
        text("speedtree_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
        checkbox("TEST", "false",
                  checked = "true", unchecked = "false")
        param("UNITY_BUILD", "true")
        param("use_incredi_build", "false")
    }

    vcs {
        root("dava_framework_DavaResourceeditorBeastBranch", "+:.=>/dava.resourceeditor.beast")
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools", "+:Teamcity => Teamcity")

    }

     steps {
        script {
            name = "get stash commit"
            id = "RUNNER_633"
            workingDir = "dava.framework"
            scriptContent = "python %system.teamcity.build.checkoutDir%/Teamcity/get_pull_requests_commit.py --branch %teamcity.build.branch%"
        }
        script {
            name = "report commit status INPROGRESS"
            id = "RUNNER_645"
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
        }
        script {
            name = "Copy build result"
            id = "RUNNER_205"
            workingDir = "%pathToProjectApp_other%"
            scriptContent = """
                rmdir "%dava_dir%/Programs/UnitTests/Release" /S /Q
                xcopy "%pathToProjectApp_other%" "%dava_dir%/Programs/UnitTests/Release" /I /S /H /E /R /Y
            """.trimIndent()
        }
        script {
            name = "UnitTest"
            id = "RUNNER_14"
            workingDir = "dava.framework/Programs/UnitTests/scripts"
            scriptContent = """
                python %system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/rm.py %pathToProjectApp_other%/UnitTests %system.teamcity.build.checkoutDir%/dava.framework/Programs/UnitTests/Release
                python start_unit_tests.py --teamcity
            """.trimIndent()
        }
        script {
            name = "SelfTest"
            id = "RUNNER_957"
            workingDir = "%pathToProjectApp%"
            scriptContent = """
                ResourceEditor.exe --selftest
                QuickEd.exe --selftest
            """.trimIndent()
        }
        script {
            name = "report commit status SUCCESSFUL"
            id = "RUNNER_647"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            id = "RUNNER_648"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
        stepsOrder = arrayListOf("RUNNER_633", "RUNNER_645", "RUNNER_1083", "RUNNER_20", "RUNNER_82", "RUNNER_106", "RUNNER_205", "RUNNER_117", "RUNNER_14", "RUNNER_957", "RUNNER_647", "RUNNER_648")
    }

    triggers {
        vcs {
            id = "vcsTrigger"
            branchFilter = "+:<default>"
        }
    }

    failureConditions {
        executionTimeoutMin = 120
        errorMessage = true
    }

    features {
        feature {
            id = "BUILD_EXT_8"
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
            id = "BUILD_EXT_27"
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
    }
    
    disableSettings("RQ_52", "RUNNER_9")
})
