#include "UI/Flow/UIFlowViewComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIFlowViewComponent)
{
    ReflectionRegistrator<UIFlowViewComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIFlowViewComponent* c) { SafeRelease(c); })
    .Field("viewYaml", &UIFlowViewComponent::GetViewYaml, &UIFlowViewComponent::SetViewYaml)
    .Field("controlName", &UIFlowViewComponent::GetControlName, &UIFlowViewComponent::SetControlName)
    .Field("containerPath", &UIFlowViewComponent::GetContainerPath, &UIFlowViewComponent::SetContainerPath)
    .Field("modelName", &UIFlowViewComponent::GetModelName, &UIFlowViewComponent::SetModelName)
    .Field("modelScope", &UIFlowViewComponent::GetModelScope, &UIFlowViewComponent::SetModelScope)
    .End();
}

UIFlowViewComponent::UIFlowViewComponent() = default;

UIFlowViewComponent::UIFlowViewComponent(const UIFlowViewComponent& dst) = default;

UIFlowViewComponent::~UIFlowViewComponent() = default;

UIFlowViewComponent* UIFlowViewComponent::Clone() const
{
    return new UIFlowViewComponent(*this);
}

void UIFlowViewComponent::SetViewYaml(const FilePath& path)
{
    viewYamlPath = path;
}

void UIFlowViewComponent::SetControlName(const String& name)
{
    controlName = name;
}

void UIFlowViewComponent::SetContainerPath(const String& path)
{
    containerPath = path;
}

void UIFlowViewComponent::SetModelName(const String& name)
{
    modelName = name;
}

void UIFlowViewComponent::SetModelScope(const String& scope)
{
    modelScope = scope;
}
}
