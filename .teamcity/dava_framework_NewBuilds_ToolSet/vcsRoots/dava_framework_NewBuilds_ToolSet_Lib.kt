package dava_framework_NewBuilds_ToolSet.vcsRoots

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.vcs.SvnVcsRoot

object dava_framework_NewBuilds_ToolSet_Lib : SvnVcsRoot({
    uuid = "6bfd10ba-a821-4440-af03-b8bfffb04a24"
    extId = "dava_framework_NewBuilds_ToolSet_Lib"
    name = "lib"
    url = "https://svn2.wargaming.net/svn/dava_framework/dava.test/lib/"
    userName = "%teamcity_login%"
    password = "zxx38986f37ccea38c04e7be9a8371adc18"
})
