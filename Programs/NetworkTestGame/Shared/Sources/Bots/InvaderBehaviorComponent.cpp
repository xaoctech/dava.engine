#include "InvaderBehaviorComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(InvaderBehaviorComponent)
DAVA_VIRTUAL_REFLECTION_IMPL(InvaderBehaviorComponent)
{
    ReflectionRegistrator<InvaderBehaviorComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

const Vector<String> InvaderBehaviorComponent::roleNames = {
    "OBSERVER",
    "SHOOTER",
    "TARGET"
};
const Vector<String> InvaderBehaviorComponent::scenarioNames = {
    "STILL",
    "SLIDING_TARGET",
    "SLIDING_SHOOTER",
    "SLIDING_BOTH",
    "WAGGING_TARGET",
    "WAGGING_SHOOTER",
    "WAGGING_BOTH",
    "DODGING_TARGET"
};

DAVA::Component* InvaderBehaviorComponent::Clone(DAVA::Entity* toEntity)
{
    InvaderBehaviorComponent* component = new InvaderBehaviorComponent();
    component->SetRole(GetRole());
    component->SetEntity(toEntity);
    return component;
}
