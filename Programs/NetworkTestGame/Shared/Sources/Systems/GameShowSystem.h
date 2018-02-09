#pragma once

#include "Entity/SceneSystem.h"
#include "Base/BaseTypes.h"
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class Scene;
class Entity;
class NetworkPlayerComponent;
}

class GameShowSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameShowSystem, DAVA::SceneSystem);

    GameShowSystem(DAVA::Scene* scene);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    DAVA::UnorderedMap<DAVA::NetworkID, bool> disclosureEntities;
    DAVA::NetworkPlayerComponent* playerComponent = nullptr;
};
