#include "BehaviorComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(BehaviorComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(BehaviorComponent)
{
    ReflectionRegistrator<BehaviorComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* BehaviorComponent::Clone(Entity* toEntity)
{
    BehaviorComponent* component = new BehaviorComponent();
    component->SetEntity(toEntity);
    return component;
}
