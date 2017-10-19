package dava_framework

import dava_framework.buildTypes.*
import jetbrains.buildServer.configs.kotlin.v10.*
import jetbrains.buildServer.configs.kotlin.v10.Project

object Project : Project({
    uuid = "ecdef047-68ff-4000-9bde-e6e0e695b38d"
    extId = "dava_framework"
    parentId = "dava"
    name = "dava.framework"

    buildType(dava_framework_NewBuilds_PullRequestTest)
    buildType(dava_framework_Doxygen)

    template(dava_framework_TemplateDAVABase)
    template(dava_framework_TemplateDavaTools_win)
    template(dava_framework_TemplateDAVATools_mac)

    params {
        param("branch_specification", """
            +:(refs/pull-requests/*/merge)
            +:(refs/pull-requests/*/from)
            +:refs/heads/*
        """.trimIndent())
        select("config.cmake_bin", "%config.framework_dir%/Bin/CMakeWin32/bin/cmake.exe", display = ParameterDisplay.PROMPT,
                options = listOf("cmake", "%config.framework_dir%/Bin/CMakeWin32/bin/cmake.exe"))
        param("config.framework_dir", "%system.teamcity.build.checkoutDir%/dava.framework")
        param("dava_dir", "%system.teamcity.build.checkoutDir%/dava.framework")
        param("dava_gen", "%system.teamcity.build.checkoutDir%/dava.framework/RepoTools/Scripts/dava_gen.py")
        param("DavaConfigWin", """C:\BuildAgent\teamcity-client\conf\DavaConfigQt56_msvc2017.in""")
        param("framework_name", "development")
        param("stash_restapi_login", "dava_teamcity")
        password("stash_restapi_password", "zxx38986f37ccea38c04e7be9a8371adc18")
        param("teamcity_restapi_login", "job-tcdava")
        password("teamcity_restapi_password", "zxx38986f37ccea38c04e7be9a8371adc18")
        select("VS_VERSION", "Visual Studio 15 2017", display = ParameterDisplay.PROMPT,
                options = listOf("Visual Studio 15 2017", "Visual Studio 12"))
        param("wgtf_default_branch", "dava/development")
    }

    cleanup {
        artifacts(days = 20)
    }
})
