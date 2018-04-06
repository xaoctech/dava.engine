#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/ComponentGroup.h"

namespace DAVA
{
class EntityConfigManager;
class Entity;
class NetworkFactoryComponent;

class NetworkFactorySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkFactorySystem, DAVA::SceneSystem);
    NetworkFactorySystem(Scene* scene);
    ~NetworkFactorySystem() override{};

    void PrepareForRemove() override{};
    void ProcessFixed(float32 timeElapsed) override;

private:
    uint8 GetCurrentDomain(NetworkPlayerID playerId);

    ComponentGroup<NetworkFactoryComponent>* factoryGroup;
    ComponentGroupOnAdd<NetworkFactoryComponent>* factoryGroupPending;
    std::unique_ptr<EntityConfigManager> entityConfigManager;
};
};
