#include "BattleRoyaleBehaviorComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(BattleRoyaleBehaviorComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(BattleRoyaleBehaviorComponent)
{
    ReflectionRegistrator<BattleRoyaleBehaviorComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* BattleRoyaleBehaviorComponent::Clone(Entity* toEntity)
{
    BattleRoyaleBehaviorComponent* component = new BattleRoyaleBehaviorComponent();
    component->SetEntity(toEntity);
    return component;
}
