#pragma once

#include <Base/FastName.h>
#include <Base/UnordererSet.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>


#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class Scene;
class Entity;
}

namespace GameInputSystemDetail
{
DAVA::Vector2 GetNormalizedTeleportPosition(const DAVA::Vector2& pos);
}

class GameInputSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameInputSystem, DAVA::BaseSimulationSystem);

    GameInputSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void ApplyDigitalActions(DAVA::Entity* entity,
                             const DAVA::Vector<DAVA::FastName>& actions,
                             DAVA::uint32 clientFrameId,
                             DAVA::float32 duration);

    void ApplyAnalogActions(DAVA::Entity* entity,
                            const DAVA::AnalogActionsMap& actions,
                            DAVA::uint32 clientFrameId,
                            DAVA::float32 duration);

private:
    DAVA::EntityGroup* entityGroup = nullptr;

    // server only
    void ApplyCameraDelta(DAVA::Entity* entity, const DAVA::Quaternion& cameraDelta,
                          DAVA::uint32 clientFrameId,
                          DAVA::float32 duration) const;

    bool CanMove(const DAVA::Entity* entity) const;
};
