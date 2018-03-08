#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPredictionSingleComponent)
{
    ReflectionRegistrator<NetworkPredictionSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

NetworkPredictionSingleComponent::NetworkPredictionSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::FixedProcesses)
{
}

void NetworkPredictionSingleComponent::Clear()
{
    mispredictedEntities.clear();
}
}
