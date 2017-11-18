package dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.schedule

object dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerformanceTestsIOS : BuildType({
    uuid = "fc1cf975-0cee-4fa0-88b2-e81bdcf76db9"
    extId = "dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerformanceTestsIOS"
    name = "PerformanceTests_iOS"

    artifactRules = """
        %pathToProjectRoot%artifacts
        %pathToProject%/Release-iphoneos/PerformanceTests.zip
    """.trimIndent()

    params {
        password("agent_password", "zxx52467cdee324aa9768b065e3e10cc85e2cca34bb02a162e9b2687c974d0760a2192f478978efc1c532e956cd1e9bea9d")
        param("branch_to_compare", "development")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/dava_gen.py")
        param("IOS_TOOLCHAIN", "%system.teamcity.build.checkoutDir%/Sources/CMake/Toolchains/ios.toolchain.cmake")
        param("pathToProject", "dava.framework/Programs/PerfomanceTests/_build")
        param("pathToProjectRoot", "dava.framework/Programs/PerfomanceTests/")
        param("ResourceEditor_add_definitions", "-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%")
        param("ResourceEditor_pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/ResourceEditor")
        param("ResourceEditor_pathToProjectBuild", "%system.teamcity.build.checkoutDir%/b_ResourceEditor")
        param("speedtree_branch", "trunk")
        param("statistic_end_time", "120000")
        param("statistic_start_time", "0")
        select("test", "All",
                options = listOf("All", "asia", "rudniki", "materials"))
        param("test_time", "120000")
        param("UNITY_BUILD", "true")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:.=>dava.framework")
        root(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.vcsRoots.dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_DavaFrameworkPerformanceTes, "+:.=>performance.test")
        root("dava_framework_DavaSpeedtreeBranch", "+:.=>/dava.speedtree")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "ResourceEditor generate project"
            workingDir = "%ResourceEditor_pathToProjectBuild%"
            scriptContent = """
                rm -rf *
                python %dava_gen% macos %ResourceEditor_pathToProject% --add_definitions=%ResourceEditor_add_definitions%
            """.trimIndent()
        }
        script {
            name = "ResourceEditor build"
            workingDir = "%ResourceEditor_pathToProjectBuild%"
            scriptContent = "xcodebuild -project ResourceEditor.xcodeproj -configuration Release"
        }
        script {
            name = "Unlock Keychain"
            scriptContent = "security unlock-keychain -p %agent_password% /Users/Admin/Library/Keychains/login.keychain"
        }
        script {
            name = "generate project"
            workingDir = "%pathToProject%"
            scriptContent = """
                export PATH=/Applications/CMake.app/Contents/bin:${'$'}PATH
                rm -rf *
                cmake -G"Xcode" \
                -DCMAKE_TOOLCHAIN_FILE=../../../Sources/CMake/Toolchains/ios.toolchain.cmake \
                -DTEAMCITY_DEPLOY=true \
                -DPERFORMANCE_TEST_DATA_SVN=%system.teamcity.build.checkoutDir%/performance.test \
                -DRES_EDITOR_PATH=%ResourceEditor_pathToProjectBuild%/app \
                -DGPU_PARAM=PowerVR_iOS \
                -DENABLE_MEM_PROFILING=true \
                -DUNITY_BUILD=true \
                ..
                
                rm -rf %system.teamcity.build.checkoutDir%/%pathToProject%/PerformanceTests.app
            """.trimIndent()
        }
        script {
            name = "build project"
            workingDir = "%pathToProject%"
            scriptContent = """xcodebuild -project PerformanceTests.xcodeproj -configuration Release CODE_SIGN_IDENTITY="iPhone Developer" DEVELOPMENT_TEAM="9KMD79CS7L""""
        }
        script {
            name = "deploy and run"
            workingDir = "%pathToProjectRoot%/scripts"
            scriptContent = """
                cp -R %system.teamcity.build.checkoutDir%/%pathToProject%/Release-iphoneos/PerformanceTests.app %system.teamcity.build.checkoutDir%/%pathToProject%/PerformanceTests.app
                
                python start_tests.py --build release --platform ios --branch %teamcity.build.branch% --without-ui --test %test% --test-time %test_time%
            """.trimIndent()
        }
        script {
            name = "pack app"
            workingDir = "%pathToProject%/Release-iphoneos"
            scriptContent = """
                rm -rf *.zip
                zip -r PerformanceTests.zip PerformanceTests.app
            """.trimIndent()
        }
        script {
            name = "create_html_report"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "%pathToProjectRoot%artifacts"
            scriptContent = """
                cp ../../../../performance.test/Data/create_report.py create_report.py
                python create_report.py --branch %teamcity.build.branch%
            """.trimIndent()
        }
    }

    triggers {
        schedule {
            enabled = false
            schedulingPolicy = daily {
                hour = 23
            }
            triggerBuild = always()
            withPendingChangesOnly = false
            param("revisionRule", "lastFinished")
            param("dayOfWeek", "Sunday")
        }
    }

    dependencies {
        artifacts("dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerformanceTestsIOS") {
            buildRule = lastSuccessful("%branch_to_compare%")
            cleanDestination = true
            artifactRules = "*.txt => %pathToProjectRoot%old_artifacts"
        }
    }

    requirements {
        exists("env.UNIT_TEST")
        doesNotEqual("system.agent.name", "by2-badava-mac-07", "RQ_32")
    }
    
    disableSettings("RQ_32")
})
