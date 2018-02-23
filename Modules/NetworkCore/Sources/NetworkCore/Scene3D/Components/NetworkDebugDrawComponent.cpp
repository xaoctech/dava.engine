#include "NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h"

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
REGISTER_CLASS(NetworkDebugDrawComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDebugDrawComponent)
{
    ReflectionRegistrator<NetworkDebugDrawComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

NetworkDebugDrawComponent::NetworkDebugDrawComponent()
{
}

Component* NetworkDebugDrawComponent::Clone(Entity* toEntity)
{
    NetworkDebugDrawComponent* uc = new NetworkDebugDrawComponent();
    uc->SetEntity(toEntity);
    return uc;
}
NetworkDebugDrawComponent::~NetworkDebugDrawComponent()
{
}
}
