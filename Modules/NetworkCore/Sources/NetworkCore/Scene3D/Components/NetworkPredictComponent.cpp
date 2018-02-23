#include "NetworkPredictComponent.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "NetworkCore/Snapshot.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Logger/Logger.h>

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
    Entity* entity = component->GetEntity();
    DVASSERT(!entity || !entity->GetScene() || IsClientOwner(entity));
    return IsPredictedComponent(ReflectedTypeDB::GetByPointer(component)->GetType());
}
} //namespace DAVA
