#include "NetworkCore/Scene3D/Components/NetworkMotionComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
REGISTER_CLASS(NetworkMotionComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkMotionComponent)
{
    ReflectionRegistrator<NetworkMotionComponent>::Begin()
    .Field("paramNames", &NetworkMotionComponent::paramNames)[M::Replicable()]
    .Field("paramValues", &NetworkMotionComponent::paramValues)[M::Replicable(), M::ComparePrecision(0.001f)]
    .Field("currentMotionIds", &NetworkMotionComponent::currentMotionIds)[M::Replicable()]
    .Field("nextMotionIds", &NetworkMotionComponent::nextMotionIds)[M::Replicable()]
    .Field("transitionPhases", &NetworkMotionComponent::transitionPhases)[M::Replicable(), M::FloatQuantizeParam(0.0f, 1.0f), M::ComparePrecision(0.001f)]
    .Field("transitionInfoIndices", &NetworkMotionComponent::transitionInfoIndices)[M::Replicable()]
    .Field("motionCurrentPhaseIndices", &NetworkMotionComponent::motionCurrentPhaseIndices)[M::Replicable()]
    .Field("motionCurrentPhases", &NetworkMotionComponent::motionCurrentPhases)[M::Replicable(), M::FloatQuantizeParam(0.0f, 1.0f), M::ComparePrecision(0.001f)]
    .ConstructorByPointer()
    .End();
}

NetworkMotionComponent::NetworkMotionComponent()
    : paramNames(NumMaxParams)
    , paramValues(NumMaxParams)
    , motionCurrentPhaseIndices(NumMaxLayers * NumMaxMotions)
    , motionCurrentPhases(NumMaxLayers * NumMaxMotions)
    , currentMotionIds(NumMaxLayers)
    , nextMotionIds(NumMaxLayers)
    , transitionPhases(NumMaxLayers)
    , transitionInfoIndices(NumMaxLayers)
{
}

Component* NetworkMotionComponent::Clone(Entity* toEntity)
{
    NetworkMotionComponent* component = new NetworkMotionComponent();
    component->paramNames = paramNames;
    component->paramValues = paramValues;
    component->currentMotionIds = currentMotionIds;
    component->nextMotionIds = nextMotionIds;
    component->transitionPhases = transitionPhases;
    component->transitionInfoIndices = transitionInfoIndices;
    component->motionCurrentPhaseIndices = motionCurrentPhaseIndices;
    component->motionCurrentPhases = motionCurrentPhases;

    component->SetEntity(toEntity);
    return component;
}
}