package dava_framework_NewBuilds_Tests_UnitTests.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.ScheduleTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.schedule

object dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework_2 : BuildType({
    uuid = "f4ff0594-66cd-4d8d-8d21-6b8e7ed3af07"
    extId = "dava_framework_NewBuilds_Tests_UnitTests_UploadDavaFramework_2"
    name = "Upload DAVA Framework_test"


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
                git remote set-url origin https://%remote_login%:%remote_pass%@github.com/dava/dava.framework.git
            """.trimIndent()
        }
        script {
            name = "Clearing"
            workingDir = "dava.framework/Bin/RepoTools/Scripts/GithubTools"
            scriptContent = "python github_preparation.py --path %dava_dir% --repo %dava_dir%"
        }
        script {
            name = "git push"
            enabled = false
            workingDir = "dava.framework"
            scriptContent = "git push dava_git development --force"
        }
    }

    triggers {
        schedule {
            enabled = false
            schedulingPolicy = daily {
                hour = 0
            }
            triggerBuild = always()
            withPendingChangesOnly = false
            param("dayOfWeek", "Sunday")
        }
    }

    requirements {
        equals("system.agent.name", "by2-badava-mac-07")
    }
})
