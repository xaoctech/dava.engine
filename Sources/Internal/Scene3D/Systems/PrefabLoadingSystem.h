#pragma once

#include "Entity/SceneSystem.h"
#include "Base/Vector.h"

namespace DAVA
{
class Entity;
class PrefabComponent;
class PrefabLoadingSystem : public SceneSystem
{
public:
    PrefabLoadingSystem(Scene* scene);
    ~PrefabLoadingSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void Process(float32 delta) override;

private:
    UnorderedMap<PrefabComponent*, Vector<Entity*>> loadedPrefabs;

    DAVA_VIRTUAL_REFLECTION(PrefabLoadingSystem, SceneSystem);
};
} // namespace DAVA
