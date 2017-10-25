#include "UI/Events/UIMovieEventComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIMovieEventComponent)
{
    ReflectionRegistrator<UIMovieEventComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIMovieEventComponent* o) { o->Release(); })
    .Field("startEvent", &UIMovieEventComponent::GetStartEventAsString, &UIMovieEventComponent::SetStartEventFromString)
    .Field("stopEvent", &UIMovieEventComponent::GetStopEventAsString, &UIMovieEventComponent::SetStopEventFromString)
    .End();
}
IMPLEMENT_UI_COMPONENT(UIMovieEventComponent);

UIMovieEventComponent::UIMovieEventComponent()
{
}

UIMovieEventComponent::UIMovieEventComponent(const UIMovieEventComponent& src)
    : startEvent(src.startEvent)
    ,
    stopEvent(src.stopEvent)
{
}

UIMovieEventComponent::~UIMovieEventComponent()
{
}

UIMovieEventComponent* UIMovieEventComponent::Clone() const
{
    return new UIMovieEventComponent(*this);
}

const FastName& UIMovieEventComponent::GetStartEvent() const
{
    return startEvent;
}

void UIMovieEventComponent::SetStartEvent(const FastName& value)
{
    startEvent = value;
}

String UIMovieEventComponent::GetStartEventAsString() const
{
    return startEvent.IsValid() ? startEvent.c_str() : String("");
}

void UIMovieEventComponent::SetStartEventFromString(const String& value)
{
    SetStartEvent(value.empty() ? FastName() : FastName(value));
}

const FastName& UIMovieEventComponent::GetStopEvent() const
{
    return stopEvent;
}

void UIMovieEventComponent::SetStopEvent(const FastName& value)
{
    stopEvent = value;
}

String UIMovieEventComponent::GetStopEventAsString() const
{
    return stopEvent.IsValid() ? stopEvent.c_str() : String("");
}

void UIMovieEventComponent::SetStopEventFromString(const String& value)
{
    SetStopEvent(value.empty() ? FastName() : FastName(value));
}
}
