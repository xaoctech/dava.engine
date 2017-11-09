package dava_framework_NewBuilds_Tests_TestBed.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_Tests_TestBed_Android : BuildType({
    uuid = "f50fc7e7-54fc-42e9-95ab-8d969f01cb9f"
    extId = "dava_framework_NewBuilds_Tests_TestBed_Android"
    name = "TestBed_Android"

    artifactRules = "%dava_dir%/Programs/TestBed/Platforms/Android/TestBed/build/outputs/apk/TestBed-fat-release.apk"

    params {
        param("ANDROID_NATIVE_API_LEVEL", "14")
        param("ANDROID_TARGET_API_LEVEL", "23")
        param("branch_name", "trunk")
        param("branches_or_tags", "branches")
        param("env.PATH", "/Users/Admin/.rvm/gems/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/gems/ruby-2.0.0-p0@global/bin:/Users/Admin/.rvm/rubies/ruby-2.0.0-p0/bin:/Users/Admin/.rvm/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:/usr/local/git/bin:/opt/local/bin:/Users/Admin/QtSDK/Desktop/Qt/474/gcc/bin:/Users/Admin/Downloads/Development/android-ndk-r10:/Users/Admin/AIRSDK_Compiler/bin:/Users/Admin/Downloads/gradle-1.7/bin:/Users/Admin/Downloads/Development/sdk/platform-tools/")
        password("key.alias", "zxx09a921466e3f12ec775d03cbe80d301b")
        password("key.store", "zxx81afd735ea35e25d775d03cbe80d301b")
        checkbox("memory_leaks_test", "", display = ParameterDisplay.HIDDEN,
                  checked = "false")
        select("NDK_TYPE", "CRYSTAX", display = ParameterDisplay.PROMPT,
                options = listOf("CRYSTAX", "GOOGLE"))
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/TestBed/_build")
        param("project.branch_name", "%branch_name%")
        param("resources_branch_name", "trunk")
        param("resources_dir", "%svn_dir%/%resources_branch_name%/resources")
        param("resources_svn", "https://svn2.wargaming.net/svn/blitz/%branches_or_tags%/%resources_branch_name%/resources")
        param("svn_dir", "%teamcity.agent.home.dir%/../svn")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:=>/dava.framework")

        cleanCheckout = true
    }

    steps {
        script {
            name = "clear"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = "python delete_folder.py %pathToProject%"
        }
        script {
            name = "generate properties"
            workingDir = "dava.framework/Programs/TestBed/Platforms/Android"
            scriptContent = """
                echo sdk.dir=%env.ANDROID_STUDIO_SDK% >> local.properties
                echo ndk.dir=%env.ANDROID_STUDIO_NDK% >> local.properties
            """.trimIndent()
        }
        script {
            name = "gradle wrapper"
            workingDir = "dava.framework/Programs/TestBed/Platforms/Android"
            scriptContent = "gradle wrapper"
        }
        script {
            name = "Gradlew build"
            workingDir = "dava.framework/Programs/TestBed/Platforms/Android"
            scriptContent = "./gradlew TestBed:assembleFatRelease"
        }
    }

    failureConditions {
        executionTimeoutMin = 60
    }

    features {
        feature {
            type = "teamcity.stash.status"
            param("stash_host", "https://%stash_hostname%")
            param("stash_only_latest", "true")
            param("stash_username", "i_petrochenko")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxxf12c6f6e95b4c11aad2927fa4df2c366")
        }
    }

    requirements {
        contains("system.agent.name", "mac")
    }
})
