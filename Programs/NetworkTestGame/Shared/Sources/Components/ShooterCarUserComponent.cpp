#include "Components/ShooterCarUserComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

REGISTER_CLASS(ShooterCarUserComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCarUserComponent)
{
    using namespace DAVA;

    ReflectionRegistrator<ShooterCarUserComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("carNetworkId", &ShooterCarUserComponent::carNetworkId)[M::Replicable(), M::HiddenField()]
    .Field("passengerIndex", &ShooterCarUserComponent::passengerIndex)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterCarUserComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterCarUserComponent* component = new ShooterCarUserComponent();
    component->carNetworkId = carNetworkId;
    component->passengerIndex = passengerIndex;

    component->SetEntity(toEntity);

    return component;
}

DAVA::NetworkID ShooterCarUserComponent::GetCarNetworkId() const
{
    return carNetworkId;
}

DAVA::uint32 ShooterCarUserComponent::GetPassengerIndex() const
{
    return passengerIndex;
}
