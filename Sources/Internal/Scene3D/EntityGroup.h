#pragma once

#include "Base/HashVector.h"
#include "Functional/Signal.h"
#include "Scene3D/EntityMatcher.h"

namespace DAVA
{
class Entity;
class Component;
class SceneSystem;

class EntityGroup
{
public:
    HashVector<Entity>& GetEntities();

    std::unique_ptr<Signal<Entity*>> onEntityAdded;
    std::unique_ptr<Signal<Entity*>> onEntityRemoved;

private:
    EntityGroup();

    void MoveTo(EntityGroup& dest, bool clearAfterMove = false);

    HashVector<Entity> entities;
    Vector<Entity*> cachedAdded;
    Vector<Entity*> cachedRemoved;

    friend class EntitiesManager;
};

class EntityGroupOnAdd
{
public:
    Vector<Entity*> entities;
    ~EntityGroupOnAdd();

private:
    EntityGroupOnAdd() = default;
    EntityGroupOnAdd(EntityGroup* group, SceneSystem* sceneSystem);

    void MoveTo(EntityGroupOnAdd& dest, bool clearAfterMove = false);
    void MergeTo(EntityGroupOnAdd& dest, bool uniqueOnly = false);

    void OnAdded(Entity* entity);
    void OnRemoved(Entity* entity);

    EntityGroup* group = nullptr;
    SceneSystem* sceneSystem = nullptr;

    friend class EntitiesManager;
};
}