#include "UI/Flow/Services/EngineUIService.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(EngineUIService)
{
    ReflectionRegistrator<EngineUIService>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](EngineUIService* s) { delete s; })
    .Field("engine", &EngineUIService::GetEngine, nullptr)
    .Field("engineContext", &EngineUIService::GetEngineContext, nullptr)
    .End();
}

const Engine* EngineUIService::GetEngine() const
{
    return Engine::Instance();
}

const EngineContext* EngineUIService::GetEngineContext() const
{
    return GetEngine()->GetContext();
}
}