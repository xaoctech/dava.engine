#pragma once

#include "FileSystem/FilePath.h"

namespace DAVA
{
class ProgramOptions;
}

class SceneConsoleHelper
{
public:
    static bool InitializeQualitySystem(const DAVA::ProgramOptions& options, const DAVA::FilePath& targetPathname);
    static DAVA::FilePath CreateQualityPathname(const DAVA::FilePath& qualityPathname,
                                                const DAVA::FilePath& targetPathname = "",
                                                const DAVA::FilePath& resourceFolder = "");
    static void FlushRHI();
};
