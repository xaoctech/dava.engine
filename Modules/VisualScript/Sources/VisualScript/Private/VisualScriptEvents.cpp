#include "VisualScript/VisualScriptEvents.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptEvent)
{
    ReflectionRegistrator<VisualScriptEvent>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ButtonClickEvent)
{
    ReflectionRegistrator<ButtonClickEvent>::Begin()
    .ConstructorByPointer()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(TimerEvent)
{
    ReflectionRegistrator<TimerEvent>::Begin()
    .ConstructorByPointer()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntitiesCollideEvent)
{
    ReflectionRegistrator<EntitiesCollideEvent>::Begin()
    .ConstructorByPointer()
    .Field("collisionInfo", &EntitiesCollideEvent::collisionInfo)
    .End();
}

} //DAVA
