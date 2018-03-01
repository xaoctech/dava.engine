#include "UI/DataBinding/UIDataScopeComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
using std::shared_ptr;

DAVA_VIRTUAL_REFLECTION_IMPL(UIDataScopeComponent)
{
    ReflectionRegistrator<UIDataScopeComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIDataScopeComponent* o) { o->Release(); })
    .Field("expression", &UIDataScopeComponent::GetExpression, &UIDataScopeComponent::SetExpression)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIDataScopeComponent);

UIDataScopeComponent::UIDataScopeComponent(const UIDataScopeComponent& c)
    : expression(c.expression)
{
    isDirty = true;
}

UIDataScopeComponent* UIDataScopeComponent::Clone() const
{
    return new UIDataScopeComponent(*this);
}

const String& UIDataScopeComponent::GetExpression() const
{
    return expression;
}

void UIDataScopeComponent::SetExpression(const String& expression_)
{
    expression = expression_;
    isDirty = true;
}

bool UIDataScopeComponent::IsDirty() const
{
    return isDirty;
}

void UIDataScopeComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}
}
