#pragma once

#include <Base/BaseTypes.h>
#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class Entity;
}

class PhysicsProjectileInputSystem : public DAVA::INetworkInputSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(PhysicsProjectileInputSystem, DAVA::INetworkInputSimulationSystem);

public:
    PhysicsProjectileInputSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};
    DAVA::Entity* CreateProjectileModel() const;

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
};
