package dava_framework_NewBuilds_Tests_UnitTests.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.commitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS : BuildType({
    uuid = "5651a1a2-c614-4c0a-808e-c38d1ba32dc4"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS"
    name = "REQUEST_WOTB_UnitTests"


    params {
        param("check_folders", "Sources/Internal;Modules;Sources/CMake")
        text("client_branch", "<default>", label = "бранч игры", description = "<имя_бранча>  или refs/pull-requests/<номер_пулреквеста>/from", display = ParameterDisplay.PROMPT, allowEmpty = true)
        param("request_configuration", "dava_framework_NewBuilds_WoTBlitz_Trun_430_Tests_UnitTestsMacOS")
    }

    vcs {
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools", "+:=>/BuildBTools")
        root("dava_DavaFrameworkStash", "+:=>/dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "Install pip modules"
            workingDir = "BuildBTools/Teamcity"
            scriptContent = """
                pip install --upgrade pip
                pip install -r requirements.txt
            """.trimIndent()
        }
        script {
            name = "Run build depends of folders"
            workingDir = "BuildBTools/Teamcity"
            scriptContent = """python run_build_depends_of_folders.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash.wargaming.net --configuration_name %request_configuration% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --framework_branch "%teamcity.build.branch%" --check_folders "%check_folders%" --client_branch "%client_branch%" --root_configuration_id "%teamcity.build.id%" --request_stash_mode true --convert_to_merge_requests false"""
        }
    }

    failureConditions {
        errorMessage = true
    }

    features {
        feature {
            type = "teamcity.stash.status"
            enabled = false
            param("stash_host", "https://stash.wargaming.net")
            param("stash_only_latest", "true")
            param("stash_username", "dava_teamcity")
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
        doesNotEqual("system.agent.name", "by2-badava-mac-22")
    }
})
