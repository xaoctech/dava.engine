#include "SpineModule.h"

#include "UI/Spine/UISpineBonesComponent.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/UISpineSystem.h"

#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControlSystem.h>
#include <UI/Styles/UIStyleSheetSystem.h>

namespace DAVA
{
SpineModule::SpineModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISpineComponent, "Spine");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISpineBonesComponent, "SpineBones");
}

void SpineModule::Init()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    ComponentManager* cm = context->componentManager;
    cm->RegisterComponent<UISpineComponent>();
    cm->RegisterComponent<UISpineBonesComponent>();

    UIControlSystem* cs = context->uiControlSystem;
    cs->AddSystem(std::make_unique<UISpineSystem>(), cs->GetStyleSheetSystem());

    cs->AddSingleComponent(std::make_unique<UISpineSingleComponent>());
}

void SpineModule::Shutdown()
{
    const Engine* engine = Engine::Instance();
    const EngineContext* context = engine->GetContext();

    UIControlSystem* cs = context->uiControlSystem;
    cs->RemoveSystem(cs->GetSystem<UISpineSystem>());
}
}