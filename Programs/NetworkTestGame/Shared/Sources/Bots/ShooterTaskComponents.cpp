#include "ShooterTaskComponents.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ShooterAttackStandingStillTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAttackStandingStillTaskComponent)
{
    ReflectionRegistrator<ShooterAttackStandingStillTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterAttackStandingStillTaskComponent::Clone(Entity* toEntity)
{
    ShooterAttackStandingStillTaskComponent* component = new ShooterAttackStandingStillTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterAttackStandingStillTaskComponent::ShooterAttackStandingStillTaskComponent(uint32 targetId_)
    : targetId(targetId_)
{
}

REGISTER_CLASS(ShooterAttackPursuingTargetTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAttackPursuingTargetTaskComponent)
{
    ReflectionRegistrator<ShooterAttackPursuingTargetTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterAttackPursuingTargetTaskComponent::Clone(Entity* toEntity)
{
    ShooterAttackPursuingTargetTaskComponent* component = new ShooterAttackPursuingTargetTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterAttackPursuingTargetTaskComponent::ShooterAttackPursuingTargetTaskComponent(uint32 targetId_)
    : targetId(targetId_)
{
}

REGISTER_CLASS(ShooterAttackCirclingAroundTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAttackCirclingAroundTaskComponent)
{
    ReflectionRegistrator<ShooterAttackCirclingAroundTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterAttackCirclingAroundTaskComponent::Clone(Entity* toEntity)
{
    ShooterAttackCirclingAroundTaskComponent* component = new ShooterAttackCirclingAroundTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterAttackCirclingAroundTaskComponent::ShooterAttackCirclingAroundTaskComponent(uint32 targetId_, bool right_)
    : targetId(targetId_)
    , right(right_)
{
}

REGISTER_CLASS(ShooterAttackWaggingTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAttackWaggingTaskComponent)
{
    ReflectionRegistrator<ShooterAttackWaggingTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterAttackWaggingTaskComponent::Clone(Entity* toEntity)
{
    ShooterAttackWaggingTaskComponent* component = new ShooterAttackWaggingTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterAttackWaggingTaskComponent::ShooterAttackWaggingTaskComponent(uint32 targetId_)
    : targetId(targetId_)
{
}

REGISTER_CLASS(ShooterMoveToPointShortestTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMoveToPointShortestTaskComponent)
{
    ReflectionRegistrator<ShooterMoveToPointShortestTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterMoveToPointShortestTaskComponent::Clone(Entity* toEntity)
{
    ShooterMoveToPointShortestTaskComponent* component = new ShooterMoveToPointShortestTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterMoveToPointShortestTaskComponent::ShooterMoveToPointShortestTaskComponent(const Vector3& point_)
    : point(point_)
{
}

REGISTER_CLASS(ShooterMoveToPointWindingTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMoveToPointWindingTaskComponent)
{
    ReflectionRegistrator<ShooterMoveToPointWindingTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterMoveToPointWindingTaskComponent::Clone(Entity* toEntity)
{
    ShooterMoveToPointWindingTaskComponent* component = new ShooterMoveToPointWindingTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterMoveToPointWindingTaskComponent::ShooterMoveToPointWindingTaskComponent(const Vector3& point_)
    : point(point_)
{
}

REGISTER_CLASS(ShooterHangAroundTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterHangAroundTaskComponent)
{
    ReflectionRegistrator<ShooterHangAroundTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterHangAroundTaskComponent::Clone(Entity* toEntity)
{
    ShooterHangAroundTaskComponent* component = new ShooterHangAroundTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterHangAroundTaskComponent::ShooterHangAroundTaskComponent(Vector3& point_, float32 angle_)
    : point(point_)
    , angle(angle_)
{
}

REGISTER_CLASS(ShooterDriveTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ShooterDriveTaskComponent)
{
    ReflectionRegistrator<ShooterDriveTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* ShooterDriveTaskComponent::Clone(Entity* toEntity)
{
    ShooterDriveTaskComponent* component = new ShooterDriveTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

ShooterDriveTaskComponent::ShooterDriveTaskComponent(const Vector3& point_, uint32 carId_)
    : point(point_)
    , carId(carId_)
{
}
