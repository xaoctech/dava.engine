#include "Scene3D/EntityGroup.h"

namespace DAVA
{
HashVector<Entity>& EntityGroup::GetEntities()
{
    return entities;
}

EntityGroup::EntityGroup()
{
    onEntityAdded.reset(new Signal<Entity*>());
    onEntityRemoved.reset(new Signal<Entity*>());
}

void EntityGroup::MoveTo(EntityGroup& dest, bool clearAfterMove)
{
    dest.entities = std::move(entities);
    dest.cachedAdded = std::move(cachedAdded);
    dest.cachedRemoved = std::move(cachedRemoved);

    if (clearAfterMove)
    {
        entities.Clear();
        cachedAdded.clear();
        cachedRemoved.clear();
    }
}

EntityGroupOnAdd::EntityGroupOnAdd(EntityGroup* group, SceneSystem* sceneSystem)
    : group(group)
    , sceneSystem(sceneSystem)
{
    group->onEntityAdded->Connect(this, &EntityGroupOnAdd::OnAdded);
    group->onEntityRemoved->Connect(this, &EntityGroupOnAdd::OnRemoved);
    for (Entity* e : group->GetEntities())
    {
        OnAdded(e);
    }
}

EntityGroupOnAdd::~EntityGroupOnAdd()
{
    group->onEntityAdded->Disconnect(this);
    group->onEntityRemoved->Disconnect(this);
}

void EntityGroupOnAdd::MoveTo(EntityGroupOnAdd& dest, bool clearAfterMove)
{
    dest.entities = std::move(entities);

    if (clearAfterMove)
    {
        entities.clear();
    }

    dest.group = group;
    dest.sceneSystem = sceneSystem;
}

void EntityGroupOnAdd::MergeTo(EntityGroupOnAdd& dest, bool uniqueOnly)
{
    if (uniqueOnly)
    {
        for (Entity* entity : entities)
        {
            if (std::find(dest.entities.begin(), dest.entities.end(), entity) == dest.entities.end())
            {
                dest.entities.push_back(entity);
            }
        }
    }
    else
    {
        dest.entities.insert(dest.entities.end(), entities.begin(), entities.end());
    }
}

void EntityGroupOnAdd::OnAdded(Entity* entity)
{
    entities.push_back(entity);
}

void EntityGroupOnAdd::OnRemoved(Entity* entity)
{
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
}
}
