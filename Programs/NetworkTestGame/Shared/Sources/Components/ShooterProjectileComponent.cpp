#include "ShooterProjectileComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

REGISTER_CLASS(ShooterProjectileComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterProjectileComponent)
{
    using namespace DAVA;

    // Public since we need to animate other players correctly (i.e. use angle X for look up/down animations)
    ReflectionRegistrator<ShooterProjectileComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Type", &ShooterProjectileComponent::GetProjectileType, &ShooterProjectileComponent::SetProjectileType)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterProjectileComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterProjectileComponent* component = new ShooterProjectileComponent();
    component->type = type;

    component->SetEntity(toEntity);

    return component;
}

void ShooterProjectileComponent::SetProjectileType(ShooterProjectileComponent::ProjectileType value)
{
    type = value;
}

ShooterProjectileComponent::ProjectileType ShooterProjectileComponent::GetProjectileType() const
{
    return type;
}