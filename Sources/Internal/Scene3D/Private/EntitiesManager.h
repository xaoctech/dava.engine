#pragma once

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Scene3D/ComponentGroup.h"
#include "Scene3D/Entity.h"
#include "Scene3D/EntityGroup.h"
#include "Scene3D/EntityMatcher.h"

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

    ///entity group
    struct EntityGroupKey
    {
        ComponentMask mask;
        MatcherFunction matcher;
        bool operator==(const EntityGroupKey& other) const;
    };
    struct EntityGroupKeyHasher
    {
        std::size_t operator()(const EntityGroupKey& k) const;
    };
    UnorderedMap<EntityGroupKey, EntityGroup, EntityGroupKeyHasher> entityGroups;
    void UpdateEntityGroups();
    ///

    ///component group
    struct ComponentGroupKey
    {
        const Type* componentType;
        ComponentMask mask;
        MatcherFunction matcher;
        bool operator==(const ComponentGroupKey& other) const;
    };
    struct ComponentGroupKeyHasher
    {
        std::size_t operator()(const ComponentGroupKey& k) const;
    };
    UnorderedMap<ComponentGroupKey, ComponentGroupBase*, ComponentGroupKeyHasher> componentGroups;
    ///

    void CacheEntityAdded(EntityGroup* group, Entity* entity);
    Vector<EntityGroup*> entityGroupsWithAdded;
    void CacheEntityRemoved(EntityGroup* group, Entity* entity);
    Vector<EntityGroup*> entityGroupsWithRemoved;

    void CacheComponentAdded(ComponentGroupBase* group, Component* component);
    Vector<ComponentGroupBase*> componentGroupsWithAdded;
    void CacheComponentRemoved(ComponentGroupBase* group, Component* component);
    Vector<ComponentGroupBase*> componentGroupsWithRemoved;
    void UpdateComponentGroups();

    void UpdateCaches();

    template <class Matcher, class... Args>
    EntityGroup* AquireEntityGroup(Entity* entity);

    template <class Matcher, class T, class... Args>
    ComponentGroup<T>* AquireComponentGroup(Entity* entity);
};

template <class Matcher, class T, class... Args>
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
    ComponentGroupKey key{ componentType, mask, &Matcher::Match };
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
        bool needAdd = Matcher::Match(mask, e->GetAvailableComponentMask());
        if (needAdd)
        {
            uint32 count = e->GetComponentCount();
            for (uint32 i = 0; i < count; ++i)
            {
                Component* c = e->GetComponentByIndex(i);
                ComponentMask componentToCheckType = ComponentUtils::MakeMask(c->GetType());
                bool needAddComponent = Matcher::Match(componentToCheckType, mask);
                if (needAddComponent)
                {
                    ComponentGroup<T>* group = static_cast<ComponentGroup<T>*>(base);
                    group->components.Add(static_cast<T*>(c));
                }
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

template <class Matcher, class... Args>
EntityGroup* EntitiesManager::AquireEntityGroup(Entity* entity)
{
    auto list = { ComponentUtils::MakeMask<Args>()... };
    ComponentMask mask = 0;
    for (ComponentMask m : list)
    {
        mask |= m;
    }

    EntityGroupKey key{ mask, &Matcher::Match };

    EntityGroup* eg = nullptr;
    auto it = entityGroups.find(key);
    if (it == entityGroups.end())
    {
        auto result = entityGroups.emplace(key, EntityGroup());
        DVASSERT(result.second == true);
        eg = &result.first->second;

        Function<void(Entity*)> recursiveRegister = [&](Entity* e)
        {
            bool needAdd = Matcher::Match(mask, e->GetAvailableComponentMask());
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
