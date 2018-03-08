#include "NetworkPredictComponent.h"

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPredictComponent)
{
    NetworkPredictComponent* (*ctorFn)() = []()
    {
        return new NetworkPredictComponent(ComponentMask());
    };

    ReflectionRegistrator<NetworkPredictComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer(ctorFn)
    .Field("predictionMask", &NetworkPredictComponent::predictionMask)[M::Replicable()]
    .End();
}

NetworkPredictComponent::NetworkPredictComponent(ComponentMask mask)
    : predictionMask(std::move(mask))
{
}

Component* NetworkPredictComponent::Clone(Entity* toEntity)
{
    NetworkPredictComponent* uc = new NetworkPredictComponent(predictionMask);
    uc->SetEntity(toEntity);

    return uc;
}

void NetworkPredictComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    DVASSERT(false);
    // TODO:
    // ...
}

void NetworkPredictComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    DVASSERT(false);
    // TODO:
    // ...
}

void NetworkPredictComponent::SetForPrediction(const Type* componentType)
{
    DVASSERT(entity == nullptr, "You can't setup prediction when component is already belong to entity");
    predictionMask.Set(componentType);
}

const ComponentMask& NetworkPredictComponent::GetPredictionMask() const
{
    return predictionMask;
}

} //namespace DAVA
