package dava_framework.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_Doxygen : BuildType({
    uuid = "0dce8ccd-c14f-4cb5-b389-846323b9f5bf"
    extId = "dava_framework_Doxygen"
    name = "Doxygen"
    description = "Generating documentation with doxygen"

    artifactRules = """Doc\Doxygen\doxygen\html => html"""

    vcs {
        root("dava_DavaFrameworkStash")

        checkoutMode = CheckoutMode.ON_AGENT
    }

    steps {
        script {
            name = "doxygen"
            workingDir = "Doc/Doxygen"
            scriptContent = """"../../Bin/doxygen/doxygen""""
        }
    }

    requirements {
        equals("env.OS", "Windows_NT")
    }
})
