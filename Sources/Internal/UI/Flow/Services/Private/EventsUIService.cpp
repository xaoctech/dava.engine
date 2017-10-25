#include "UI/Flow/Services/EventsUIService.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Reflection/ReflectionRegistrator.h"
// TODO: uncomment after merging Events
//#include "UI/Events/UIEventsSingleComponent.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(EventsUIService)
{
    ReflectionRegistrator<EventsUIService>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](EventsUIService* s) { delete s; })
    .Method("send", &EventsUIService::Send)
    .End();
}

void EventsUIService::Send(UIControl* control, const FastName& event)
{
    // TODO: Uncomment after merging Events
    //    UIEventsSingleComponent* events = Engine::Instance()->GetContext()->uiControlSystem->GetSingleComponent<UIEventsSingleComponent>();
    //    if (events)
    //    {
    //        events->DispatchEvent(control, event);
    //    }
}
}
