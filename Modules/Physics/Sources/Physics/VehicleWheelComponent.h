#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

#include <physx/vehicle/PxVehicleDriveNW.h>

namespace DAVA
{
class Entity;

class VehicleWheelComponent final : public Component
{
public:
    float32 GetRadius() const;
    void SetRadius(float32 value);

    float32 GetWidth() const;
    void SetWidth(float32 value);

    float32 GetMaxHandbrakeTorque() const;
    void SetMaxHandbrakeTorque(float32 value);

    float32 GetMaxSteerAngle() const;
    void SetMaxSteerAngle(float32 value);

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleWheelComponent, Component);

private:
    // Fields for setting up a wheel
    float32 radius = 0.5f;
    float32 width = 0.4f;
    float32 maxHandbrakeTorque = 0.0f;
    float32 maxSteerAngle = 0.0f; // In radians

    // Fields containing data from ongoing simulation
    // Data from simulation is written into these by physics system
    // If resimulation happens, they will be updated by snapshot system, and written into physx
    // No need to have public getters for them
    float32 jounce;
    float32 rotationSpeed;
    float32 correctedRotationSpeed;
    float32 rotationAngle;

private:
    friend class PhysicsVehiclesSubsystem; // For accessing simulation fields
};
}
