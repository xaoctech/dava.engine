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

    for (auto& pair : backUp.componentGroups)
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

        bool needAdd = pair.first.maskMatcher(mask, entity->GetAvailableComponentMask());
        if (needAdd)
        {
            uint32 count = entity->GetComponentCount();
            for (uint32 i = 0; i < count; ++i)
            {
                Component* c = entity->GetComponentByIndex(i);
                bool needAddComponent = pair.first.typeMatcher(base->trackedType, c->GetType());
                if (needAddComponent)
                {
                    CacheComponentAdded(base, c);
                }
            }
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first.mask;
        EntityGroup& eg = pair.second;

        bool needAdd = pair.first.maskMatcher(cm, entity->GetAvailableComponentMask());
        if (needAdd)
        {
            CacheEntityAdded(&eg, entity);
        }
    }

    if (isInDetachedState)
    {
        entitiesAddedInDetachedState.insert(entity);
    }
}

void EntitiesManager::UnregisterEntity(Entity* entity)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask mask = pair.first.mask;
        ComponentGroupBase* base = pair.second;
        const Type* componentType = pair.first.componentType;

        bool needRemove = pair.first.maskMatcher(mask, entity->GetAvailableComponentMask());
        if (needRemove)
        {
            uint32 count = entity->GetComponentCount();
            for (uint32 i = 0; i < count; ++i)
            {
                Component* c = entity->GetComponentByIndex(i);
                bool needAddComponent = pair.first.typeMatcher(base->trackedType, c->GetType());
                if (needAddComponent)
                {
                    CacheComponentRemoved(base, c);
                }
            }
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first.mask;
        EntityGroup& eg = pair.second;

        bool needRemove = pair.first.maskMatcher(cm, entity->GetAvailableComponentMask());
        if (needRemove)
        {
            CacheEntityRemoved(&eg, entity);
        }
    }

    if (isInDetachedState && entitiesAddedInDetachedState.erase(entity) == 0)
    {
        entitiesRemovedInDetachedState.insert(entity);
    }
}

void EntitiesManager::RegisterComponent(Entity* entity, Component* component)
{
    for (auto& pair : componentGroups)
    {
        ComponentMask cm = pair.first.mask;
        ComponentGroupBase* cg = pair.second;

        const Type* componentType = component->GetType();
        const Type* trackedType = cg->trackedType;
        const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
        ComponentMask componentToCheckType = ComponentUtils::MakeMask(componentType);

        bool isAllRequiredComponentsAvailable = pair.first.maskMatcher(cm, entityComponentMask);
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            if (entity->GetComponentCount(componentType) == 1) //it's the 1st time entity is added to system
            {
                uint32 count = entity->GetComponentCount();
                for (uint32 i = 0; i < count; ++i)
                {
                    Component* c = entity->GetComponentByIndex(i);
                    bool needAddComponent = pair.first.typeMatcher(trackedType, c->GetType());
                    if (needAddComponent)
                    {
                        CacheComponentAdded(cg, c);
                    }
                }
            }
            else //extra component for already added entity
            {
                bool needAddComponent = componentType == cg->trackedType;
                if (needAddComponent)
                {
                    CacheComponentAdded(cg, component);
                }
            }
        }
    }

    if (entity->GetComponentCount(component->GetType()) == 1)
    {
        for (auto& pair : entityGroups)
        {
            ComponentMask cm = pair.first.mask;
            EntityGroup& eg = pair.second;

            const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
            ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

            bool isAllRequiredComponentsAvailable = pair.first.maskMatcher(cm, entityComponentMask);
            bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

            bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
            if (fit)
            {
                CacheEntityAdded(&eg, entity);
            }
        }
    }

    if (isInDetachedState)
    {
        componentsAddedInDetachedState[component] = entity;
    }
}

