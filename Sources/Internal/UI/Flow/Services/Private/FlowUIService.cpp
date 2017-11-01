#include "UI/Flow/Services/FlowUIService.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/2D/Sprite.h"
#include "Render/Texture.h"
#include "UI/Flow/UIFlowStateComponent.h"
#include "UI/Flow/UIFlowStateSystem.h"
#include "UI/Render/UIRenderSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenshoter.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(FlowUIService)
{
    ReflectionRegistrator<FlowUIService>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](FlowUIService* s) { delete s; })
    .Method("ActivateState", &FlowUIService::ActivateState)
    .Method("DeactivateState", &FlowUIService::DeactivateState)
    .Method("PreloadState", &FlowUIService::PreloadState)
    .Method("IsStateLoaded", &FlowUIService::IsStateLoaded)
    .Method("IsStateActive", &FlowUIService::IsStateActive)
    .Method("HasTransitions", &FlowUIService::HasTransitions)
    .Method("GetCurrentSingleState", &FlowUIService::GetCurrentSingleState)
    .Method("GetCurrentMultipleStates", &FlowUIService::GetCurrentMultipleStates)
    .End();
}

void FlowUIService::ActivateState(const String& path, bool background)
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    sys->ActivateState(sys->FindStateByPath(path), background);
}

void FlowUIService::DeactivateState(const String& path, bool background)
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    sys->DeactivateState(sys->FindStateByPath(path), background);
}

void FlowUIService::PreloadState(const String& path, bool background)
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    sys->PreloadState(sys->FindStateByPath(path), background);
}

bool FlowUIService::IsStateLoaded(const String& path)
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->IsStateLoaded(sys->FindStateByPath(path));
}

bool FlowUIService::IsStateActive(const String& path)
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->IsStateActive(sys->FindStateByPath(path));
}

bool FlowUIService::HasTransitions()
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->HasTransitions();
}

UIFlowStateComponent* FlowUIService::GetCurrentSingleState()
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->GetCurrentSingleState();
}

const Vector<UIFlowStateComponent*>& FlowUIService::GetCurrentMultipleStates()
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->GetCurrentMultipleStates();
}
}
