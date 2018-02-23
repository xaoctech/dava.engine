#pragma once

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"
#include "Entity/SceneSystem.h"
#include "Engine/EngineSettings.h"

namespace DAVA
{
class Entity;
class TransformComponent;
class TransformInterpolationComponent;
class TransformSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(TransformSystem, SceneSystem);

    TransformSystem(Scene* scene);
    ~TransformSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

private:
    Vector<Entity*> updatableEntities;

    void EntityNeedUpdate(Entity* entity);
    void HierarchicAddToUpdate(Entity* entity);
    void OnEngineSettingsChanged(EngineSettings::eSetting);

    bool UpdateEntity(Entity* entity, bool forceUpdate = false);
    Matrix4 GetWorldTransform(TransformComponent* tc, bool* isFinal) const;

    int32 passedNodes;
    int32 multipliedNodes;
    float32 timeElapsed;

    EngineSettings::eSettingValue interpolationMode;
    float32 interpolationSpeed;
};
};
