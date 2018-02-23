#include "UI/VisualScript/Private/UIVisualScriptEvents.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIInitEvent)
{
    ReflectionRegistrator<UIInitEvent>::Begin()
    .ConstructorByPointer()
    .Field("component", &UIInitEvent::component)
    .End();
}
const FastName UIInitEvent::NAME = FastName("UIInitEvent");

DAVA_VIRTUAL_REFLECTION_IMPL(UIReleaseEvent)
{
    ReflectionRegistrator<UIReleaseEvent>::Begin()
    .ConstructorByPointer()
    .Field("component", &UIReleaseEvent::component)
    .End();
}
const FastName UIReleaseEvent::NAME = FastName("UIReleaseEvent");

DAVA_VIRTUAL_REFLECTION_IMPL(UIProcessEvent)
{
    ReflectionRegistrator<UIProcessEvent>::Begin()
    .ConstructorByPointer()
    .Field("component", &UIProcessEvent::component)
    .Field("frameDelta", &UIProcessEvent::frameDelta)
    .End();
}
const FastName UIProcessEvent::NAME = FastName("UIProcessEvent");

DAVA_VIRTUAL_REFLECTION_IMPL(UIEventProcessEvent)
{
    ReflectionRegistrator<UIEventProcessEvent>::Begin()
    .ConstructorByPointer()
    .Field("component", &UIEventProcessEvent::component)
    .Field("eventName", &UIEventProcessEvent::eventName)
    .End();
}
const FastName UIEventProcessEvent::NAME = FastName("UIEventProcessEvent");

} //DAVA
