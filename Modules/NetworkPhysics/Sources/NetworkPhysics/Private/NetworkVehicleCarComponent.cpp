#include "NetworkPhysics/NetworkVehicleCarComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkVehicleCarComponent)
{
    ReflectionRegistrator<NetworkVehicleCarComponent>::Begin()
    [M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .Field("numWheels", &NetworkVehicleCarComponent::numWheels)[M::Replicable(), M::HiddenField()]
    .Field("engineRotationSpeed", &NetworkVehicleCarComponent::engineRotationSpeed)[M::Replicable(), M::HiddenField()]
    .Field("gear", &NetworkVehicleCarComponent::gear)[M::Replicable(), M::HiddenField()]
    .Field("analogInputStates", &NetworkVehicleCarComponent::analogInputStates)[M::Replicable(), M::HiddenField()]
    .Field("wheelsRotationSpeed", &NetworkVehicleCarComponent::wheelsRotationSpeed)[M::Replicable(), M::HiddenField()]
    .Field("wheelsCorrectedRotationSpeed", &NetworkVehicleCarComponent::wheelsCorrectedRotationSpeed)[M::Replicable(), M::HiddenField()]
    .Field("wheelsRotationAngle", &NetworkVehicleCarComponent::wheelsRotationAngle)[M::Replicable(), M::HiddenField()]
    .Field("wheelsOrientation", &NetworkVehicleCarComponent::wheelsOrientation)[M::Replicable(), M::HiddenField()]
    .Field("wheelsPosition", &NetworkVehicleCarComponent::wheelsPosition)[M::Replicable(), M::HiddenField()]
    .Field("chassisOrientation", &NetworkVehicleCarComponent::chassisOrientation)[M::Replicable(), M::HiddenField()]
    .Field("chassisPosition", &NetworkVehicleCarComponent::chassisPosition)[M::Replicable(), M::HiddenField()]
    .Field("jounces", &NetworkVehicleCarComponent::jounces)[M::Replicable(), M::HiddenField()]
    .End();
}

Component* NetworkVehicleCarComponent::Clone(Entity* toEntity)
{
    NetworkVehicleCarComponent* result = new NetworkVehicleCarComponent();
    result->analogInputStates = analogInputStates;
    result->chassisOrientation = chassisOrientation;
    result->chassisPosition = chassisPosition;
    result->engineRotationSpeed = engineRotationSpeed;
    result->entity = entity;
    result->gear = gear;
    result->jounces = jounces;
    result->numWheels = numWheels;
    result->wheelsOrientation = wheelsOrientation;
    result->wheelsPosition = wheelsPosition;
    result->wheelsRotationAngle = wheelsRotationAngle;
    result->wheelsRotationSpeed = wheelsRotationSpeed;
    result->wheelsCorrectedRotationSpeed = wheelsCorrectedRotationSpeed;

    result->SetEntity(toEntity);

    return result;
}
}