package dava_framework_Editors_ResourceEditorPublic.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Editors_ResourceEditorPublic_ResourceEditorWin : BuildType({
    template(dava_framework.buildTypes.dava_framework_TemplateDavaTools_win)
    uuid = "68ed3638-d2d5-4e40-b495-025b0617e27a"
    extId = "dava_framework_Editors_ResourceEditorPublic_ResourceEditorWin"
    name = "ResourceEditor_win"


    params {
        param("add_definitions", "-DPUBLIC_BUILD=true,-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_WIN=%DavaConfigWin%")
        param("appID", "RE")
        text("beast_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "ResourceEditor")
        param("runPathWin", "ResourceEditor/dava.framework/Tools/ResourceEditor/ResourceEditor.exe")
        text("speedtree_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
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
