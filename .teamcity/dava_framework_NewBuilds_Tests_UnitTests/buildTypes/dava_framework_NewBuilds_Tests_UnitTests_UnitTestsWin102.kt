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

object dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102 : BuildType({
    uuid = "a409c079-d840-4f15-bad6-3b437f396cac"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102"
    name = "UnitTests_Win10"

    artifactRules = "%pathToProject%/AppPackages/%projectName%.zip"

    params {
        param("check_folders", "Sources/Internal;Modules;Sources/CMake;Programs/UnitTests")
        param("env.build_failed", "true")
        param("env.build_required", "true")
        param("env.from_commit", "0")
        checkbox("packProject", "no",
                  checked = "yes", unchecked = "no")
        param("pathToBin", """%system.teamcity.build.checkoutDir%\Tools\Bin""")
        param("pathToCmakeList", """Programs\%projectName%""")
        param("pathToProject", """%pathToCmakeList%\Release""")
        param("projectName", "UnitTests")
    }

    vcs {
        root("dava_DavaFrameworkStash")
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools", "+:Teamcity => Teamcity")

        checkoutMode = CheckoutMode.ON_AGENT
        cleanCheckout = true
        checkoutDir = "%system.teamcity.buildConfName%"
    }

    steps {
        script {
            name = "get stash commit"
            scriptContent = "python Teamcity/get_pull_requests_commit.py --branch %teamcity.build.branch%"
        }
        script {
            name = "report commit status INPROGRESS"
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status INPROGRESS --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% In progress ...""""
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
            scriptContent = """python run_build_depends_of_folders.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --framework_branch %teamcity.build.branch% --check_folders "%check_folders%" --root_configuration_id %teamcity.build.id%"""
        }
        script {
            name = "clear"
            enabled = false
            scriptContent = """
                if "true" == "%env.build_required%" (
                git clean -d -x -f
                )
            """.trimIndent()
        }
        script {
            name = "Generating project"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                %system.teamcity.build.checkoutDir%/Programs/WinStoreCMake/mk_project.bat %system.teamcity.build.checkoutDir%\%pathToCmakeList% %system.teamcity.build.checkoutDir%
                )
            """.trimIndent()
        }
        script {
            name = "Building project for platform x86"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                cmake --build . --config Release -- /p:Platform=x86 /p:GenerateAppxPackageOnBuild=Never
                )
            """.trimIndent()
        }
        script {
            name = "Generating project unity build"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                %system.teamcity.build.checkoutDir%/Programs/WinStoreCMake/mk_project.bat %system.teamcity.build.checkoutDir%\%pathToCmakeList% %system.teamcity.build.checkoutDir% UnityBuild
                )
            """.trimIndent()
        }
        script {
            name = "Building project for platform x64"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                cmake --build . --config Release -- /p:Platform=x64 /p:GenerateAppxPackageOnBuild=Never
                )
            """.trimIndent()
        }
        script {
            name = "Building project for platform arm"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                cmake --build . --config Release -- /p:Platform=arm /p:GenerateAppxPackageOnBuild=Never
                )
            """.trimIndent()
        }
        script {
            name = "Creating bundle"
            workingDir = "%pathToProject%"
            scriptContent = """
                if "true" == "%env.build_required%" (
                rm -rf AppPackages
                rm -rf BundleArtifacts
                cmake --build . --config Release -- /p:AppxBundle=Always /p:AppxBundlePlatforms="x86|x64|ARM"
                )
            """.trimIndent()
        }
        script {
            name = "Packing artefacts"
            workingDir = "%pathToProject%/AppPackages"
            scriptContent = """if "%packProject%" == "yes" (zip -0 -r %projectName%.zip %projectName%/)"""
        }
        script {
            name = "Deploying & running (x64)"
            enabled = false
            workingDir = "Projects/UnitTests/scripts"
            scriptContent = "python start_unit_tests.py uwp x64 --teamcity"
        }
        script {
            name = "Deploying & running (ARM)"
            enabled = false
            workingDir = "Projects/UnitTests/scripts"
            scriptContent = "python start_unit_tests.py uwp arm --teamcity"
        }
        script {
            name = "CHANGE_SUCCESSFUL_BUILD_DESCRIPTION"
            enabled = false
            workingDir = "Teamcity"
            scriptContent = """
                if "false" == "%env.build_required%" (
                if exist "run_build.py" (
                python run_build.py --teamcity_url https://teamcity2.wargaming.net --login "%teamcity_restapi_login%" --password "%teamcity_restapi_password%" --branch "%teamcity.build.branch%" --agent_name "%teamcity.agent.name%" --queue_at_top true --configuration_name dava_framework_TeamcityTools_ChangeBuildDescription --properties param.commit:%build.vcs.number.dava_DavaFrameworkStash%,param.configuration_id:%system.teamcity.buildType.id%,param.root_build_id:%teamcity.build.id%
                )
                )
            """.trimIndent()
        }
        script {
            name = "report commit status SUCCESSFUL"
            executionMode = BuildStep.ExecutionMode.RUN_ON_SUCCESS
            workingDir = "Teamcity"
            scriptContent = """
                echo "##teamcity[setParameter name='env.build_failed' value='false']"
                python report_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status SUCCESSFUL --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Good job !"
            """.trimIndent()
        }
        script {
            name = "report commit status FAILED"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """python report_build_status.py --reported_status %env.build_failed% --teamcity_url https://teamcity2.wargaming.net --stash_url https://%stash_hostname% --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --status FAILED --root_build_id %teamcity.build.id% --configuration_name %system.teamcity.buildType.id% --commit %env.from_commit% --abbreviated_build_name true --description "%teamcity.build.branch% Need to work!""""
        }
    }

    triggers {
        vcs {
            branchFilter = "+:<default>"
        }
    }

    failureConditions {
        errorMessage = true
    }

    features {
        feature {
            type = "teamcity.stash.status"
            enabled = false
            param("stash_host", "https://%stash_hostname%")
            param("stash_only_latest", "true")
            param("stash_username", "dava_teamcity")
            param("stash_failCancelledBuilds", "true")
            param("secure:stash_username", "zxx38986f37ccea38c0775d03cbe80d301b")
        }
        commitStatusPublisher {
            enabled = false
            vcsRootExtId = "dava_DavaFrameworkStash"
            publisher = bitbucketServer {
                url = "https://%stash_hostname%"
                userName = "dava_teamcity"
                password = "zxx38986f37ccea38c0775d03cbe80d301b"
            }
        }
    }

    requirements {
        exists("env.windows10")
    }
})