void EntitiesManager::UnregisterComponent(Entity* entity, Component* component)
{
    const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
    ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

    for (auto& pair : componentGroups)
    {
        ComponentMask cm = pair.first.mask;
        ComponentGroupBase* cg = pair.second;

        const Type* componentType = component->GetType();
        const Type* trackedType = cg->trackedType;

        bool isAllRequiredComponentsAvailable = pair.first.maskMatcher(cm, entityComponentMask);
        bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

        bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
        if (fit)
        {
            if (entity->GetComponentCount(componentType) == 1) //group is no more interested in entity
            {
                uint32 count = entity->GetComponentCount();
                for (uint32 i = 0; i < count; ++i)
                {
                    Component* c = entity->GetComponentByIndex(i);
                    bool needAddComponent = pair.first.typeMatcher(trackedType, c->GetType());
                    if (needAddComponent)
                    {
                        CacheComponentRemoved(cg, c);
                    }
                }
            }
            else //remove just this component
            {
                bool needRemoveComponent = componentType == cg->trackedType;
                if (needRemoveComponent)
                {
                    CacheComponentRemoved(cg, component);
                }
            }
        }
    }

    if (isInDetachedState && componentsAddedInDetachedState.erase(component) == 0)
    {
        componentsRemovedInDetachedState[component] = entity;
    }

    if (entity->GetComponentCount(component->GetType()) == 1)
    {
        for (auto& pair : entityGroups)
        {
            ComponentMask cm = pair.first.mask;
            EntityGroup& eg = pair.second;

            bool isAllRequiredComponentsAvailable = pair.first.maskMatcher(cm, entityComponentMask);
            bool isComponentMarkedForCheckAvailable = (cm & componentToCheckType) == componentToCheckType;

            bool fit = isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable;
            if (fit)
            {
                CacheEntityRemoved(&eg, entity);
            }
        }
    }
}

void EntitiesManager::RegisterDetachedEntity(Entity* entity)
{
    DVASSERT(isInDetachedState);

    SuppressSignals();
    for (auto& pair : componentGroups)
    {
        ComponentMask mask = pair.first.mask;
        ComponentGroupBase* base = pair.second;
        const Type* componentType = pair.first.componentType;

        bool needAdd = pair.first.maskMatcher(mask, entity->GetAvailableComponentMask());
        if (needAdd)
        {
            uint32 count = entity->GetComponentCount();
            for (uint32 i = 0; i < count; ++i)
            {
                Component* c = entity->GetComponentByIndex(i);
                bool needAddComponent = pair.first.typeMatcher(base->trackedType, c->GetType());
                if (needAddComponent)
                {
                    CacheComponentAdded(base, c);
                }
            }
        }
    }

    for (auto& pair : entityGroups)
    {
        ComponentMask cm = pair.first.mask;
        EntityGroup& eg = pair.second;

        bool needAdd = pair.first.maskMatcher(cm, entity->GetAvailableComponentMask());
        if (needAdd)
        {
            CacheEntityAdded(&eg, entity);
        }
    }
    AllowSignals();
}

bool EntitiesManager::IsInDetachedState()
{
    return isInDetachedState;
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
    if (!isSignalsSuppressed)
    {
        group->onEntityAdded->Emit(entity);
    }
    group->cachedAdded.push_back(entity);
    entityGroupsWithAdded.push_back(group);
}

void EntitiesManager::CacheEntityRemoved(EntityGroup* group, Entity* entity)
{
    if (!isSignalsSuppressed)
    {
        group->onEntityRemoved->Emit(entity);
    }
    group->cachedRemoved.push_back(entity);
    entityGroupsWithRemoved.push_back(group);
}

void EntitiesManager::CacheComponentAdded(ComponentGroupBase* group, Component* component)
{
    if (!isSignalsSuppressed)
    {
        group->EmitAdded(component);
    }
    group->cachedAdded.push_back(component);
    componentGroupsWithAdded.push_back(group);
}

