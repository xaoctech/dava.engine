package dava_framework_NewBuilds_ToolSet.vcsRoots

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.vcs.SvnVcsRoot

object dava_framework_NewBuilds_ToolSet_Tests : SvnVcsRoot({
    uuid = "abab834e-a27a-4d4a-9a23-da0f5ec50df8"
    extId = "dava_framework_NewBuilds_ToolSet_Tests"
    name = "tests"
    url = "https://svn2.wargaming.net/svn/dava_framework/dava.test/tests/"
    userName = "%teamcity_login%"
    password = "zxx38986f37ccea38c04e7be9a8371adc18"
})
