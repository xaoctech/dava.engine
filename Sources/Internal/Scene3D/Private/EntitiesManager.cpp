#include "Scene3D/Private/EntitiesManager.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
EntitiesManager::EntitiesManager()
{
}

EntitiesManager::~EntitiesManager()
{
    for (auto& pair : componentGroups)
    {
        SafeDelete(pair.second);
    }
}

void EntitiesManager::RegisterEntity(Entity* entity)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask mask = pair.first.mask;
        ComponentGroupBase* base = pair.second;
        const Type* componentType = pair.first.componentType;

        bool needAdd = (mask & entity->GetAvailableComponentMask()) == mask;
        if (needAdd)
        {
            uint32 size = entity->GetComponentCount(componentType);
            for (uint32 i = 0; i < size; ++i)
            {
                Component* c = entity->GetComponent(componentType, i);
                CacheComponentAdded(base, c);
            }
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first;
        EntityGroup& eg = pair.second;

        bool needAdd = (cm & entity->GetAvailableComponentMask()) == cm;
        if (needAdd)
        {
            CacheEntityAdded(&eg, entity);
        }
    }
}

void EntitiesManager::UnregisterEntity(Entity* entity)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask mask = pair.first.mask;
        ComponentGroupBase* base = pair.second;
        const Type* componentType = pair.first.componentType;

        bool needRemove = (mask & entity->GetAvailableComponentMask()) == mask;
        if (needRemove)
        {
            uint32 size = entity->GetComponentCount(componentType);
            for (uint32 i = 0; i < size; ++i)
            {
                Component* c = entity->GetComponent(componentType, i);
                CacheComponentRemoved(base, c);
            }
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first;
        EntityGroup& eg = pair.second;

        bool needRemove = (cm & entity->GetAvailableComponentMask()) == cm;
        if (needRemove)
        {
            CacheEntityRemoved(&eg, entity);
        }
    }
}

void EntitiesManager::RegisterComponent(Entity* entity, Component* component)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask cm = pair.first.mask;
        ComponentGroupBase* cg = pair.second;

        const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
        ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

        bool isAllRequiredComponentsAvailable = (entityComponentMask & cm) == cm;
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            CacheComponentAdded(cg, component);
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first;
        EntityGroup& eg = pair.second;

        const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
        ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

        bool isAllRequiredComponentsAvailable = (entityComponentMask & cm) == cm;
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            if (entity->GetComponentCount(component->GetType()) == 1)
            {
                CacheEntityAdded(&eg, entity);
            }
        }
    }
}

void EntitiesManager::UnregisterComponent(Entity* entity, Component* component)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask cm = pair.first.mask;
        ComponentGroupBase* cg = pair.second;

        const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
        ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

        bool isAllRequiredComponentsAvailable = (entityComponentMask & cm) == cm;
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            CacheComponentRemoved(cg, component);
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first;
        EntityGroup& eg = pair.second;

        const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
        ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

        bool isAllRequiredComponentsAvailable = (entityComponentMask & cm) == cm;
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            if (entity->GetComponentCount(component->GetType()) == 1)
            {
                CacheEntityRemoved(&eg, entity);
            }
        }
    }
}

void EntitiesManager::UpdateEntityGroups()
{
    for (EntityGroup* group : entityGroupsWithAdded)
    {
        for (Entity* e : group->cachedAdded)
        {
            group->entities.Add(e);
        }
        group->cachedAdded.clear();
    }
    entityGroupsWithAdded.clear();

    for (EntityGroup* group : entityGroupsWithRemoved)
    {
        for (Entity* e : group->cachedRemoved)
        {
            group->entities.Remove(e);
        }
        group->cachedRemoved.clear();
    }
    entityGroupsWithRemoved.clear();
}

void EntitiesManager::CacheEntityAdded(EntityGroup* group, Entity* entity)
{
    group->onEntityAdded->Emit(entity);
    group->cachedAdded.push_back(entity);
    entityGroupsWithAdded.push_back(group);
}

void EntitiesManager::CacheEntityRemoved(EntityGroup* group, Entity* entity)
{
    group->onEntityRemoved->Emit(entity);
    group->cachedRemoved.push_back(entity);
    entityGroupsWithRemoved.push_back(group);
}

void EntitiesManager::CacheComponentAdded(ComponentGroupBase* group, Component* component)
{
    group->EmitAdded(component);
    group->cachedAdded.push_back(component);
    componentGroupsWithAdded.push_back(group);
}

void EntitiesManager::CacheComponentRemoved(ComponentGroupBase* group, Component* component)
{
    group->EmitRemoved(component);
    group->cachedRemoved.push_back(component);
    componentGroupsWithRemoved.push_back(group);
}

void EntitiesManager::UpdateComponentGroups()
{
    for (ComponentGroupBase* groupBase : componentGroupsWithAdded)
    {
        groupBase->UpdateCachedAdded();
    }
    componentGroupsWithAdded.clear();

    for (ComponentGroupBase* groupBase : componentGroupsWithRemoved)
    {
        groupBase->UpdateCachedRemoved();
    }
    componentGroupsWithRemoved.clear();
}

void EntitiesManager::UpdateCaches()
{
    UpdateEntityGroups();
    UpdateComponentGroups();
}

bool EntitiesManager::ComponentMaskStruct::operator==(const ComponentMaskStruct& other) const
{
    return (mask == other.mask) && (componentType == other.componentType);
}

std::size_t EntitiesManager::ComponentMaskStructHasher::operator()(const ComponentMaskStruct& k) const
{
    return std::hash<ComponentMask>()(k.mask)
    ^ (std::hash<const Type*>()(k.componentType) << 1);
}
}