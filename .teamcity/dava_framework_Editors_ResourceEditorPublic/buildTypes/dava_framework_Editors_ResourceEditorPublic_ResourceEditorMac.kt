package dava_framework_Editors_ResourceEditorPublic.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Editors_ResourceEditorPublic_ResourceEditorMac : BuildType({
    template(dava_framework.buildTypes.dava_framework_TemplateDAVATools_mac)
    uuid = "db086cbd-7fbb-4e40-8ef8-5d0f9806342a"
    extId = "dava_framework_Editors_ResourceEditorPublic_ResourceEditorMac"
    name = "ResourceEditor_mac"

    params {
        param("add_definitions", "-DPUBLIC_BUILD=true,-DQT_VERSION=%QT_VERSION%,-DUNITY_BUILD=%UNITY_BUILD%,-DDEPLOY=true,-DCUSTOM_DAVA_CONFIG_PATH_MAC=%DavaConfigMac%")
        param("appID", "RE")
        param("pathToProject", "%system.teamcity.build.checkoutDir%/dava.framework/Programs/%ProjectName%")
        param("ProjectName", "ResourceEditor")
        text("speedtree_branch", "trunk", display = ParameterDisplay.PROMPT, allowEmpty = true)
    }

    steps {
        script {
            name = "SelfTest"
            id = "RUNNER_954"
            enabled = false
            workingDir = "%pathToProjectApp%"
            scriptContent = "./ResourceEditor.app/Contents/MacOS/ResourceEditor --selftest"
        }
        stepsOrder = arrayListOf("RUNNER_1085", "RUNNER_132", "RUNNER_133", "RUNNER_134", "RUNNER_135", "RUNNER_954")
    }
})
