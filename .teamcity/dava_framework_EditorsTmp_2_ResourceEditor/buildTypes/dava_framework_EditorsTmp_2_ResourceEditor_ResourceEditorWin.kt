package dava_framework_EditorsTmp_2_ResourceEditor.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_EditorsTmp_2_ResourceEditor_ResourceEditorWin : BuildType({
    template = "dava_framework_TemplateDavaTools_win"
    uuid = "f390eb98-d10e-46b9-8300-4e6931315858"
    extId = "dava_framework_EditorsTmp_2_ResourceEditor_ResourceEditorWin"
    name = "ResourceEditor_win"


    params {
        param("add_definitions", "-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%")
        param("appID", "RE")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "ResourceEditor")
        param("runPathWin", "ResourceEditor/dava.framework/Tools/ResourceEditor/ResourceEditor.exe")
    }

    vcs {
        cleanCheckout = true
    }

    steps {
        script {
            name = "SelfTest"
            id = "RUNNER_955"
            enabled = false
            workingDir = "%pathToProjectApp%"
            scriptContent = """ResourceEditor\dava.framework\Tools\ResourceEditor\ResourceEditor.exe --selftest"""
        }
        stepsOrder = arrayListOf("RUNNER_1083", "RUNNER_20", "RUNNER_82", "RUNNER_106", "RUNNER_117", "RUNNER_955")
    }

    requirements {
        exists("env.windows", "RQ_256")
    }
})
