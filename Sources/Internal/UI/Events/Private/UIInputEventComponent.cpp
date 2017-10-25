#include "UI/Events/UIInputEventComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIInputEventComponent)
{
    ReflectionRegistrator<UIInputEventComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIInputEventComponent* o) { o->Release(); })
    .Field("onTouchDown", &UIInputEventComponent::GetOnTouchDownEvent, &UIInputEventComponent::SetOnTouchDownEvent)
    .Field("onTouchUpInside", &UIInputEventComponent::GetOnTouchUpInsideEvent, &UIInputEventComponent::SetOnTouchUpInsideEvent)
    .Field("onTouchUpOutside", &UIInputEventComponent::GetOnTouchUpOutsideEvent, &UIInputEventComponent::SetOnTouchUpOutsideEvent)
    .Field("onHoverSet", &UIInputEventComponent::GetOnHoverSetEvent, &UIInputEventComponent::SetOnHoverSetEvent)
    .Field("onHoverRemoved", &UIInputEventComponent::GetOnHoverRemovedEvent, &UIInputEventComponent::SetOnHoverRemovedEvent)
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
