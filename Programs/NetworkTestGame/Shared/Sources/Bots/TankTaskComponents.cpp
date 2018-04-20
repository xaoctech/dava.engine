#include "TankTaskComponents.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(TankAttackTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(TankAttackTaskComponent)
{
    ReflectionRegistrator<TankAttackTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* TankAttackTaskComponent::Clone(Entity* toEntity)
{
    TankAttackTaskComponent* component = new TankAttackTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

TankAttackTaskComponent::TankAttackTaskComponent(uint32 targetId_, float reloadTime_, float initialDelay)
    : targetId(targetId_)
    , reloadTime(reloadTime_)
    , delay(initialDelay)
{
}

REGISTER_CLASS(TankMoveToPointTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(TankMoveToPointTaskComponent)
{
    ReflectionRegistrator<TankMoveToPointTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* TankMoveToPointTaskComponent::Clone(Entity* toEntity)
{
    TankMoveToPointTaskComponent* component = new TankMoveToPointTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

TankMoveToPointTaskComponent::TankMoveToPointTaskComponent(const Vector3& targetPoint_, float precision_)
    : targetPoint(targetPoint_)
    , precision(precision_)
{
}

REGISTER_CLASS(TankRandomMovementTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(TankRandomMovementTaskComponent)
{
    ReflectionRegistrator<TankRandomMovementTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* TankRandomMovementTaskComponent::Clone(Entity* toEntity)
{
    TankRandomMovementTaskComponent* component = new TankRandomMovementTaskComponent();
    component->SetEntity(toEntity);
    return component;
}
