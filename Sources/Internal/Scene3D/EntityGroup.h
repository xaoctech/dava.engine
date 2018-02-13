#pragma once

#include "Base/HashVector.h"
#include "Functional/Signal.h"
#include "Scene3D/EntityMatcher.h"

namespace DAVA
{
class Entity;
class Component;

class EntityGroup
{
public:
    HashVector<Entity>& GetEntities();

    std::unique_ptr<Signal<Entity*>> onEntityAdded;
    std::unique_ptr<Signal<Entity*>> onEntityRemoved;

private:
    EntityGroup();

    HashVector<Entity> entities;
    Vector<Entity*> cachedAdded;
    Vector<Entity*> cachedRemoved;

    friend class EntitiesManager;
};

class EntityGroupOnAdd
{
public:
    EntityGroupOnAdd(EntityGroup* group);
    ~EntityGroupOnAdd();

    Vector<Entity*> entities;

private:
    void OnAdded(Entity* entity);
    void OnRemoved(Entity* entity);
    EntityGroup* group = nullptr;
};
}