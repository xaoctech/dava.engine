#include "WaitTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(WaitTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(WaitTaskComponent)
{
    ReflectionRegistrator<WaitTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* WaitTaskComponent::Clone(Entity* toEntity)
{
    WaitTaskComponent* component = new WaitTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

WaitTaskComponent::WaitTaskComponent(Type type_, float time_)
    :
    time(time_)
    , type(type_)
{
}
