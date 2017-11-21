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

object dava_framework_NewBuilds_ToolSet_ToolSetMac : BuildType({
    template(dava_framework.buildTypes.dava_framework_TemplateDAVATools_mac)
    uuid = "a3f20776-ef99-4e39-bd04-4e5fac191e7e"
    extId = "dava_framework_NewBuilds_ToolSet_ToolSetMac"
    name = "ToolSet_mac"

    artifactRules = """
        %pathToOutPackDir%/*.zip
        %pathToProjectBuild%/Coverage
        %pathToProjectBuild%/MODULES_LOG.txt
        %pathToProjectBuild%/Info.json
    """.trimIndent()

    params {
        param("add_definitions", "-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%,-DIGNORE_FILE_TREE_CHECK=true")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/dava_gen.py")
        param("env.build_failed", "true")
        param("env.from_commit", "0")
        param("env.LANG", "en_US.UTF-8")
        param("LAUNCHER_ENABLE", "0")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/Toolset")
        param("pathToProjectApp_other", "%pathToProjectBuild%/app_other")
        param("ProjectName", "ToolSet")
        text("speedtree_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
    }

    vcs {
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools", "+:Teamcity => Teamcity")

    }

    steps {
        script {
            name = "get stash commit"
            id = "RUNNER_625"
            workingDir = "dava.framework"
            scriptContent = "python %system.teamcity.build.checkoutDir%/Teamcity/get_pull_requests_commit.py --branch %teamcity.build.branch%"
        }
        script {
            name = "report commit status INPROGRESS"
            id = "RUNNER_626"
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
        }
        script {
            name = "UnitTest"
            id = "RUNNER_15"
            workingDir = "%pathToProjectApp_other%"
            scriptContent = "python %system.teamcity.build.checkoutDir%/dava.framework/Programs/UnitTests/scripts/start_unit_tests.py --teamcity"
        }
        script {
            name = "SelfTest"
            id = "RUNNER_956"
            workingDir = "%pathToProjectApp%"
            scriptContent = """
                ./ResourceEditor.app/Contents/MacOS/ResourceEditor --selftest
                ./QuickEd.app/Contents/MacOS/QuickEd --selftest
            """.trimIndent()
        }
        script {
            name = "report commit status SUCCESSFUL"
            id = "RUNNER_627"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            id = "RUNNER_629"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
        stepsOrder = arrayListOf("RUNNER_625", "RUNNER_626", "RUNNER_1085", "RUNNER_132", "RUNNER_133", "RUNNER_134", "RUNNER_135", "RUNNER_15", "RUNNER_956", "RUNNER_627", "RUNNER_629")
    }

    triggers {
        vcs {
            id = "vcsTrigger"
            branchFilter = "+:<default>"
        }
    }

    failureConditions {
        executionTimeoutMin = 160
        errorMessage = true
    }

    features {
        feature {
            id = "BUILD_EXT_7"
            type = "teamcity.stash.status"
            enabled = false
            param("stash_host", "https://%stash_hostname%")
            param("stash_only_latest", "true")
            param("stash_username", "dava_teamcity")
            param("stash_vcsignorecsv", "dava.framework_wgtf_stash")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxx38986f37ccea38c0775d03cbe80d301b")
        }
        commitStatusPublisher {
            id = "BUILD_EXT_28"
            enabled = false
            vcsRootExtId = "dava_DavaFrameworkStash"
            publisher = bitbucketServer {
                url = "https://%stash_hostname%"
                userName = "dava_teamcity"
                password = "zxx38986f37ccea38c0775d03cbe80d301b"
            }
        }
    }
    
    disableSettings("RUNNER_131")
})
