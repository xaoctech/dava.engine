#include "SpineModule.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/UISpineSystem.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControlSystem.h>
#include <UI/Styles/UIStyleSheetSystem.h>

namespace DAVA
{
SpineModule::SpineModule(Engine * engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISpineComponent, "Spine");
}

void SpineModule::Init()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();
    ComponentManager* cm = context->componentManager;
    cm->RegisterComponent<UISpineComponent>();

    UIControlSystem* cs = context->uiControlSystem;
    cs->AddSystem(std::make_unique<UISpineSystem>(), cs->GetStyleSheetSystem());
}

void SpineModule::Shutdown()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();
    UIControlSystem* cs = context->uiControlSystem;
    cs->RemoveSystem(cs->GetSystem<UISpineSystem>());
}
}