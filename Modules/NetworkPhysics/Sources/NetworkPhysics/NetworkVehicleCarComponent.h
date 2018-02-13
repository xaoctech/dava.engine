#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>

#include <physx/vehicle/PxVehicleDriveNW.h>

#include <array>

namespace DAVA
{
class Entity;

/**
		Component that stores replicated information about a vehicle.
		Since only NetworkTransformComponent is predicted on a client,
		this information only used during resimulation to sync local vehicle with remote one
		(i.e. if transform is out of sync and we need to resimulate, we have to have the same state on a server and a client for resimulation to be correct).
	*/
class NetworkVehicleCarComponent final : public Component
{
public:
    // TODO: Use PX_MAX_NB_WHEELS when FixedArray is ready for network
    static const uint32 MAX_WHEELS_NUMBER = 4;

public:
    Component* Clone(Entity* toEntity) override;

private:
    DAVA_VIRTUAL_REFLECTION(NetworkVehicleCarComponent, Component);

private:
    friend class NetworkPhysicsSystem;

public:
    uint32 numWheels;
    float32 engineRotationSpeed;
    uint32 gear;
    std::array<float32, physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS> analogInputStates;
    std::array<float32, MAX_WHEELS_NUMBER> jounces;
    std::array<float32, MAX_WHEELS_NUMBER> wheelsRotationSpeed;
    std::array<float32, MAX_WHEELS_NUMBER> wheelsCorrectedRotationSpeed;
    std::array<float32, MAX_WHEELS_NUMBER> wheelsRotationAngle;
    std::array<Quaternion, MAX_WHEELS_NUMBER> wheelsOrientation;
    std::array<Vector3, MAX_WHEELS_NUMBER> wheelsPosition;
    Quaternion chassisOrientation;
    Vector3 chassisPosition;
};
}