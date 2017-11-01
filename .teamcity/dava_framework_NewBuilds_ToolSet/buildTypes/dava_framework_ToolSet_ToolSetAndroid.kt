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

object dava_framework_ToolSet_ToolSetAndroid : BuildType({
    uuid = "a4724ae3-4c02-42e5-95af-5cc3c178218b"
    extId = "dava_framework_ToolSet_ToolSetAndroid"
    name = "ToolSet_Android"

    artifactRules = """
        dava.framework/Programs/UnitTests/Platforms/Android/UnitTests/build/outputs/apk/UnitTests-fat-release.apk
        dava.framework/Programs/SceneViewer/Platforms/Android/SceneViewer/build/outputs/apk/SceneViewer-fat-release.apk
        dava.framework/Programs/TestBed/Platforms/Android/TestBed/build/outputs/apk/TestBed-fat-release.apk
        dava.framework/Programs/UIViewer/Platforms/Android/UIViewer/build/outputs/apk/UIViewer-fat-release.apk
        dava.framework/Programs/PerfomanceTests/Platforms/Android/PerfomanceTests/build/outputs/apk/PerfomanceTests-fat-release.apk
    """.trimIndent()

    params {
        param("env.build_failed", "true")
        param("env.build_required", "true")
        param("env.from_commit", "0")
        param("env.PATH", "/Library/Frameworks/Python.framework/Versions/2.7/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0@global/bin:/Users/Admin/.rvm/rubies/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:/usr/local/git/bin:/opt/local/bin:/Users/Admin/QtSDK/Desktop/Qt/474/gcc/bin:/Users/Admin/Downloads/Development/android-ndk-r10:/Users/Admin/AIRSDK_Compiler/bin:/Users/Admin/Downloads/gradle-1.7/bin:/Users/Admin/Downloads/Development/sdk/platform-tools/")
        select("NDK_TYPE", "GOOGLE", display = ParameterDisplay.PROMPT,
                options = listOf("CRYSTAX", "GOOGLE"))
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:=>/dava.framework")
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
            name = "Install pip modules"
            workingDir = "Teamcity"
            scriptContent = """
                pip install --upgrade pip
                pip install -r requirements.txt
            """.trimIndent()
        }
        script {
            name = "clear"
            workingDir = "dava.framework"
            scriptContent = "git clean -d -x -f"
        }
        script {
            name = "Gradle build"
            workingDir = "dava.framework/Programs/Toolset/Scripts"
            scriptContent = "python android_build.py --sdk_dir %env.ANDROID_STUDIO_SDK% --ndk_dir %env.ANDROID_STUDIO_NDK%"
        }
        script {
            name = "UnitTest"
            workingDir = "dava.framework/Programs/Toolset/Scripts"
            scriptContent = "python start_tests.py --platform ANDROID --sdk_dir %env.ANDROID_STUDIO_SDK% --davaRoot %system.teamcity.build.checkoutDir%/dava.framework"
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
            enabled = false
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
        exists("env.UNIT_TEST")
        doesNotEqual("system.agent.name", "by2-badava-mac-08")
        doesNotEqual("system.agent.name", "by2-badava-mac-11", "RQ_87")
    }
})
