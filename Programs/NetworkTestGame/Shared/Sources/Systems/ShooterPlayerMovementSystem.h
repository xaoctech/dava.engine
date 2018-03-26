#pragma once

#include <Base/FastName.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class Scene;
class VehicleCarComponent;
class CharacterControllerComponent;
}

// Responsible for moving and rotating character entities
class ShooterMovementSystem final : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterMovementSystem, DAVA::BaseSimulationSystem);

    ShooterMovementSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);

    void RotateEntityTowardsCurrentAim(DAVA::Entity* entity);

private:
    void BeforeCharacterMove(DAVA::CharacterControllerComponent* cct);
    void AfterCharacterMove(DAVA::CharacterControllerComponent* cct);
    void MoveCharacter(DAVA::Entity* player, const DAVA::Vector3& offset, DAVA::float32 duration) const;
    void MoveCar(DAVA::VehicleCarComponent* car, DAVA::float32 acceleration, DAVA::float32 steer, DAVA::float32 duration) const;

private:
    DAVA::EntityGroup* entityGroup = nullptr;

    // Used for collisions in the past
    DAVA::uint32 lastClientFrameId = 0; // Last handled frame id
    DAVA::UnorderedMap<DAVA::NetworkID, DAVA::Map<DAVA::uint32, DAVA::uint32>> replicationInfoHistory; // CCT -> (frame id, fdiff for this frame id)
};
