#include "UI/Properties/UIUserPropertiesComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"
#include "Utils/Utils.h"

#include <cctype>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIUserPropertiesComponent)
{
    ReflectionRegistrator<UIUserPropertiesComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIUserPropertiesComponent* c) { SafeRelease(c); })
    .Field("properties", &UIUserPropertiesComponent::GetProperties, &UIUserPropertiesComponent::SetProperties)[M::MergeableField()]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIUserPropertiesComponent);

UIUserPropertiesComponent::UIUserPropertiesComponent() = default;

UIUserPropertiesComponent::UIUserPropertiesComponent(const UIUserPropertiesComponent& dst) = default;

UIUserPropertiesComponent::~UIUserPropertiesComponent() = default;

UIUserPropertiesComponent* UIUserPropertiesComponent::Clone() const
{
    return new UIUserPropertiesComponent(*this);
}

VarTable& UIUserPropertiesComponent::GetProperties()
{
    return properties;
}

void UIUserPropertiesComponent::SetProperties(const VarTable& value)
{
    properties = value;
    isDirty = true;
}

bool UIUserPropertiesComponent::IsDirty() const
{
    return isDirty;
}

void UIUserPropertiesComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}
}
