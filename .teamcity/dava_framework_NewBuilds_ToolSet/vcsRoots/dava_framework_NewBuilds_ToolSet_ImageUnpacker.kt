package dava_framework_NewBuilds_ToolSet.vcsRoots

import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.vcs.SvnVcsRoot

object dava_framework_NewBuilds_ToolSet_ImageUnpacker : SvnVcsRoot({
    uuid = "115268cf-28b5-4664-9f1a-51160888f647"
    extId = "dava_framework_NewBuilds_ToolSet_ImageUnpacker"
    name = "ImageUnpacker"
    url = "https://svn2.wargaming.net/svn/dava_framework/dava.test/ImageUnpacker"
    userName = "%teamcity_login%"
    password = "zxx38986f37ccea38c04e7be9a8371adc18"
})
