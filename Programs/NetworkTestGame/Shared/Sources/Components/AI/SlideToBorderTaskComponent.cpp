#include "SlideToBorderTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(SlideToBorderTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(SlideToBorderTaskComponent)
{
    ReflectionRegistrator<SlideToBorderTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* SlideToBorderTaskComponent::Clone(Entity* toEntity)
{
    SlideToBorderTaskComponent* component = new SlideToBorderTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

SlideToBorderTaskComponent::SlideToBorderTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}
