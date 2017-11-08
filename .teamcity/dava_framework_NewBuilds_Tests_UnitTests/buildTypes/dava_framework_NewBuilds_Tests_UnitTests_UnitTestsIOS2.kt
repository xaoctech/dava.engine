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
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2 : BuildType({
    uuid = "4975d025-16c7-44b7-b763-4aed3727fd43"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2"
    name = "UnitTests_iOS"


    params {
        password("agent_password", "zxxddcc8bc1d3b46b3e775d03cbe80d301b")
        param("check_folders", "Sources/Internal;Modules;Sources/CMake;Programs/UnitTests")
        param("env.build_failed", "true")
        param("env.build_required", "true")
        param("env.from_commit", "0")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/UnitTests/build")
        param("teamcity_restapi_login", "dava_teamcity")
        param("teamcity_restapi_password", "zxx38986f37ccea38c04e7be9a8371adc18")
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
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
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
                python RepoTools/Scripts/delete_folder.py %pathToProject%
                
                fi
            """.trimIndent()
        }
        script {
            name = "generate project"
            workingDir = "%pathToProject%"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                export PATH=/Applications/CMake.app/Contents/bin:${'$'}PATH
                cmake -G"Xcode" \
                -DCMAKE_TOOLCHAIN_FILE=../../../Sources/CMake/Toolchains/ios.toolchain.cmake \
                -DTEAMCITY_DEPLOY=true \
                -DDISABLE_MEMORY_PROFILER=true \
                ..
                fi
            """.trimIndent()
        }
        script {
            name = "Unlock Keychain"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                security unlock-keychain -p %agent_password% /Users/Admin/Library/Keychains/login.keychain
                fi
            """.trimIndent()
        }
        script {
            name = "build project"
            workingDir = "%pathToProject%"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                xcodebuild -project UnitTests.xcodeproj -configuration Release CODE_SIGN_IDENTITY="iPhone Developer" DEVELOPMENT_TEAM="9KMD79CS7L"
                fi
            """.trimIndent()
        }
        script {
            name = "deploy and run"
            workingDir = "dava.framework/Programs/UnitTests/scripts"
            scriptContent = """
                if [ "true" == "%env.build_required%" ]
                then
                cp -R %pathToProject%/Release-iphoneos/UnitTests.app %pathToProject%/UnitTests.app
                python start_unit_tests.py ios --teamcity
                fi
            """.trimIndent()
        }
        script {
            name = "CHANGE_SUCCESSFUL_BUILD_DESCRIPTION"
            enabled = false
            workingDir = "Teamcity"
            scriptContent = """
                if [ "false" == "%env.build_required%" ]
                then
                
                if [ -f "run_build.py" ]
                then
                
                python run_build.py --teamcity_url https://teamcity2.wargaming.net \
                --login "%teamcity_restapi_login%" \
                --password "%teamcity_restapi_password%" \
                --branch "%teamcity.build.branch%" \
                --agent_name "%teamcity.agent.name%" \
                --queue_at_top true \
                --configuration_name dava_framework_TeamcityTools_ChangeBuildDescription \
                --properties param.commit:%build.vcs.number.dava_DavaFrameworkStash%,param.configuration_id:%system.teamcity.buildType.id%,param.root_build_id:%teamcity.build.id%
                
                fi
                
                
                fi
            """.trimIndent()
        }
        script {
            name = "report commit status SUCCESSFUL"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
    }

    triggers {
        vcs {
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
        doesNotEqual("system.agent.name", "by2-badava-mac-07", "RQ_54")
    }
    
    disableSettings("RQ_54")
})
