#pragma once

#include <ModuleManager/IModule.h>

namespace DAVA
{
class Engine;

class SpineModule : public IModule
{
public:
    SpineModule(Engine* engine);
    ~SpineModule() override = default;

    void Init() override;
    void Shutdown() override;
};
}