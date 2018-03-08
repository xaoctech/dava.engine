#pragma once

#include "FeatureManager/FeatureManager.h"

#include "Platform/Process.h"

namespace DAVA
{
class FeatureManagerUtils
{
public:
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_LINUX__)
    void InitLocalFeatureServer(const String& configFolder);
    void ShutdownLocalFeatureServer();
    void DownloadConfig();

private:
    Process* procStart = nullptr;
#endif
};
}
