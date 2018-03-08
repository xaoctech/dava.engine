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

class InvaderEntityFillSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderEntityFillSystem, DAVA::SceneSystem);

    explicit InvaderEntityFillSystem(DAVA::Scene* scene);

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void FillEntity(DAVA::Entity* entity);
    DAVA::Entity* GetModel(const DAVA::String& pathname) const;

    mutable DAVA::UnorderedMap<DAVA::String, DAVA::Entity*> modelCache;
    BattleOptionsSingleComponent* optionsComp = nullptr;

    DAVA::EntityGroupOnAdd* subscriber = nullptr;
};
