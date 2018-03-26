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

size_t NetworkMovementComponent::HistoryGetSize()
{
    return interpolationHistory.size();
}

NetworkMovementComponent::MoveState& NetworkMovementComponent::HistoryAt(size_t index)
{
    DVASSERT(index < interpolationHistory.size());

    index += interpolationHistoryPushBackPos;

    if (index >= interpolationHistory.size())
    {
        index -= interpolationHistory.size();
    }

    return interpolationHistory[index];
}

NetworkMovementComponent::MoveState& NetworkMovementComponent::HistoryBack()
{
    size_t index = interpolationHistoryPushBackPos;

    if (index > 0)
    {
        index--;
    }
    else
    {
        index = interpolationHistory.size() - 1;
    }

    return interpolationHistory[index];
}

void NetworkMovementComponent::HistoryResize(size_t size)
{
    interpolationHistory.resize(size);
    interpolationHistoryPushBackPos = 0;

    // clear previous history
    for (auto& v : interpolationHistory)
    {
        v = MoveState();
    }
}

void NetworkMovementComponent::HistoryPushBack(MoveState&& state)
{
    interpolationHistory[interpolationHistoryPushBackPos] = std::move(state);

    interpolationHistoryPushBackPos++;
    if (interpolationHistoryPushBackPos == interpolationHistory.size())
    {
        interpolationHistoryPushBackPos = 0;
    }
}

};
