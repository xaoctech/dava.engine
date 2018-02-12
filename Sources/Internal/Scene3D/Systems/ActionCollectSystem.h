#pragma once

#include "Base/BaseTypes.h"
#include "Base/UnordererMap.h"
#include "Entity/SceneSystem.h"
#include "Input/ActionSystem.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"

namespace DAVA
{
class Scene;
class ActionsSingleComponent;

class ActionCollectSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ActionCollectSystem, SceneSystem);

    ActionCollectSystem(Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void OnAction(Action action);

    void PrepareForRemove() override;

private:
    void BindActionSet(uint32 deviceId);

    ActionsSingleComponent* actionsSingleComponent = nullptr;
    UnorderedMap<uint32, ActionSet> devicesToActionSets;
    bool connected = false;
};
}
