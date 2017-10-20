package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Editors_UIViewer_UIViewerAndroidCrystaXNdk : BuildType({
    uuid = "8cfe4fc8-575b-415c-a44a-22d8856efbb0"
    extId = "dava_framework_Editors_UIViewer_UIViewerAndroidCrystaXNdk"
    name = "UIViewer_Android_CrystaX_NDK"

    artifactRules = "%pathToProject%/binzip/*.zip"

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
        param("pathToOutPackDir", "%pathToProject%/binzip")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/UIViewer/_build")
        param("pathToProjectApp", "%pathToProject%/bin")
        param("project.branch_name", "%branch_name%")
        param("ProjectName", "UIViewer")
        param("resources_branch_name", "trunk")
        param("resources_dir", "%svn_dir%/%resources_branch_name%/resources")
        param("resources_svn", "https://svn2.wargaming.net/svn/blitz/%branches_or_tags%/%resources_branch_name%/resources")
        param("svn_dir", "%teamcity.agent.home.dir%/../svn")
    }

    vcs {
        root("dava_DavaFrameworkStash", "+:=>/dava.framework")

        checkoutMode = CheckoutMode.ON_AGENT
        cleanCheckout = true
    }

    steps {
        script {
            name = "create version.h"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = "python create_version_h.py --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number% --branch_info %teamcity.build.branch%"
        }
        script {
            name = "clear"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = "python delete_folder.py %pathToProject%"
        }
        script {
            name = "generate project"
            workingDir = "%pathToProject%"
            scriptContent = """
                export PATH=/Applications/CMake.app/Contents/bin:${'$'}PATH
                cmake -G"Unix Makefiles" \
                -DCMAKE_TOOLCHAIN_FILE=../../../Sources/CMake/Toolchains/android.toolchain.cmake \
                -DCUSTOM_DAVA_CONFIG_PATH=/Volumes/DavaConfigQt54.in \
                -DIGNORE_FILE_TREE_CHECK=true \
                -DDISABLE_MEMORY_PROFILER=true \
                -DANDROID_NDK_TYPE=%NDK_TYPE% \
                -DUNITY_BUILD=true \
                ..
                # YES second time call same command, android make toolchain fix
                cmake -G"Unix Makefiles" ..
            """.trimIndent()
        }
        script {
            name = "build project"
            workingDir = "%pathToProject%"
            scriptContent = """python %dava_dir%/Bin/RepoTools/Scripts/dava_build_wrapper.py --config=Release --teamcityLog=true --pathToDava=%dava_dir% --pathToBuild=%pathToProject% --nativeOptions="-j8""""
        }
        script {
            name = "pack app"
            workingDir = "dava.framework/RepoTools/Scripts"
            scriptContent = """
                mkdir %pathToProject%/tmp
                mv %pathToProject%/bin/UIViewerApp-release.apk %pathToProject%/tmp/UIViewerApp-release.apk
                rm -rf %pathToProject%/bin
                mkdir %pathToProject%/bin
                mv %pathToProject%/tmp/UIViewerApp-release.apk %pathToProject%/bin/UIViewerApp-release.apk
                
                python pack_app.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir%/dava.framework --build_number %build.number%
            """.trimIndent()
        }
    }

    failureConditions {
        executionTimeoutMin = 60
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
        exists("env.FRAMEWORK")
    }
})
