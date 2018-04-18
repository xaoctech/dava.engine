#include "NetworkPhysics/HitboxesDebugDrawComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
REGISTER_CLASS(HitboxesDebugDrawComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(HitboxesDebugDrawComponent)
{
    ReflectionRegistrator<HitboxesDebugDrawComponent>::Begin()
    .Field("serverHitboxPositions", &HitboxesDebugDrawComponent::serverHitboxPositions)[M::Replicable()]
    .Field("serverHitboxOrientations", &HitboxesDebugDrawComponent::serverHitboxOrientations)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

HitboxesDebugDrawComponent::HitboxesDebugDrawComponent()
    : serverHitboxPositions(NumMaxHitboxes)
    , serverHitboxOrientations(NumMaxHitboxes)
    , clientHitboxPositions(NumMaxHitboxes)
    , clientHitboxOrientations(NumMaxHitboxes)
{
}

Component* HitboxesDebugDrawComponent::Clone(Entity* toEntity)
{
    HitboxesDebugDrawComponent* component = new HitboxesDebugDrawComponent();
    component->serverHitboxPositions = serverHitboxPositions;
    component->serverHitboxOrientations = serverHitboxOrientations;
    component->clientHitboxPositions = clientHitboxPositions;
    component->clientHitboxOrientations = clientHitboxOrientations;

    component->SetEntity(toEntity);
    return component;
}
}