void EntitiesManager::CacheComponentRemoved(ComponentGroupBase* group, Component* component)
{
    if (!isSignalsSuppressed)
    {
        group->EmitRemoved(component);
    }
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

void EntitiesManager::SuppressSignals()
{
    isSignalsSuppressed = true;
}

void EntitiesManager::AllowSignals()
{
    isSignalsSuppressed = false;
}

void EntitiesManager::DetachGroups()
{
    if (isInDetachedState)
    {
        DVASSERT(!isInDetachedState, "Recursive detach is not supported.");
        return;
    }

    DVASSERT(entityGroupsWithAdded.empty());
    DVASSERT(entityGroupsWithRemoved.empty());

    DVASSERT(componentGroupsWithAdded.empty());
    DVASSERT(componentGroupsWithRemoved.empty());

    for (auto& entityGroup : entityGroups)
    {
        auto result = backUp.entityGroups.emplace(entityGroup.first, EntityGroup());

        DVASSERT(result.second);

        entityGroup.second.MoveTo(result.first->second);
    }

    for (auto& componentGroup : componentGroups)
    {
        auto result = backUp.componentGroups.emplace(componentGroup.first, componentGroup.second->Create());

        DVASSERT(result.second);

        componentGroup.second->MoveTo(result.first->second);
    }

    UpdateCaches();

    isInDetachedState = true;
}

void EntitiesManager::RestoreGroups()
{
    if (!isInDetachedState)
    {
        DVASSERT(isInDetachedState, "EntitiesManager is not in detached state.");
        return;
    }

    UpdateCaches();

    for (auto& p : backUp.entityGroups)
    {
        auto it = entityGroups.find(p.first);

        DVASSERT(it != entityGroups.end());

        p.second.MoveTo(it->second);
    }
    backUp.entityGroups.clear();

    for (auto& p : backUp.componentGroups)
    {
        auto it = componentGroups.find(p.first);

        DVASSERT(it != componentGroups.end());

        p.second->MoveTo(it->second, false);
        SafeDelete(p.second);
    }
    backUp.componentGroups.clear();

    isInDetachedState = false;

    SuppressSignals();
    {
        for (const auto& p : componentsAddedInDetachedState)
        {
            Component* component = p.first;
            Entity* entity = p.second;
            if (entitiesAddedInDetachedState.count(entity) == 0)
            {
                RegisterComponent(entity, component);
            }
        }

        for (Entity* entity : entitiesAddedInDetachedState)
        {
            RegisterEntity(entity);
        }

        UpdateCaches();

        for (const auto& p : componentsRemovedInDetachedState)
        {
            Component* component = p.first;
            Entity* entity = p.second;
            if (entitiesRemovedInDetachedState.count(entity) == 0)
            {
                UnregisterComponent(entity, component);
            }
        }

        for (Entity* entity : entitiesRemovedInDetachedState)
        {
            UnregisterEntity(entity);
        }

        UpdateCaches();
    }
    AllowSignals();

    entitiesAddedInDetachedState.clear();
    entitiesRemovedInDetachedState.clear();
    componentsAddedInDetachedState.clear();
    componentsRemovedInDetachedState.clear();
}

bool EntitiesManager::ComponentGroupKey::operator==(const ComponentGroupKey& other) const
{
    return (mask == other.mask)
    && (componentType == other.componentType)
    && (maskMatcher == other.maskMatcher)
    && (typeMatcher == other.typeMatcher);
}

std::size_t EntitiesManager::ComponentGroupKeyHasher::operator()(const ComponentGroupKey& k) const
{
    return (std::hash<ComponentMask>()(k.mask)
            ^ (std::hash<const Type*>()(k.componentType) << 2) >> 2)
    ^ ((std::hash<void*>()(reinterpret_cast<void*>(k.maskMatcher)) << 1) >> 1)
    ^ (std::hash<void*>()(reinterpret_cast<void*>(k.typeMatcher)) << 1);
}

std::size_t EntitiesManager::EntityGroupKeyHasher::operator()(const EntityGroupKey& k) const
{
    return std::hash<ComponentMask>()(k.mask)
    ^ (std::hash<void*>()(reinterpret_cast<void*>(k.maskMatcher)) << 1);
}

bool EntitiesManager::EntityGroupKey::operator==(const EntityGroupKey& other) const
{
    return (mask == other.mask) && (maskMatcher == other.maskMatcher);
}
}