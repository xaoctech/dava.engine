#pragma once

#include <Base/FastName.h>
#include <Entity/SceneSystem.h>

#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class Scene;
class VehicleCarComponent;
}

// Responsible for moving and rotating character entities
class ShooterMovementSystem final : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterMovementSystem, DAVA::INetworkInputSimulationSystem);

    ShooterMovementSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;
    void Simulate(DAVA::Entity* entity) override;
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
    void RotateEntityTowardsCurrentAim(DAVA::Entity* entity);

private:
    void MoveCharacter(DAVA::Entity* player, const DAVA::Vector3& offset) const;
    void MoveCar(DAVA::VehicleCarComponent* car, DAVA::float32 acceleration, DAVA::float32 steer) const;

private:
    DAVA::UnorderedSet<DAVA::Entity*> playerEntities;
};