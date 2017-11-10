package dava_framework_NewBuilds_Tests_UnitTests.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.schedule

object dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework : BuildType({
    uuid = "6ce80ec3-cd5f-4717-be62-258c499d92ef"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework"
    name = "Upload DAVA Framework"


    params {
        password("remote_login", "zxxa9f6c31aad443105775d03cbe80d301b")
        password("remote_pass", "zxxce4d693c52a6529a7cba448ac4842296c3db07e945307465")
        param("submodule_path", "Sources/External/ngt")
    }

    vcs {
        checkoutMode = CheckoutMode.ON_AGENT
        cleanCheckout = true
    }

    steps {
        script {
            name = "git clone stash"
            enabled = false
            workingDir = "dava.framework"
            scriptContent = """
                git clone https://%stash_hostname%/scm/df/dava.framework.git .
                git remote add dava_git https://%remote_login%:%remote_pass%@github.com/dava/dava.framework.git
            """.trimIndent()
        }
        script {
            name = "Clearing"
            enabled = false
            workingDir = "dava.framework/Bin/RepoTools/Scripts/GithubTools"
            scriptContent = "python github_preparation.py --path %dava_dir% --repo %dava_dir%"
        }
        script {
            name = "git push"
            enabled = false
            workingDir = "dava.framework"
            scriptContent = """
                git pull dava_git development
                git push dava_git development --force
            """.trimIndent()
        }
    }

    triggers {
        schedule {
            schedulingPolicy = daily {
                hour = 0
            }
            branchFilter = "+:<default>"
            triggerBuild = always()
            withPendingChangesOnly = false
            param("revisionRule", "lastFinished")
            param("dayOfWeek", "Sunday")
        }
    }

    dependencies {
        dependency(dava_framework_Launcher.buildTypes.dava_framework_Launcher_LauncherMacOS) {
            snapshot {
            }
        }
        dependency(dava_framework_Launcher.buildTypes.dava_framework_Launcher_LauncherWin) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin32) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_NewBuilds_ToolSet_ToolSetMac) {
            snapshot {
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_NewBuilds_ToolSet_ToolSetWin) {
            snapshot {
            }
        }
    }

    requirements {
        equals("system.agent.name", "by2-badava-mac-07")
    }
})
