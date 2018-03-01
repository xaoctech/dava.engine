#include "NetworkPredictComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Snapshot.h"

#include <Logger/Logger.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPredictComponent)
{
    ReflectionRegistrator<NetworkPredictComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .Field("frameActionID", &NetworkPredictComponent::GetFrameActionIDSnap, &NetworkPredictComponent::SetFrameActionIDSnap)[M::Replicable()]
    .Field("PredictedComponentMask", &NetworkPredictComponent::predictedComponentMask)[M::Replicable()]
    .End();
}

Component* NetworkPredictComponent::Clone(Entity* toEntity)
{
    NetworkPredictComponent* uc = new NetworkPredictComponent();
    uc->SetEntity(toEntity);

    return uc;
}

void NetworkPredictComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void NetworkPredictComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

bool NetworkPredictComponent::IsPredictedComponent(Component* component) const
{
    return IsPredictedComponent(ReflectedTypeDB::GetByPointer(component)->GetType());
}
} //namespace DAVA
