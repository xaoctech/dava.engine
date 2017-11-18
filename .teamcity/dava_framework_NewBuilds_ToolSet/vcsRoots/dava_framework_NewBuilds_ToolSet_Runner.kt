package dava_framework_NewBuilds_ToolSet.vcsRoots

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.vcs.SvnVcsRoot

object dava_framework_NewBuilds_ToolSet_Runner : SvnVcsRoot({
    uuid = "3bbca5c7-265a-4974-b977-f63d4f96007f"
    extId = "dava_framework_NewBuilds_ToolSet_Runner"
    name = "runner"
    url = "https://svn2.wargaming.net/svn/dava_framework/dava.test/runner/"
    userName = "%teamcity_login%"
    password = "zxx38986f37ccea38c04e7be9a8371adc18"
})
