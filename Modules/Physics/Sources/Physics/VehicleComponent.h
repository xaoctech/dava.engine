#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

#include <physx/vehicle/PxVehicleUtilControl.h>

namespace physx
{
class PxVehicleDriveNW;
};

namespace DAVA
{
class Entity;

class VehicleComponent final : public Component
{
public:
    void ResetInputData();

    void SetDigitalAcceleration(bool value);
    void SetDigitalSteerLeft(bool value);
    void SetDigitalSteerRight(bool value);
    void SetDigitalBrake(bool value);

    void SetAnalogAcceleration(float32 value);
    void SetAnalogSteer(float32 value);
    void SetAnalogBrake(float32 value);

private:
    physx::PxVehicleDriveNWRawInputData GetRawInputData();

    virtual uint32 GetType() const override;
    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleComponent, Component);

private:
    friend class PhysicsVehiclesSubsystem;

    physx::PxVehicleDriveNW* vehicle = nullptr;

    bool digitalAcceleration = false;
    bool digitalSteerLeft = false;
    bool digitalSteerRight = false;
    bool digitalBrake = false;
    float32 analogAcceleration = 0.0f;
    float32 analogSteer = 0.0f;
    float32 analogBrake = 0.0f;
};
}