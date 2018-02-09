#include "RocketSpawnComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(RocketSpawnComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(RocketSpawnComponent)
{
    ReflectionRegistrator<RocketSpawnComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Progress", &RocketSpawnComponent::progress)[M::Replicable()]
    .End();
}

Component* RocketSpawnComponent::Clone(Entity* toEntity)
{
    RocketSpawnComponent* component = new RocketSpawnComponent();
    component->SetEntity(toEntity);
    return component;
}
