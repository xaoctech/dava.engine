package dava_framework_TeamcityTools.buildTypes

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.ScriptBuildStep.*
import jetbrains.buildServer.configs.kotlin.v10.buildSteps.script

object dava_framework_WoTBlitzStash_MergeSvnTrunkToStashMaster : BuildType({
    uuid = "be8eb4ad-246e-4ed6-8ca1-8d427ca2f58a"
    extId = "dava_framework_WoTBlitzStash_MergeSvnTrunkToStashMaster"
    name = "MergeSvnTrunkToStashMaster"


    vcs {
        checkoutMode = CheckoutMode.ON_SERVER
    }

    steps {
        script {
            workingDir = "/Users/Admin/git-svn"
            scriptContent = "sh wot.blitz_svn2git.sh"
        }
    }

    requirements {
        matches("system.agent.name", "by2-badava-mac-07")
    }
})
