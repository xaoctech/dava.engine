#pragma once

#include "Reflection/Reflection.h"
#include "UI/Flow/UIFlowService.h"

namespace DAVA
{
class Engine;
class EngineContext;

class EngineUIService : public UIFlowService
{
    DAVA_VIRTUAL_REFLECTION(EngineUIService, UIFlowService);

public:
    const Engine* GetEngine() const;
    const EngineContext* GetEngineContext() const;
};
}