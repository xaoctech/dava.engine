package dava_framework_NewBuilds_Tests_UnitTests.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.CommitStatusPublisher.*
import jetbrains.buildServer.configs.kotlin.v10.buildFeatures.commitStatusPublisher
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ExecBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ExecBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.exec
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid : BuildType({
    uuid = "126b0ce6-8b5b-4502-bfc1-ae4e9986438a"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid"
    name = "UnitTests_Android"

    artifactRules = "dava.framework/Programs/UnitTests/Platforms/Android/UnitTests/build/outputs/apk/UnitTests-fat-release.apk"

    params {
        param("check_folders", "Sources/Internal;Modules;Sources/CMake;Programs/UnitTests")
        param("env.build_failed", "true")
        param("env.build_required", "true")
        param("env.from_commit", "0")
        param("env.PATH", "/Library/Frameworks/Python.framework/Versions/2.7/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0@global/bin:/Users/Admin/.rvm/rubies/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:/usr/local/git/bin:/opt/local/bin:/Users/Admin/QtSDK/Desktop/Qt/474/gcc/bin:/Users/Admin/Downloads/Development/android-ndk-r10:/Users/Admin/AIRSDK_Compiler/bin:/Users/Admin/Downloads/gradle-1.7/bin:/Users/Admin/Downloads/Development/sdk/platform-tools/")
        select("NDK_TYPE", "GOOGLE", display = ParameterDisplay.PROMPT,
                options = listOf("CRYSTAX", "GOOGLE"))
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/UnitTests/_build")
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
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
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
            name = "Run build depends of folders"
            enabled = false
            workingDir = "Teamcity"
            scriptContent = """python run_build_depends_of_folders.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --framework_branch %teamcity.build.branch% --check_folders "%check_folders%" --root_configuration_id %teamcity.build.id%"""
        }
        script {
            name = "clear"
            workingDir = "dava.framework"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                git clean -d -x -f
                fi
            """.trimIndent()
        }
        script {
            name = "remove old apk"
            workingDir = "%pathToProject%"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                rm -rf bin/*-release.apk
                rm -rf libs
                fi
            """.trimIndent()
        }
        script {
            name = "Generate properties"
            workingDir = "dava.framework/Programs/UnitTests/Platforms/Android"
            scriptContent = """
                echo sdk.dir=%env.ANDROID_STUDIO_SDK% >> local.properties
                echo ndk.dir=%env.ANDROID_STUDIO_NDK% >> local.properties
            """.trimIndent()
        }
        script {
            name = "gradle wrapper"
            workingDir = "dava.framework/Programs/UnitTests/Platforms/Android"
            scriptContent = "gradle wrapper"
        }
        script {
            name = "Gradle build"
            workingDir = "dava.framework/Programs/UnitTests/Platforms/Android"
            scriptContent = "./gradlew UnitTests:assembleFatRelease"
        }
        exec {
            name = "uninstall apk"
            workingDir = "dava.framework/"
            path = "%env.HOME%/Downloads/Development/sdk/platform-tools/adb"
            arguments = "uninstall com.dava.UnitTestApp"
            param("script.content", "%env.HOME%/Downloads/Development/sdk/platform-tools/adb")
        }
        script {
            name = "deploy apk"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                %env.HOME%/Downloads/Development/sdk/platform-tools/adb -d install -r %dava_dir%/Programs/UnitTests/Platforms/Android/UnitTests/build/outputs/apk/UnitTests-fat-release.apk
                fi
            """.trimIndent()
        }
        script {
            name = "start tests on device"
            workingDir = "dava.framework/Programs/UnitTests/scripts"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                python start_unit_tests.py android --teamcity
                fi
            """.trimIndent()
        }
        script {
            name = "report commit status SUCCESSFUL"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
    }

    triggers {
        vcs {
            enabled = false
            branchFilter = "+:<default>"
        }
    }

    failureConditions {
        executionTimeoutMin = 30
        errorMessage = true
    }

    features {
        feature {
            type = "teamcity.stash.status"
            enabled = false
            param("stash_host", "https://stash-dava.wargaming.net")
            param("stash_only_latest", "true")
            param("stash_username", "dava_teamcity")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxx38986f37ccea38c0775d03cbe80d301b")
        }
        commitStatusPublisher {
            enabled = false
            vcsRootExtId = "dava_DavaFrameworkStash"
            publisher = bitbucketServer {
                url = "https://stash-dava.wargaming.net"
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
    
    disableSettings("RQ_87")
})
