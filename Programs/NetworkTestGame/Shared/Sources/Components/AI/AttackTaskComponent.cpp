#include "AttackTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(AttackTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(AttackTaskComponent)
{
    ReflectionRegistrator<AttackTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* AttackTaskComponent::Clone(Entity* toEntity)
{
    AttackTaskComponent* component = new AttackTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

AttackTaskComponent::AttackTaskComponent(uint32 targetID_, float reloadTime_, float initialDelay)
    :
    targetID(targetID_)
    , reloadTime(reloadTime_)
    , delay(initialDelay)
{
}
