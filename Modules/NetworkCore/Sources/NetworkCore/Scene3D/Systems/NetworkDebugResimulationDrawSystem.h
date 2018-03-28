#pragma once

#include "NetworkCore/NetworkCoreUtils.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
class NetworkEntitiesSingleComponent;
class NetworkResimulationSingleComponent;
class NetworkTimeSingleComponent;

class NetworkDebugResimulationDrawSystem final : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkDebugResimulationDrawSystem, SceneSystem);

public:
    NetworkDebugResimulationDrawSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ProcessFixed(float32 timeElapsed) override;

private:
    const NetworkEntitiesSingleComponent* networkEntitiesSingleComponent = nullptr;
    const NetworkResimulationSingleComponent* networkResimulationSingleComponent = nullptr;
    const NetworkTimeSingleComponent* networkTimeSingleComponent = nullptr;

    struct DebugInfo
    {
        float32 fadeTime = 0;
    };

    UnorderedMap<NetworkID, DebugInfo> elements;
};
} // namespace DAVA
