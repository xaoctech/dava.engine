#include "UI/Flow/Services/FlowUIService.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/2D/Sprite.h"
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
    .Method("activateState", &FlowUIService::ActivateState)
    .Method("deactivateState", &FlowUIService::DeactivateState)
    .Method("preloadState", &FlowUIService::PreloadState)
    .Method("isStateLoaded", &FlowUIService::IsStateLoaded)
    .Method("isStateActive", &FlowUIService::IsStateActive)
    .Method("hasTransitions", &FlowUIService::HasTransitions)
    .Method("getCurrentState", &FlowUIService::GetCurrentState)
    .Method("getScreenshot", &FlowUIService::GetScreenshot)
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

UIFlowStateComponent* FlowUIService::GetCurrentState()
{
    UIFlowStateSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIFlowStateSystem>();
    return sys->GetCurrentState();
}

Sprite* FlowUIService::GetScreenshot()
{
    UIControl* screen = GetEngineContext()->uiControlSystem->GetScreen();
    UIScreenshoter* shoter = GetEngineContext()->uiControlSystem->GetRenderSystem()->GetScreenshoter();
    RefPtr<Texture> texture = shoter->MakeScreenshot(screen, FORMAT_RGBA8888, true, true);
    Sprite* sprite = Sprite::CreateFromTexture(texture.Get(), 0, 0, screen->GetSize().dx, screen->GetSize().dy, false);
    return sprite;
}
}
