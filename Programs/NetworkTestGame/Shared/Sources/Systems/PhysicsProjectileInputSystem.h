#pragma once

#include <Base/BaseTypes.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Entity;
}

class PhysicsProjectileInputSystem : public DAVA::BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(PhysicsProjectileInputSystem, DAVA::BaseSimulationSystem);

public:
    PhysicsProjectileInputSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};
    DAVA::Entity* CreateProjectileModel() const;

private:
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);

    DAVA::EntityGroup* entityGroup = nullptr;
};
