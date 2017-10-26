#include "UI/Events/UIMovieEventComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIMovieEventComponent)
{
    ReflectionRegistrator<UIMovieEventComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIMovieEventComponent* o) { o->Release(); })
    .Field("startEvent", &UIMovieEventComponent::GetStartEvent, &UIMovieEventComponent::SetStartEvent)
    .Field("stopEvent", &UIMovieEventComponent::GetStopEvent, &UIMovieEventComponent::SetStopEvent)
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

const FastName& UIMovieEventComponent::GetStopEvent() const
{
    return stopEvent;
}

void UIMovieEventComponent::SetStopEvent(const FastName& value)
{
    stopEvent = value;
}
}
