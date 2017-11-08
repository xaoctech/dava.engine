package dava_framework.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_NewBuilds_PullRequestTest : BuildType({
    uuid = "4ef8150f-54a8-43ca-9847-caa8045c3323"
    extId = "dava_framework_NewBuilds_PullRequestTest"
    name = "Pull-request_TEST"


    params {
        param("branch_specification", """
            +:(refs/pull-requests/*/merge)
            +:refs/heads/*
        """.trimIndent())
        param("reverse.dep.*.branch_specification", """
            +:(refs/pull-requests/*/merge)
            +:refs/heads/*
        """.trimIndent())
        text("reverse.dep.*.client_branch", "<default>", label = "бранч игры", description = "<имя_бранча>  или refs/pull-requests/<номер_пулреквеста>/from", display = ParameterDisplay.PROMPT, allowEmpty = true)
    }

    vcs {
        root("dava_framework_UIEditor_BuildmachineWargamingNetTools")

        checkoutMode = CheckoutMode.ON_SERVER
    }

    steps {
        script {
            name = "Install pip modules"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = """
                pip install --upgrade pip
                pip install -r requirements.txt
            """.trimIndent()
        }
        script {
            name = "Send Build Status to Slack"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            scriptContent = "curl http://ba-manager.wargaming.net/modules/bot_slack/build_notify.php?build_id=%teamcity.build.id%"
        }
        script {
            name = "Update depend build status"
            executionMode = BuildStep.ExecutionMode.ALWAYS
            workingDir = "Teamcity"
            scriptContent = "python update_depend_build_status.py --teamcity_url https://teamcity2.wargaming.net --stash_url https://stash-dava.wargaming.net --stash_login %stash_restapi_login%  --stash_password %stash_restapi_password% --teamcity_login %teamcity_restapi_login% --teamcity_password %teamcity_restapi_password% --container_configuration_id %teamcity.build.id% --costum_dependent_build dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS"
        }
    }

    dependencies {
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_FormatTestTmp) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_RequestWotbUnitTestsMacOS) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsAndroid) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsIOS2) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsLinux) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin102) {
            snapshot {
                reuseBuilds = ReuseBuilds.NO
                onDependencyCancel = FailureAction.ADD_PROBLEM
            }
        }
        dependency(dava_framework_NewBuilds_Tests_UnitTests.buildTypes.dava_framework_NewBuilds_Tests_UnitTests_UnitTestsWin32) {
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
        doesNotContain("teamcity.agent.name", "by2-badava-mac-22")
    }
})
