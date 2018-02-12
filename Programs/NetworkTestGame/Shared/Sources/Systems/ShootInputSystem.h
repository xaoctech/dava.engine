#pragma once

#include "Base/BaseTypes.h"
#include "NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h"

namespace DAVA
{
class Entity;
class TransformComponent;
class ActionsSingleComponent;
}

class ShootInputSystem : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShootInputSystem, DAVA::INetworkInputSimulationSystem);

    ShootInputSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

protected:
    void ApplyDigitalActions(DAVA::Entity* entity,
                             const DAVA::Vector<DAVA::FastName>& actions,
                             DAVA::uint32 clientFrameId,
                             DAVA::float32 duration) const override;

    bool CanShoot(const DAVA::Entity* entity) const;

    DAVA::ActionsSingleComponent* actionsSingleComponent = nullptr;
};
