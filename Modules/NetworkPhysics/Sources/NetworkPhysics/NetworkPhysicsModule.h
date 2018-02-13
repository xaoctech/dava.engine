#pragma once

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
class NetworkPhysicsModule : public IModule
{
public:
    NetworkPhysicsModule(Engine* engine);

    void Init() override;
    void Shutdown() override;

    DAVA_VIRTUAL_REFLECTION(NetworkPhysicsModule, IModule);
};
}
