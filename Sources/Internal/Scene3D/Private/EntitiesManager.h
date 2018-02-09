#pragma once

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Scene3D/ComponentGroup.h"
#include "Scene3D/Entity.h"
#include "Scene3D/EntityGroup.h"

namespace DAVA
{
class EntitiesManager
{
public:
    EntitiesManager();
    ~EntitiesManager();

    void RegisterEntity(Entity* entity);
    void UnregisterEntity(Entity* entity);
    void RegisterComponent(Entity* entity, Component* component);
    void UnregisterComponent(Entity* entity, Component* component);

    UnorderedMap<ComponentMask, EntityGroup> entityGroups;
    void UpdateEntityGroups();
    void CacheEntityAdded(EntityGroup* group, Entity* entity);
    Vector<EntityGroup*> entityGroupsWithAdded;
    void CacheEntityRemoved(EntityGroup* group, Entity* entity);
    Vector<EntityGroup*> entityGroupsWithRemoved;

    struct ComponentMaskStruct
    {
        const Type* componentType;
        ComponentMask mask;
        bool operator==(const ComponentMaskStruct& other) const;
    };
    struct ComponentMaskStructHasher
    {
        std::size_t operator()(const ComponentMaskStruct& k) const;
    };
    UnorderedMap<ComponentMaskStruct, ComponentGroupBase*, ComponentMaskStructHasher> componentGroups;

    void CacheComponentAdded(ComponentGroupBase* group, Component* component);
    Vector<ComponentGroupBase*> componentGroupsWithAdded;
    void CacheComponentRemoved(ComponentGroupBase* group, Component* component);
    Vector<ComponentGroupBase*> componentGroupsWithRemoved;
    void UpdateComponentGroups();

    void UpdateCaches();

    template <class... Args>
    EntityGroup* AquireEntityGroup(Entity* entity);
    template <class... Args>
    EntityGroup* GetEntityGroup();

    template <class T, class... Args>
    ComponentGroup<T>* AquireComponentGroup(Entity* entity);
    template <class T, class... Args>
    ComponentGroup<T>* GetComponentGroup();
};

template <class T, class... Args>
ComponentGroup<T>* EntitiesManager::GetComponentGroup()
{
    const Type* componentType = Type::Instance<T>();
    ComponentMask mask = 0;
    std::initializer_list<const Type*> list = { Type::Instance<Args>()... };
    if (list.size() == 0)
    {
        mask = ComponentUtils::MakeMask<T>();
    }
    else
    {
        for (const Type* t : list)
        {
            mask |= ComponentUtils::MakeMask(t);
        }
    }

    ComponentMaskStruct key{ componentType, mask };
    auto it = componentGroups.find(key);
    if (it == componentGroups.end())
    {
        DAVA_THROW(Exception, "ComponentGroup not created, call AquireComponentGroup first");
    }
    else
    {
        ComponentGroupBase* base = it->second;
        return DynamicTypeCheck<ComponentGroup<T>*>(base);
    }
}

template <class T, class... Args>
ComponentGroup<T>* EntitiesManager::AquireComponentGroup(Entity* entity)
{
    const Type* componentType = Type::Instance<T>();
    ComponentMask mask = 0;
    std::initializer_list<const Type*> list = { Type::Instance<Args>()... };
    if (list.size() == 0)
    {
        mask = ComponentUtils::MakeMask<T>();
    }
    else
    {
        for (const Type* t : list)
        {
            mask |= ComponentUtils::MakeMask(t);
        }
    }

    ComponentGroupBase* base = nullptr;
    ComponentMaskStruct key{ componentType, mask };
    auto it = componentGroups.find(key);
    if (it == componentGroups.end())
    {
        auto result = componentGroups.emplace(key, new ComponentGroup<T>);
        DVASSERT(result.second == true);
        base = result.first->second;
    }
    else
    {
        base = it->second;
    }

    Function<void(Entity*)> recursiveRegister = [&](Entity* e)
    {
        bool needAdd = (mask & e->GetAvailableComponentMask()) == mask;
        if (needAdd)
        {
            uint32 size = e->GetComponentCount(componentType);
            for (uint32 i = 0; i < size; ++i)
            {
                Component* c = e->GetComponent(componentType, i);
                ComponentGroup<T>* group = static_cast<ComponentGroup<T>*>(base);
                group->components.Add(static_cast<T*>(c));
            }
        }

        for (Entity* child : e->children)
        {
            recursiveRegister(child);
        }
    };

    recursiveRegister(entity);

    return static_cast<ComponentGroup<T>*>(base);
}

template <class... Args>
EntityGroup* EntitiesManager::GetEntityGroup()
{
    auto list = { ComponentUtils::MakeMask<Args>()... };
    ComponentMask mask = 0;
    for (ComponentMask m : list)
    {
        mask |= m;
    }

    auto it = entityGroups.find(mask);
    if (it == entityGroups.end())
    {
        DAVA_THROW(Exception, "EntityGroup not created, call AquireEntityGroup first");
    }
    else
    {
        return &it->second;
    }
}

template <class... Args>
EntityGroup* EntitiesManager::AquireEntityGroup(Entity* entity)
{
    auto list = { ComponentUtils::MakeMask<Args>()... };
    ComponentMask mask = 0;
    for (ComponentMask m : list)
    {
        mask |= m;
    }

    EntityGroup* eg = nullptr;
    auto it = entityGroups.find(mask);
    if (it == entityGroups.end())
    {
        auto result = entityGroups.emplace(mask, EntityGroup());
        DVASSERT(result.second == true);
        eg = &result.first->second;

        Function<void(Entity*)> recursiveRegister = [&](Entity* e)
        {
            bool needAdd = (mask & e->GetAvailableComponentMask()) == mask;
            if (needAdd)
            {
                eg->entities.Add(e);
            }

            for (Entity* child : e->children)
            {
                recursiveRegister(child);
            }
        };

        recursiveRegister(entity);
    }
    else
    {
        eg = &it->second;
    }

    return eg;
}
}
