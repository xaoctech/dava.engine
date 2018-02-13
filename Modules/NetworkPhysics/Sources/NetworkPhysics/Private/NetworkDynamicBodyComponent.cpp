#include "NetworkPhysics/NetworkDynamicBodyComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
Component* NetworkDynamicBodyComponent::Clone(Entity* toEntity)
{
    NetworkDynamicBodyComponent* result = new NetworkDynamicBodyComponent();
    result->linearVelocity = linearVelocity;
    result->angularVelocity = angularVelocity;
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDynamicBodyComponent)
{
    ReflectionRegistrator<NetworkDynamicBodyComponent>::Begin()
    [M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .Field("linearVelocity", &NetworkDynamicBodyComponent::linearVelocity)[M::Replicable(), M::HiddenField()]
    .Field("angularVelocity", &NetworkDynamicBodyComponent::angularVelocity)[M::Replicable(), M::HiddenField()]
    .End();
}
}