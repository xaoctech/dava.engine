#pragma once

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class ModuleManager;
class NetworkCoreDebugOverlayItem;

class NetworkCoreModule : public IModule
{
    DAVA_VIRTUAL_REFLECTION(NetworkCoreModule, IModule);

public:
    NetworkCoreModule(Engine* engine);

    void Init() override;
    void Shutdown() override;

private:
    std::unique_ptr<NetworkCoreDebugOverlayItem> netDebugOverlay;
};
};
