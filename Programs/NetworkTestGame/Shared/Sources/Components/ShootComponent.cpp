#include "ShootComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

using namespace DAVA;

const uint32 ShootComponent::MAX_DISTANCE = 100;
const float32 ShootComponent::MOVE_SPEED = 100.f;

DAVA_VIRTUAL_REFLECTION_IMPL(ShootComponent)
{
    ReflectionRegistrator<ShootComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("distance", &ShootComponent::distance)[M::Replicable()]
    .End();
}

ShootComponent::ShootComponent()
{
}

ShootComponent::~ShootComponent()
{
}

Component* ShootComponent::Clone(Entity* toEntity)
{
    ShootComponent* component = new ShootComponent();
    component->SetEntity(toEntity);
    return component;
}

void ShootComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void ShootComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

ShootPhase ShootComponent::GetPhase() const
{
    return phase;
}
void ShootComponent::SetPhase(ShootPhase phase_)
{
    phase = phase_;
}

uint32 ShootComponent::GetDistance() const
{
    return distance;
}

void ShootComponent::SetDistance(uint32 distance_)
{
    distance = distance_;
}

DAVA::uint32 ShootComponent::GetShootType() const
{
    return shootTypeMask;
}
void ShootComponent::SetShootType(DAVA::uint32 shootTypeMask_)
{
    shootTypeMask = shootTypeMask_;
}

DAVA::Entity* ShootComponent::GetShooter() const
{
    return shooter;
}

void ShootComponent::SetShooter(DAVA::Entity* shooter_)
{
    shooter = shooter_;
}
