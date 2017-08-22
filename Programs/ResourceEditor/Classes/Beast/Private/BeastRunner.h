#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include "Beast/BeastProxy.h"

#include <TArc/WindowSubSystem/UI.h>

#include <FileSystem/FilePath.h>

#include <memory>

namespace DAVA
{
class Scene;

namespace TArc
{
class WaitHandle;
}
}

class BeastManager;
class BeastProxy;

class BeastRunner final
{
public:
    BeastRunner(BeastProxy* proxy, DAVA::Scene* scene, const DAVA::FilePath& scenePath, const DAVA::FilePath& outputPath, eBeastMode mode, std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle);
    ~BeastRunner();

    void RunUIMode();

    void Start();
    bool Process();
    void Finish(bool canceled);

private:
    void PackLightmaps();
    DAVA::FilePath GetLightmapDirectoryPath() const;

private:
    BeastManager* beastManager = nullptr;
    BeastProxy* beastProxy = nullptr;

    std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle;

    DAVA::Scene* workingScene = nullptr;
    const DAVA::FilePath scenePath;
    DAVA::FilePath outputPath;
    DAVA::uint64 startTime = 0;
    eBeastMode beastMode = eBeastMode::MODE_LIGHTMAPS;

    bool cancelledManually = false;
};

#endif //#if defined (__DAVAENGINE_BEAST__)
