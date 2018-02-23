#pragma once

#include <ModuleManager/IModule.h>

namespace DAVA
{
class Engine;

/** VisualScriptModule module.
     Register systems and components for work with VisualScript.
*/
class VisualScriptModule : public IModule
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptModule, IModule);

public:
    VisualScriptModule(Engine* engine);
    ~VisualScriptModule() override = default;

    void Init() override;
    void Shutdown() override;
};
}
