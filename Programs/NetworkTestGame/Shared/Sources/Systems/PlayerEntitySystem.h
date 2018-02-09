#pragma once

#include "Game.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/EntityGroup.h>

namespace DAVA
{
class Scene;
class Entity;
}
class BattleOptionsSingleComponent;

class PlayerEntitySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PlayerEntitySystem, DAVA::SceneSystem);

    PlayerEntitySystem(DAVA::Scene* scene);

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void FillTankPlayerEntity(DAVA::Entity* entity);
    void FillCarPlayerEntity(DAVA::Entity* entity);
    void FillCharacterPlayerEntity(DAVA::Entity* entity);

    DAVA::Entity* GetModel(const DAVA::String& pathname) const;

private:
    mutable DAVA::UnorderedMap<DAVA::String, DAVA::Entity*> modelCache;

    BattleOptionsSingleComponent* optionsComp = nullptr;

    std::unique_ptr<DAVA::EntityGroupOnAdd> tanksSubscriber = nullptr;
    std::unique_ptr<DAVA::EntityGroupOnAdd> carsSubscriber = nullptr;
    std::unique_ptr<DAVA::EntityGroupOnAdd> charsSubscriber = nullptr;
};
