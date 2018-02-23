#include "UI/VisualScript/UIVisualScriptComponent.h"

#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIVisualScriptComponent)
{
    ReflectionRegistrator<UIVisualScriptComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIVisualScriptComponent* c) { SafeRelease(c); })
    .Field("enabled", &UIVisualScriptComponent::IsEnabled, &UIVisualScriptComponent::SetEnabled)
    .Field("scriptPath", &UIVisualScriptComponent::GetScriptPath, &UIVisualScriptComponent::SetScriptPath)
    .Field("properties", &UIVisualScriptComponent::GetProperties, &UIVisualScriptComponent::SetProperties)[M::MergeableField()]
    .End();
}
IMPLEMENT_UI_COMPONENT(UIVisualScriptComponent);

UIVisualScriptComponent::UIVisualScriptComponent() = default;

UIVisualScriptComponent::UIVisualScriptComponent(const UIVisualScriptComponent& dst) = default;

UIVisualScriptComponent::~UIVisualScriptComponent() = default;

UIVisualScriptComponent* UIVisualScriptComponent::Clone() const
{
    return new UIVisualScriptComponent(*this);
}

void UIVisualScriptComponent::SetScriptPath(const FilePath& filePath)
{
    if (scriptPath != filePath)
    {
        scriptPath = filePath;
        needReload = true;
    }
}

void UIVisualScriptComponent::SetEnabled(bool v)
{
    enabled = v;
}

void UIVisualScriptComponent::SetNeedReload(bool v)
{
    needReload = v;
}

VarTable& UIVisualScriptComponent::GetProperties()
{
    return properties;
}

void UIVisualScriptComponent::SetProperties(const VarTable& value)
{
    properties = value;
    needReload = true;
}
}
