#include "VisualScript/VisualScriptEvents.h"
#include "Scene3D/Components/VisualScriptComponent.h"

#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SingleComponents/CollisionSingleComponent.h>

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

DAVA_VIRTUAL_REFLECTION_IMPL(ProcessEvent)
{
    ReflectionRegistrator<ProcessEvent>::Begin()
    .ConstructorByPointer()
    .Field("component", &ProcessEvent::component)
    .Field("frameDelta", &ProcessEvent::frameDelta)
    .End();
}

} //DAVA
