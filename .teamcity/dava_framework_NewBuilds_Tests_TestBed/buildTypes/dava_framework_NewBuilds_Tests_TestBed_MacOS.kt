package dava_framework_NewBuilds_Tests_TestBed.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_Tests_TestBed_MacOS : BuildType({
    uuid = "3f2252ce-b5d2-4a86-b27b-0914a22b1419"
    extId = "dava_framework_NewBuilds_Tests_TestBed_MacOS"
    name = "TestBed_MacOS"

    artifactRules = "%pathToProjectBuild%/app/TestBed.zip"

    params {
        param("add_definitions", "-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%,-DIGNORE_FILE_TREE_CHECK=true")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/dava_gen.py")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/TestBed")
        param("pathToProjectBuild", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/TestBed/_build")
        param("platform", "macos")
        param("UNITY_BUILD", "true")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:. => dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
        cleanCheckout = true
    }

    steps {
        script {
            name = "generate project"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "python %dava_gen% %platform% %pathToProject% --add_definitions=%add_definitions%"
        }
        script {
            name = "build project"
            workingDir = "%pathToProjectBuild%"
            scriptContent = "python %dava_dir%/Bin/RepoTools/Scripts/dava_build_wrapper.py --config=Release --teamcityLog=true --pathToDava=%dava_dir% --pathToBuild=%pathToProjectBuild%"
        }
        script {
            name = "pack app"
            workingDir = "%pathToProjectBuild%/app"
            scriptContent = """
                rm -rf *.zip
                zip -r TestBed.zip TestBed.app/
            """.trimIndent()
        }
    }

    failureConditions {
        executionTimeoutMin = 120
    }

    features {
        feature {
            type = "teamcity.stash.status"
            param("stash_host", "https://stash.wargaming.net")
            param("stash_only_latest", "true")
            param("stash_username", "i_petrochenko")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxxf12c6f6e95b4c11aad2927fa4df2c366")
        }
    }

    requirements {
        exists("env.macos")
    }
})
