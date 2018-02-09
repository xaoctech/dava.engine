#include "RandomMovementTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(RandomMovementTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(RandomMovementTaskComponent)
{
    ReflectionRegistrator<RandomMovementTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* RandomMovementTaskComponent::Clone(Entity* toEntity)
{
    RandomMovementTaskComponent* component = new RandomMovementTaskComponent();
    component->SetEntity(toEntity);
    return component;
}
