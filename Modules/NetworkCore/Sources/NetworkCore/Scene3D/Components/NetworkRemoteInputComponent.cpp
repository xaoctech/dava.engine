#include "NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h"

#include <FileSystem/KeyedArchive.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
REGISTER_CLASS(NetworkRemoteInputComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkRemoteInputComponent)
{
    ReflectionRegistrator<NetworkRemoteInputComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .Field("triggeredActions", &NetworkRemoteInputComponent::triggeredActions)[M::Replicable()]
    .Field("triggeredAnalogActionsValues", &NetworkRemoteInputComponent::triggeredAnalogActionsValues)[M::Replicable()]
    .Field("actionFrames", &NetworkRemoteInputComponent::actionFrames)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

NetworkRemoteInputComponent::NetworkRemoteInputComponent()
    : triggeredActions()
    , triggeredAnalogActionsValues()
    , actionFrames()
{
}

Component* NetworkRemoteInputComponent::Clone(Entity* toEntity)
{
    NetworkRemoteInputComponent* clone = new NetworkRemoteInputComponent();
    clone->triggeredActions = triggeredActions;
    clone->triggeredAnalogActionsValues = triggeredAnalogActionsValues;
    clone->actionFrames = actionFrames;

    clone->SetEntity(toEntity);

    return clone;
}

void NetworkRemoteInputComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    archive->SetByteArray("networkRemoteInputComponent.triggeredActions", reinterpret_cast<uint8*>(triggeredActions.data()), ACTIONS_BUFFER_SIZE * sizeof(uint64));
    archive->SetByteArray("networkRemoteInputComponent.triggeredAnalogActionsValues", reinterpret_cast<uint8*>(triggeredAnalogActionsValues.data()), ACTIONS_BUFFER_SIZE * sizeof(uint64));
    archive->SetByteArray("networkRemoteInputComponent.actionFrames", reinterpret_cast<uint8*>(actionFrames.data()), ACTIONS_BUFFER_SIZE * sizeof(uint32));
}

void NetworkRemoteInputComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    memcpy(triggeredActions.data(), archive->GetByteArray("networkRemoteInputComponent.triggeredActions"), ACTIONS_BUFFER_SIZE * sizeof(uint64));
    memcpy(triggeredAnalogActionsValues.data(), archive->GetByteArray("networkRemoteInputComponent.triggeredAnalogActionsValues"), ACTIONS_BUFFER_SIZE * sizeof(uint64));
    memcpy(actionFrames.data(), archive->GetByteArray("networkRemoteInputComponent.actionFrames"), ACTIONS_BUFFER_SIZE * sizeof(uint32));
}

void NetworkRemoteInputComponent::AddActionToReplicate(const FastName& action)
{
    actionsToReplicate.insert(action);
}
}
