#pragma once

#include <Base/UnordererSet.h>

#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class Scene;
}

// System that responsible for invoking attack actions (i.e. raycasting, spawning bullets etc.)
class ShooterPlayerAttackSystem : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterPlayerAttackSystem, DAVA::INetworkInputSimulationSystem);

    ShooterPlayerAttackSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;

private:
    void SpawnBullet(DAVA::Entity* player, DAVA::uint32 clientFrameId) const;
    void RaycastAttack(DAVA::Entity* player, DAVA::uint32 clientFrameId) const;
    void RocketAttack(DAVA::Entity* player, DAVA::uint32 clientFrameId) const;

private:
    DAVA::UnorderedSet<DAVA::Entity*> playerEntities;
};