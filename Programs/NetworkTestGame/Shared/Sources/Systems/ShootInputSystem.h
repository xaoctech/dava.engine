#pragma once

#include <Base/BaseTypes.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Entity;
class TransformComponent;
class ActionsSingleComponent;
}

class ShootInputSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShootInputSystem, DAVA::BaseSimulationSystem);

    ShootInputSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

protected:
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);

    bool CanShoot(const DAVA::Entity* entity) const;

    DAVA::EntityGroup* entityGroup = nullptr;
    DAVA::ActionsSingleComponent* actionsSingleComponent = nullptr;
};
