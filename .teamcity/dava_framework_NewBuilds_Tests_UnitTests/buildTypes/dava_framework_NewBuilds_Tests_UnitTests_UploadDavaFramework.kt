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
            workingDir = "dava.framework"
            scriptContent = """
                git clone https://%stash_hostname%/scm/df/dava.framework.git .
                git lfs pull
                git remote add dava_github https://%remote_login%:%remote_pass%@github.com/dava/dava.framework.git
            """.trimIndent()
        }
        script {
            name = "Clearing development branch"
            workingDir = "dava.framework/Bin/RepoTools/Scripts/GithubTools"
            scriptContent = """
                git checkout -b development origin/development
                python github_preparation.py --path %dava_dir% --repo %dava_dir%
            """.trimIndent()
        }
        script {
            name = "Clearing server branch"
            workingDir = "dava.framework/Bin/RepoTools/Scripts/GithubTools"
            scriptContent = """
                git checkout -b server origin/server
                python github_preparation.py --path %dava_dir% --repo %dava_dir%
            """.trimIndent()
        }
        script {
            name = "Clearing new_render_ branch"
            workingDir = "dava.framework/Bin/RepoTools/Scripts/GithubTools"
            scriptContent = """
                git checkout -b new_render_ origin/new_render_
                python github_preparation.py --path %dava_dir% --repo %dava_dir%
            """.trimIndent()
        }
        script {
            name = "Git push to github"
            workingDir = "dava.framework"
            scriptContent = """
                git push -u dava_github development --force
                git push -u dava_github server --force
                git push -u dava_github new_render_ --force
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
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_Launcher.buildTypes.dava_framework_Launcher_LauncherWin) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_FormatTestTmp) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_ToolSet_ToolSetAndroid) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_ToolSet_ToolSetIos) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_ToolSet_ToolSetLinux) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_ToolSet_ToolSetWin10) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_NewBuilds_ToolSet_ToolSetWin32) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_NewBuilds_ToolSet_ToolSetMac) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_ToolSet.buildTypes.dava_framework_NewBuilds_ToolSet_ToolSetWin) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
    }

    requirements {
        equals("system.agent.name", "by2-badava-mac-07")
    }
})
