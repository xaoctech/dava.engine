package dava_framework_NewBuilds_Tests_TestBed.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_Tests_TestBed_Ios : BuildType({
    uuid = "1675494a-0988-4770-8323-6122945a7ab5"
    extId = "dava_framework_NewBuilds_Tests_TestBed_Ios"
    name = "TestBed_iOS"

    artifactRules = """
        %pathToProject%/app/TestBed.*
        menu.html
        *.plist
    """.trimIndent()

    params {
        param("app_name", "TestBed")
        param("build_url", "https://teamcity2.wargaming.net/repository/download/%system.teamcity.buildType.id%/%teamcity.build.id%:id")
        param("bundle_id", "net.wargaming.testbed")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/dava_gen.py")
        param("IOS_TOOLCHAIN", "%system.teamcity.build.checkoutDir%/Sources/CMake/Toolchains/ios.toolchain.cmake")
        param("ipa_name", "%app_name%")
        param("pack app", "%pathToProject%/app")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/Programs/TestBed/build")
        param("plist_name", "%app_name%")
    }

    vcs {
        root("dava_DavaFrameworkStash")

        checkoutMode = CheckoutMode.ON_SERVER
    }

    steps {
        script {
            name = "generate project"
            workingDir = "%pathToProject%"
            scriptContent = """
                export PATH=/Applications/CMake.app/Contents/bin:${'$'}PATH
                rm -rf *
                cmake -G"Xcode" \
                -DCMAKE_TOOLCHAIN_FILE=../../../Sources/CMake/Toolchains/ios.toolchain.cmake \
                -DDEPLOY=true \
                -DUNITY_BUILD=true \
                ..
            """.trimIndent()
        }
        script {
            name = "build project (old)"
            workingDir = "%pathToProject%"
            scriptContent = """xcodebuild -project TestBed.xcodeproj -configuration Release CODE_SIGN_IDENTITY="iPhone Developer" DEVELOPMENT_TEAM="9KMD79CS7L""""
        }
        script {
            name = "build project"
            enabled = false
            workingDir = "%pathToProject%"
            scriptContent = "python %system.teamcity.build.checkoutDir%/Bin/RepoTools/Scripts/dava_build_wrapper.py --config=Release --teamcityLog=true --pathToDava=%system.teamcity.build.checkoutDir% --pathToBuild=%pathToProject%"
        }
        script {
            name = "create ipa"
            enabled = false
            scriptContent = "xcrun -sdk iphoneos PackageApplication -v %system.teamcity.build.workingDir%/%pathToProject%/TestBed.app -o %system.teamcity.build.workingDir%/%pathToProject%/TestBed.ipa"
        }
        script {
            workingDir = "%pathToProject%/app"
            scriptContent = """
                rm -rf *.zip
                zip -r TestBed.zip TestBed.ipa
            """.trimIndent()
        }
        script {
            name = "add link to report"
            scriptContent = """
                rm -rf ./menu.html
                
                echo \<html\> >> menu.html
                echo \<body\> >> menu.html
                echo \<br/\> >> menu.html
                
                echo \<br/\> >> menu.html
                echo \<a href=\"itms-services://?action=download-manifest\&url=%build_url%/%app_name%.plist\"\>Install %system.build.number%\<\/a\> >> menu.html
                echo \<br/\> >> menu.html
                
                echo \</body\> >> menu.html
                echo \</html\> >> menu.html
            """.trimIndent()
        }
        script {
            name = "create plist iOS"
            scriptContent = """
                rm -rf ./%app_name%.plist
                
                echo "create_plist.sh %plist_name% %ipa_name% %build_url% %bundle_id%" 
                
                echo \<?xml version=\"1.0\" encoding=\"UTF-8\"?\> >> %plist_name%.plist
                echo \<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"\> >> %plist_name%.plist
                echo \<plist version=\"1.0\"\> >> %plist_name%.plist
                echo \<dict\> >> %plist_name%.plist
                echo \<key\>items\</key\> >> %plist_name%.plist
                echo \<array\> >> %plist_name%.plist
                echo \<dict\> >> %plist_name%.plist
                echo \<key\>assets\</key\> >> %plist_name%.plist
                echo \<array\> >> %plist_name%.plist
                echo \<dict\> >> %plist_name%.plist
                echo \<key\>kind\</key\> >> %plist_name%.plist
                echo \<string\>software-package\</string\> >> %plist_name%.plist
                echo \<key\>%build_url%\</key\> >> %plist_name%.plist
                echo \<string\>%build_url%/%ipa_name%.ipa\</string\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</array\> >> %plist_name%.plist
                echo \<key\>metadata\</key\> >> %plist_name%.plist
                echo \<dict\> >> %plist_name%.plist
                echo \<key\>bundle-identifier\</key\> >> %plist_name%.plist
                echo \<string\>%bundle_id%\</string\> >> %plist_name%.plist
                echo \<key\>kind\</key\> >> %plist_name%.plist
                echo \<string\>software\</string\> >> %plist_name%.plist
                echo \<key\>title\</key\> >> %plist_name%.plist
                echo \<string\>WoT Blitz\</string\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</array\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</plist\> >> %plist_name%.plist
            """.trimIndent()
        }
    }

    failureConditions {
        executionTimeoutMin = 10
    }

    features {
        feature {
            type = "teamcity.stash.status"
            param("stash_host", "https://stash-dava.wargaming.net")
            param("stash_only_latest", "true")
            param("stash_username", "i_petrochenko")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxxf12c6f6e95b4c11aad2927fa4df2c366")
        }
    }

    requirements {
        exists("env.macos")
        exists("env.UNIT_TEST", "RQ_53")
    }
    
    disableSettings("RQ_53")
})
