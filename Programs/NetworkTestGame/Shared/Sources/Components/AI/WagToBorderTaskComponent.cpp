#include "WagToBorderTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(WagToBorderTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(WagToBorderTaskComponent)
{
    ReflectionRegistrator<WagToBorderTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* WagToBorderTaskComponent::Clone(Entity* toEntity)
{
    WagToBorderTaskComponent* component = new WagToBorderTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

WagToBorderTaskComponent::WagToBorderTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}
