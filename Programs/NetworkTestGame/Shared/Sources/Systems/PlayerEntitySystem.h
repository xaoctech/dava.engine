#pragma once

#include "Game.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/ComponentGroup.h>

namespace DAVA
{
class Scene;
class Entity;
class CameraComponent;
}
class BattleOptionsSingleComponent;

class PlayerEntitySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PlayerEntitySystem, DAVA::SceneSystem);

    PlayerEntitySystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void TuneCameraComponent(DAVA::CameraComponent* camComp);
    void FillCarPlayerEntity(DAVA::Entity* entity);

    DAVA::Entity* GetModel(const DAVA::String& pathname) const;

private:
    mutable DAVA::UnorderedMap<DAVA::String, DAVA::Entity*> modelCache;

    BattleOptionsSingleComponent* optionsComp = nullptr;

    DAVA::ComponentGroupOnAdd<DAVA::CameraComponent>* cameraSubscriber = nullptr;
    DAVA::EntityGroupOnAdd* carsSubscriber = nullptr;
};
