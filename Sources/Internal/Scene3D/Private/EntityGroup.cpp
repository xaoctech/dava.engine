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

void EntityGroup::MoveTo(EntityGroup& dest)
{
    dest.entities = std::move(entities);
    dest.cachedAdded = std::move(cachedAdded);
    dest.cachedRemoved = std::move(cachedRemoved);

    entities.Clear();
    cachedAdded.clear();
    cachedRemoved.clear();
}

EntityGroupOnAdd::EntityGroupOnAdd(EntityGroup* group)
    : group(group)
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

void EntityGroupOnAdd::OnAdded(Entity* entity)
{
    entities.push_back(entity);
}

void EntityGroupOnAdd::OnRemoved(Entity* entity)
{
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
}
}
