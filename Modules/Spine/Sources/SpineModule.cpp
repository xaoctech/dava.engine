#include "SpineModule.h"
#include "UI/Spine/UISpineComponent.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
SpineModule::SpineModule(Engine * engine)
    : IModule(engine)
{
    const EngineContext* context = engine->GetContext();

    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISpineComponent, "Spine");
    
    ComponentManager* cm = context->componentManager;
    cm->RegisterComponent<UISpineComponent>();
}

void SpineModule::Init()
{
}

void SpineModule::Shutdown()
{
}
}