#include "UI/Events/UIEventsSingleComponent.h"

#include <UI/UIControl.h>

namespace DAVA
{
DefferedEvent::DefferedEvent(UIControl* control_, const FastName& event_, bool broadcast_)
    : event(event_)
    , broadcast(broadcast_)
{
    control = control_;
}

void UIEventsSingleComponent::ResetState()
{
}

bool UIEventsSingleComponent::SendEvent(UIControl* control, const FastName& event)
{
    if (event.IsValid())
    {
        events.emplace_back(control, event, false);
        return true;
    }
    return false;
}

bool UIEventsSingleComponent::SendBroadcastEvent(UIControl* control, const FastName& event)
{
    if (event.IsValid())
    {
        events.emplace_back(control, event, true);
        return true;
    }
    return false;
}
}
