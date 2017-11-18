package dava_framework_EditorsTmp_TmpResourcePacker.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v10.triggers.VcsTrigger.*
import jetbrains.buildServer.configs.kotlin.v10.triggers.vcs

object dava_framework_EditorsTmp_TmpResourcePacker_AutotestsMac : BuildType({
    uuid = "767bfaeb-253c-45da-bc13-b72e4f7f47ef"
    extId = "dava_framework_EditorsTmp_TmpResourcePacker_AutotestsMac"
    name = "Autotests_mac"

    artifactRules = """
        Projects/ResPackerTest/*.html
        Projects/ResPackerTest/Artifacts/** => Artifacts
    """.trimIndent()

    params {
        param("link", "%teamcity.serverUrl%/repository/download/%system.teamcity.buildType.id%/%build.number%")
        param("mail_list", """"o_misyuchenko@wargaming.net,v_kleschenko@wargaming.net"""")
    }

    vcs {
        root("dava_DavaFrameworkStash")

        checkoutMode = CheckoutMode.ON_SERVER
    }

    steps {
        script {
            name = "adreno"
            workingDir = "Programs/ResPackerTest"
            scriptContent = "python test.py adreno %mail_list% %link% %framework_name% %build.vcs.number.dava_DavaFrameworkStash%%"
            param("command.parameters", "%")
        }
        script {
            name = "mali"
            workingDir = "Programs/ResPackerTest"
            scriptContent = """
                python test.py mali %mail_list% %link% %framework_name% %build.vcs.number.dava_DavaFrameworkStash%
                ##dava_framework_LocalBranchMask_EditorMask_DavaFrameworkBranchLocal
            """.trimIndent()
        }
        script {
            name = "PoverVR_iOS"
            workingDir = "Programs/ResPackerTest"
            scriptContent = "python test.py PowerVR_iOS %mail_list% %link% %framework_name% %build.vcs.number.dava_DavaFrameworkStash%%"
        }
        script {
            name = "PowerVR_Android"
            workingDir = "Programs/ResPackerTest"
            scriptContent = """
                python test.py PowerVR_Android %mail_list% %link% %framework_name% %system.build.vcs.number.dava_DavaFrameworkStash%
                ##dava_framework_LocalBranchMask_EditorMask_DavaFrameworkBranchLocal
            """.trimIndent()
        }
        script {
            name = "tegra"
            workingDir = "Programs/ResPackerTest"
            scriptContent = "python test.py tegra %mail_list% %link% %framework_name% %build.vcs.number.dava_DavaFrameworkStash%"
        }
        script {
            name = "Default"
            workingDir = "Programs/ResPackerTest"
            scriptContent = """
                python test.py Default %mail_list% %link% %framework_name% %system.build.vcs.number.dava_DavaFrameworkStash%
                ##dava_framework_LocalBranchMask_EditorMask_DavaFrameworkBranchLocal
            """.trimIndent()
        }
    }

    triggers {
        vcs {
            enabled = false
            triggerRules = "+:root=ResourcePacker_AutotestingFramework:/Tools/"
        }
    }

    requirements {
        contains("teamcity.agent.name", "by2-badava-mac-08")
    }
})
