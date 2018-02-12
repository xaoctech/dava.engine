#include "ShooterBehaviorComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ShooterBehaviorComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterBehaviorComponent)
{
    ReflectionRegistrator<ShooterBehaviorComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterBehaviorComponent::Clone(Entity* toEntity)
{
    ShooterBehaviorComponent* component = new ShooterBehaviorComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterBehaviorComponent::ShooterBehaviorComponent(bool isActor_)
    :
    isActor(isActor_)
{
}
