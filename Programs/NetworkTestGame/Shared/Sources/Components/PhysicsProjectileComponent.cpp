#include "PhysicsProjectileComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsProjectileComponent)
{
    ReflectionRegistrator<PhysicsProjectileComponent>::Begin()
    [M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .Field("Type", &PhysicsProjectileComponent::GetProjectileType, &PhysicsProjectileComponent::SetProjectileType)[M::Replicable()]
    .Field("State", &PhysicsProjectileComponent::GetProjectileState, &PhysicsProjectileComponent::SetProjectileState)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

Component* PhysicsProjectileComponent::Clone(Entity* toEntity)
{
    PhysicsProjectileComponent* component = new PhysicsProjectileComponent();
    component->SetEntity(toEntity);

    return component;
}

void PhysicsProjectileComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetUInt32("type", static_cast<uint32>(type));
    archive->SetUInt32("state", static_cast<uint32>(state));
}

void PhysicsProjectileComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    type = static_cast<eProjectileTypes>(archive->GetUInt32("type", static_cast<uint32>(type)));
    state = static_cast<eProjectileStates>(archive->GetUInt32("state", static_cast<uint32>(state)));
}

PhysicsProjectileComponent::eProjectileTypes PhysicsProjectileComponent::GetProjectileType() const
{
    return type;
}

void PhysicsProjectileComponent::SetProjectileType(eProjectileTypes value)
{
    type = value;
}

PhysicsProjectileComponent::eProjectileStates PhysicsProjectileComponent::GetProjectileState() const
{
    return state;
}

void PhysicsProjectileComponent::SetProjectileState(eProjectileStates value)
{
    state = value;
}

void PhysicsProjectileComponent::SetInitialPosition(const DAVA::Vector3& value)
{
    initialPos = value;
}

DAVA::Vector3 PhysicsProjectileComponent::GetInitialPosition() const
{
    return initialPos;
}
