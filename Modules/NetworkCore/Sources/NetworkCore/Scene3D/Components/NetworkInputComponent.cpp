#include "NetworkInputComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
REGISTER_CLASS(NetworkInputComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkInputComponent)
{
    ReflectionRegistrator<NetworkInputComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .End();
}

NetworkInputComponent::NetworkInputComponent()
    : history(1, MAX_HISTORY_SIZE)
{
}

Component* NetworkInputComponent::Clone(Entity* toEntity)
{
    NetworkInputComponent* uc = new NetworkInputComponent();
    uc->SetEntity(toEntity);

    return uc;
}

void NetworkInputComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void NetworkInputComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

void NetworkInputComponent::SaveHistory(uint32 frameId, const NetworkInputComponent::Data& data)
{
    history.Push(frameId, data);
}

bool NetworkInputComponent::Data::operator==(const NetworkInputComponent::Data& rhs) const
{
    return actions == rhs.actions &&
    analogStates == rhs.analogStates &&
    cameraDelta == rhs.cameraDelta;
}

bool NetworkInputComponent::Data::operator!=(const NetworkInputComponent::Data& rhs) const
{
    return !(*this == rhs);
}

uint32 NetworkInputComponent::GetFrameFail() const
{
    return frameFail;
}

void NetworkInputComponent::SetFrameFail(uint32 frameId)
{
    frameFail = frameId;
}

const NetworkInputComponent::History& NetworkInputComponent::GetHistory() const
{
    return history;
}

NetworkInputComponent::History& NetworkInputComponent::ModifyHistory()
{
    return history;
}

bool NetworkInputComponent::HasLastCameraOrientation() const
{
    return hasLastCameraOrientation;
}

void NetworkInputComponent::SetLastCameraOrientation(const Quaternion& camOrient)
{
    lastCamOrient = camOrient;
    hasLastCameraOrientation = true;
}

const Quaternion& NetworkInputComponent::GetLastCameraOrientation() const
{
    return lastCamOrient;
}

NetworkInputComponent::ResimulationCache& NetworkInputComponent::ModifyResimulationCache()
{
    return resimulationCache;
}
}
