#include "NetworkTrafficLimitComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(NetworkTrafficLimitComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTrafficLimitComponent)
{
    ReflectionRegistrator<NetworkTrafficLimitComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

NetworkTrafficLimitComponent::NetworkTrafficLimitComponent()
{
}

Component* NetworkTrafficLimitComponent::Clone(Entity* toEntity)
{
    NetworkTrafficLimitComponent* component = new NetworkTrafficLimitComponent();
    component->SetEntity(toEntity);
    component->errorThreshold = errorThreshold;
    component->warningThreshold = warningThreshold;
    return component;
}

NetworkTrafficLimitComponent::~NetworkTrafficLimitComponent()
{
}
