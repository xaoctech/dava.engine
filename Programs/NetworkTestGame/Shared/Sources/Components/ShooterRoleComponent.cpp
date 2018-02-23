#include "Components/ShooterRoleComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

REGISTER_CLASS(ShooterRoleComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterRoleComponent)
{
    using namespace DAVA;

    ReflectionRegistrator<ShooterRoleComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Role", &ShooterRoleComponent::role)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterRoleComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterRoleComponent* component = new ShooterRoleComponent();
    component->SetRole(GetRole());
    component->SetEntity(toEntity);
    return component;
}

void ShooterRoleComponent::SetRole(ShooterRoleComponent::Role value)
{
    role = value;
}

ShooterRoleComponent::Role ShooterRoleComponent::GetRole() const
{
    return static_cast<ShooterRoleComponent::Role>(role);
}
