#include "UI/Events/UIInputEventComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIInputEventComponent)
{
    ReflectionRegistrator<UIInputEventComponent>::Begin()[M::DisplayName("Input Event"), M::Group("Input")]
    .ConstructorByPointer()
    .DestructorByPointer([](UIInputEventComponent* o) { o->Release(); })
    .Field("onTouchDown", &UIInputEventComponent::GetOnTouchDownEvent, &UIInputEventComponent::SetOnTouchDownEvent)[M::DisplayName("Touch Down")]
    .Field("onTouchUpInside", &UIInputEventComponent::GetOnTouchUpInsideEvent, &UIInputEventComponent::SetOnTouchUpInsideEvent)[M::DisplayName("Touch Up Inside")]
    .Field("onTouchUpOutside", &UIInputEventComponent::GetOnTouchUpOutsideEvent, &UIInputEventComponent::SetOnTouchUpOutsideEvent)[M::DisplayName("Touch Up Outside")]
    .Field("onValueChanged", &UIInputEventComponent::GetOnValueChangedEvent, &UIInputEventComponent::SetOnValueChangedEvent)[M::DisplayName("Value Changed")]
    .Field("onHoverSet", &UIInputEventComponent::GetOnHoverSetEvent, &UIInputEventComponent::SetOnHoverSetEvent)[M::DisplayName("Hover Start")]
    .Field("onHoverRemoved", &UIInputEventComponent::GetOnHoverRemovedEvent, &UIInputEventComponent::SetOnHoverRemovedEvent)[M::DisplayName("Hover End")]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIInputEventComponent);

UIInputEventComponent::UIInputEventComponent()
{
}

UIInputEventComponent::UIInputEventComponent(const UIInputEventComponent& src)
    : onTouchDown(src.onTouchDown)
    , onTouchUpInside(src.onTouchUpInside)
    , onTouchUpOutside(src.onTouchUpOutside)
    , onValueChanged(src.onValueChanged)
    , onHoverSet(src.onHoverSet)
    , onHoverRemoved(src.onHoverRemoved)
{
}

UIInputEventComponent::~UIInputEventComponent()
{
}

UIInputEventComponent* UIInputEventComponent::Clone() const
{
    return new UIInputEventComponent(*this);
}

const FastName& UIInputEventComponent::GetOnTouchDownEvent() const
{
    return onTouchDown;
}

void UIInputEventComponent::SetOnTouchDownEvent(const FastName& value)
{
    onTouchDown = value;
}

const FastName& UIInputEventComponent::GetOnTouchUpInsideEvent() const
{
    return onTouchUpInside;
}

void UIInputEventComponent::SetOnTouchUpInsideEvent(const FastName& value)
{
    onTouchUpInside = value;
}

const FastName& UIInputEventComponent::GetOnTouchUpOutsideEvent() const
{
    return onTouchUpOutside;
}

void UIInputEventComponent::SetOnTouchUpOutsideEvent(const FastName& value)
{
    onTouchUpOutside = value;
}

const FastName& UIInputEventComponent::GetOnValueChangedEvent() const
{
    return onValueChanged;
}

void UIInputEventComponent::SetOnValueChangedEvent(const FastName& value)
{
    onValueChanged = value;
}

const FastName& UIInputEventComponent::GetOnHoverSetEvent() const
{
    return onHoverSet;
}

void UIInputEventComponent::SetOnHoverSetEvent(const FastName& value)
{
    onHoverSet = value;
}

const FastName& UIInputEventComponent::GetOnHoverRemovedEvent() const
{
    return onHoverRemoved;
}

void UIInputEventComponent::SetOnHoverRemovedEvent(const FastName& value)
{
    onHoverRemoved = value;
}
}
