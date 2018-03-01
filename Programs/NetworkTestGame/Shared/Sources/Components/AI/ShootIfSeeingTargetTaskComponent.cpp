#include "ShootIfSeeingTargetTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ShootIfSeeingTargetTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShootIfSeeingTargetTaskComponent)
{
    ReflectionRegistrator<ShootIfSeeingTargetTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShootIfSeeingTargetTaskComponent::Clone(Entity* toEntity)
{
    ShootIfSeeingTargetTaskComponent* component = new ShootIfSeeingTargetTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShootIfSeeingTargetTaskComponent::ShootIfSeeingTargetTaskComponent(uint32 targetID_)
    : targetID(targetID_)
{
}
