#include "NetworkCore/Scene3D/Components/NetworkMovementComponent.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/SceneFile/SerializationContext.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkMovementComponent)
{
    ReflectionRegistrator<NetworkMovementComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::CantBeDeletedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

Component* NetworkMovementComponent::Clone(Entity* toEntity)
{
    NetworkMovementComponent* newMovement = new NetworkMovementComponent();
    newMovement->SetEntity(toEntity);
    return newMovement;
}

void NetworkMovementComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void NetworkMovementComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void NetworkMovementComponent::MoveSmothly(const Vector3& position, const Quaternion& rotation)
{
}

size_t NetworkMovementComponent::HistoryGetSize()
{
    return history.size();
}

NetworkMovementComponent::MoveState& NetworkMovementComponent::HistoryAt(size_t index)
{
    DVASSERT(index < history.size());

    index += pushBackPos;

    if (index >= history.size())
    {
        index -= history.size();
    }

    return history[index];
}

NetworkMovementComponent::MoveState& NetworkMovementComponent::HistoryBack()
{
    size_t index = pushBackPos;

    if (index > 0)
    {
        index--;
    }
    else
    {
        index = history.size() - 1;
    }

    return history[index];
}

void NetworkMovementComponent::HistoryResize(size_t size)
{
    // TODO:
    // ...

    history.resize(size);
}

void NetworkMovementComponent::HistoryPushBack(MoveState&& state)
{
    history[pushBackPos] = std::move(state);

    pushBackPos++;
    if (pushBackPos == history.size())
    {
        pushBackPos = 0;
    }
}

void NetworkMovementComponent::CorrectionApply(MoveState&& state)
{
    correctionTimeLeft = correctionTimeoutSec;
    correction.translation += state.translation;
    correction.rotation *= state.rotation;

    if (correction.frameId == 0)
    {
        correction.frameId = state.frameId;
    }
}
};
