package dava_framework_Editors_UIViewer.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature
import jetbrains.buildServer.configs.kotlin.v10.BuildFeature.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Editors_UIViewer_UIViewerIOS : BuildType({
    uuid = "9b65475c-bf5d-45ac-b451-90b81c5138ba"
    extId = "dava_framework_Editors_UIViewer_UIViewerIOS"
    name = "UIViewer_iOS"

    artifactRules = """
        %pathToProject%/app/*
        menu.html
        *.plist
        Desc_*.txt
        %pathToProject%/app/Desc_*.txt
    """.trimIndent()

    params {
        param("app_name", "UIViewer")
        param("build_url", "https://teamcity2.wargaming.net/repository/download/%system.teamcity.buildType.id%/%teamcity.build.id%:id")
        param("bundle_id", "net.wargaming.UIViewer")
        param("IOS_TOOLCHAIN", "%system.teamcity.build.checkoutDir%/Sources/CMake/Toolchains/ios.toolchain.cmake")
        param("ipa_name", "%app_name%")
        param("pack app", "%pathToProject%/app")
        param("pathToOutPackDir", "%pathToProject%/app")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/Programs/UIViewer/build")
        param("pathToProjectApp", "%pathToProject%/app")
        param("plist_name", "%app_name%")
        param("ProjectName", "UIViewer")
    }

    vcs {
        root("dava_DavaFrameworkStash")

        checkoutMode = CheckoutMode.ON_AGENT
        cleanCheckout = true
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
            name = "build project"
            workingDir = "%pathToProject%"
            scriptContent = """xcodebuild -project UIViewer.xcodeproj -configuration Release CODE_SIGN_IDENTITY="iPhone Developer" DEVELOPMENT_TEAM="9KMD79CS7L""""
        }
        script {
            name = "create ipa"
            scriptContent = "xcrun -sdk iphoneos PackageApplication -v %pathToProject%/app/UIViewer.app -o %pathToProject%/app/UIViewer.ipa"
        }
        script {
            enabled = false
            workingDir = "%pathToProject%/app"
            scriptContent = """
                rm -rf *.zip
                zip -r SceneViewer.zip SceneViewer.ipa
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
                echo \<string\>SceneViewer\</string\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</array\> >> %plist_name%.plist
                echo \</dict\> >> %plist_name%.plist
                echo \</plist\> >> %plist_name%.plist
            """.trimIndent()
        }
        script {
            name = "create desc"
            workingDir = "RepoTools/Scripts"
            scriptContent = "python create_pack_file.py --app_name %ProjectName% --out_path %pathToOutPackDir% --app_path %pathToProjectApp% --dava_path %system.teamcity.build.checkoutDir% --build_number %build.number%"
        }
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
        exists("env.UNIT_TEST")
    }
})
