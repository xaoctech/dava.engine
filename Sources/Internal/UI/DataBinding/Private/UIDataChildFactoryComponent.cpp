#include "UI/DataBinding/UIDataChildFactoryComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIDataChildFactoryComponent)
{
    ReflectionRegistrator<UIDataChildFactoryComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIDataChildFactoryComponent* o) { o->Release(); })
    .Field("package", &UIDataChildFactoryComponent::GetPackageExpression, &UIDataChildFactoryComponent::SetPackageExpression)
    .Field("control", &UIDataChildFactoryComponent::GetControlExpression, &UIDataChildFactoryComponent::SetControlExpression)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIDataChildFactoryComponent);

UIDataChildFactoryComponent::UIDataChildFactoryComponent(const UIDataChildFactoryComponent& c)
    : packageExpression(c.packageExpression)
    , controlExpression(c.controlExpression)
{
}

UIDataChildFactoryComponent* UIDataChildFactoryComponent::Clone() const
{
    return new UIDataChildFactoryComponent(*this);
}

const String& UIDataChildFactoryComponent::GetPackageExpression() const
{
    return packageExpression;
}

void UIDataChildFactoryComponent::SetPackageExpression(const String& package_)
{
    packageExpression = package_;
    isDirty = true;
}

const String& UIDataChildFactoryComponent::GetControlExpression() const
{
    return controlExpression;
}

void UIDataChildFactoryComponent::SetControlExpression(const String& control_)
{
    controlExpression = control_;
    isDirty = true;
}

bool UIDataChildFactoryComponent::IsDirty() const
{
    return isDirty;
}

void UIDataChildFactoryComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}
}
