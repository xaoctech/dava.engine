#include "InvaderTaskComponents.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(InvaderSlideToBorderTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(InvaderSlideToBorderTaskComponent)
{
    ReflectionRegistrator<InvaderSlideToBorderTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* InvaderSlideToBorderTaskComponent::Clone(Entity* toEntity)
{
    InvaderSlideToBorderTaskComponent* component = new InvaderSlideToBorderTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

InvaderSlideToBorderTaskComponent::InvaderSlideToBorderTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}

REGISTER_CLASS(InvaderWagToBorderTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(InvaderWagToBorderTaskComponent)
{
    ReflectionRegistrator<InvaderWagToBorderTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* InvaderWagToBorderTaskComponent::Clone(Entity* toEntity)
{
    InvaderWagToBorderTaskComponent* component = new InvaderWagToBorderTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

InvaderWagToBorderTaskComponent::InvaderWagToBorderTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}

REGISTER_CLASS(InvaderDodgeCenterTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(InvaderDodgeCenterTaskComponent)
{
    ReflectionRegistrator<InvaderDodgeCenterTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* InvaderDodgeCenterTaskComponent::Clone(Entity* toEntity)
{
    InvaderDodgeCenterTaskComponent* component = new InvaderDodgeCenterTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

InvaderDodgeCenterTaskComponent::InvaderDodgeCenterTaskComponent(bool movingRight_)
    : movingRight(movingRight_)
{
}

REGISTER_CLASS(InvaderShootIfSeeingTargetTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(InvaderShootIfSeeingTargetTaskComponent)
{
    ReflectionRegistrator<InvaderShootIfSeeingTargetTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* InvaderShootIfSeeingTargetTaskComponent::Clone(Entity* toEntity)
{
    InvaderShootIfSeeingTargetTaskComponent* component = new InvaderShootIfSeeingTargetTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

InvaderShootIfSeeingTargetTaskComponent::InvaderShootIfSeeingTargetTaskComponent(uint32 targetId_)
    : targetId(targetId_)
{
}