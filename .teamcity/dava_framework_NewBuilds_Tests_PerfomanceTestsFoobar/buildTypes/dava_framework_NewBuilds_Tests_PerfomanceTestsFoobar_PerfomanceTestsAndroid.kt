package dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ExecBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ExecBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.exec
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.schedule
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid : BuildType({
    uuid = "1c7cc66d-50b5-48fe-81eb-1dc3c287ee58"
    extId = "dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid"
    name = "PerfomanceTests_Android"

    artifactRules = """
        %pathToProjectRoot%artifacts
        dava.framework/Programs/PerfomanceTests/Platforms/Android/PerformanceTests/build/outputs/apk/PerformanceTests-fat-release.zip
    """.trimIndent()

    params {
        param("branch_to_compare", "development")
        param("env.PATH", "/Users/Admin/.rvm/gems/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0@global/bin:/Users/Admin/.rvm/rubies/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:/usr/local/git/bin:/opt/local/bin:/Users/Admin/QtSDK/Desktop/Qt/474/gcc/bin:/Users/Admin/Downloads/Development/android-ndk-r10:/Users/Admin/AIRSDK_Compiler/bin:/Users/Admin/Downloads/gradle-1.7/bin:/Users/Admin/Downloads/Development/sdk/platform-tools/")
        select("gpu", "adreno",
                options = listOf("mali", "origin", "adreno", "tegra", "PowerVR_Android"))
        param("pathToBin", """%system.teamcity.build.checkoutDir%\dava.framework\Tools\Bin""")
        param("pathToProject", "dava.framework/Programs/PerfomanceTests/_build")
        param("pathToProjectRoot", "dava.framework/Programs/PerfomanceTests/")
        param("project_name", "PerformanceTests")
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

        checkoutMode = CheckoutMode.ON_SERVER
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
            name = "remove old apk"
            workingDir = "%pathToProject%"
            scriptContent = """
                rm -rf bin/*-release.apk
                rm -rf libs
            """.trimIndent()
        }
        script {
            name = "Dava config update"
            workingDir = "dava.framework"
            scriptContent = """
                echo RES_EDITOR_PATH=%ResourceEditor_pathToProjectBuild%/app >> DavaConfig.in
                echo GPU_PARAM=%gpu% >> DavaConfig.in
            """.trimIndent()
        }
        script {
            name = "Generate properties"
            workingDir = "dava.framework/Programs/PerfomanceTests/Platforms/Android"
            scriptContent = """
                echo sdk.dir=%env.ANDROID_STUDIO_SDK% >> local.properties
                echo ndk.dir=%env.ANDROID_STUDIO_NDK% >> local.properties
            """.trimIndent()
        }
        script {
            name = "build project"
            workingDir = "dava.framework/Programs/PerfomanceTests/Platforms/Android"
            scriptContent = "./gradlew PerformanceTests:assembleFatRelease"
        }
        exec {
            name = "uninstall apk"
            workingDir = "dava.framework/"
            path = "%env.ANDROID_STUDIO_SDK%/platform-tools/adb"
            arguments = "uninstall com.dava.performancetests.PerformanceTestApp"
            param("script.content", "%env.HOME%/Downloads/Development/sdk/platform-tools/adb")
        }
        script {
            name = "deploy apk"
            workingDir = "dava.framework/Programs/PerfomanceTests/Platforms/Android/PerformanceTests/build/outputs/apk"
            scriptContent = "%env.ANDROID_STUDIO_SDK%/platform-tools/adb -d install -r PerformanceTests-fat-release.apk"
        }
        script {
            name = "start tests on device"
            workingDir = "%pathToProjectRoot%/scripts"
            scriptContent = "python start_tests.py --platform android --branch %teamcity.build.branch% --without-ui --test %test% --test-time %test_time%"
        }
        script {
            name = "pack app"
            workingDir = "dava.framework/Programs/PerfomanceTests/Platforms/Android/PerformanceTests/build/outputs/apk"
            scriptContent = """
                rm -rf *.zip
                zip -r PerformanceTests-fat-release.zip PerformanceTests-fat-release.apk
            """.trimIndent()
        }
        script {
            name = "create_html_report"
            workingDir = "%pathToProjectRoot%artifacts"
            scriptContent = """
                cp ../../../../performance.test/Data/create_report.py create_report.py
                python create_report.py --branch %teamcity.build.branch%
            """.trimIndent()
        }
    }

    triggers {
        vcs {
            enabled = false
        }
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
        artifacts("dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid") {
            buildRule = lastSuccessful("%branch_to_compare%")
            cleanDestination = true
            artifactRules = "*.txt => %pathToProjectRoot%old_artifacts"
        }
        artifacts(dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar.buildTypes.dava_framework_NewBuilds_Tests_PerfomanceTestsFoobar_PerfomanceTestsAndroid_old) {
            buildRule = lastSuccessful("%branch_to_compare%")
            cleanDestination = true
            artifactRules = "*.txt => %pathToProjectRoot%old_artifacts"
            enabled = false
        }
    }

    requirements {
        exists("env.UNIT_TEST")
    }
})
