#pragma once

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Entity/SceneSystem.h"
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
        MaskMatcherFunction maskMatcher;
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
        MaskMatcherFunction maskMatcher;
        TypeMatcherFunction typeMatcher;
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

    template <class MaskMatcher, class TypeMatcher, class T, class... Args>
    ComponentGroup<T>* AquireComponentGroup(Entity* entity);

    /* Life time of a group is handled by EntitiesManager. Group will be destroyed when `sceneSystem` is removed from Scene or Scene is destroyed. */
    EntityGroupOnAdd* AquireEntityGroupOnAdd(EntityGroup* eg, SceneSystem* sceneSystem);

    /* Life time of a group is handled by EntitiesManager. Group will be destroyed when `sceneSystem` is removed from Scene or Scene is destroyed. */
    template <class T>
    ComponentGroupOnAdd<T>* AquireComponentGroupOnAdd(ComponentGroup<T>* cg, SceneSystem* sceneSystem);

    /* Suppress signals. `onEntity(Added\Removed)`, `onComponent(Added\Removed)` signals will not be emitted when corresponding event happen. */
    void SuppressSignals();

    /* Allow signals. `onEntity(Added\Removed)`, `onComponent(Added\Removed)` signals will be emitted when corresponding event happen. */
    void AllowSignals();

    /* Backup and clear content of all groups. */
    void DetachGroups();

    /* Restore groups and merge all new entities added in detached state. */
    void RestoreGroups();

    /* Register special 'detached' entity that will not be merged after `RestoreGroups()` call */
    void RegisterDetachedEntity(Entity* entity);

    bool IsInDetachedState();

    void OnSystemRemoved(SceneSystem* sceneSystem);

private:
    Vector<std::unique_ptr<EntityGroupOnAdd>> entityGroupsOnAdd;
    Vector<std::unique_ptr<ComponentGroupOnAddBase>> componentGroupsOnAdd;

    struct
    {
        UnorderedMap<EntityGroupKey, EntityGroup, EntityGroupKeyHasher> entityGroups;
        UnorderedMap<ComponentGroupKey, ComponentGroupBase*, ComponentGroupKeyHasher> componentGroups;
        Vector<std::unique_ptr<EntityGroupOnAdd>> entityGroupsOnAdd;
        Vector<std::unique_ptr<ComponentGroupOnAddBase>> componentGroupsOnAdd;
    } backUp;

    UnorderedSet<Entity*> entitiesAddedInDetachedState;
    UnorderedSet<Entity*> entitiesRemovedInDetachedState;
    UnorderedMap<Component*, Entity*> componentsAddedInDetachedState;
    UnorderedMap<Component*, Entity*> componentsRemovedInDetachedState;

    bool isSignalsSuppressed = false;
    bool isInDetachedState = false;
};

template <class MaskMatcher, class TypeMatcher, class T, class... Args>
ComponentGroup<T>* EntitiesManager::AquireComponentGroup(Entity* entity)
{
    DVASSERT(!isInDetachedState);

    const Type* componentType = Type::Instance<T>();
    ComponentMask mask;

    if (sizeof...(Args) == 0)
    {
        mask = ComponentUtils::MakeMask<T>();
    }
    else
    {
        mask = ComponentUtils::MakeMask<Args...>();
    }

    ComponentGroupBase* base = nullptr;
    ComponentGroupKey key{ componentType, mask, &MaskMatcher::MatchMask, &TypeMatcher::MatchType };
    auto it = componentGroups.find(key);
    if (it == componentGroups.end())
    {
        auto result = componentGroups.emplace(key, new ComponentGroup<T>());
        DVASSERT(result.second == true);
        base = result.first->second;

        Function<void(Entity*)> recursiveRegister = [&](Entity* e)
        {
            bool needAdd = MaskMatcher::MatchMask(mask, e->GetAvailableComponentMask());
            if (needAdd)
            {
                uint32 count = e->GetComponentCount();
                for (uint32 i = 0; i < count; ++i)
                {
                    Component* c = e->GetComponentByIndex(i);
                    bool needAddComponent = TypeMatcher::MatchType(base->trackedType, c->GetType());
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
    }
    else
    {
        base = it->second;
    }

    return static_cast<ComponentGroup<T>*>(base);
}

template <class Matcher, class... Args>
EntityGroup* EntitiesManager::AquireEntityGroup(Entity* entity)
{
    DVASSERT(!isInDetachedState);

    ComponentMask mask = ComponentUtils::MakeMask<Args...>();

    EntityGroupKey key{ mask, &Matcher::MatchMask };

    EntityGroup* eg = nullptr;
    auto it = entityGroups.find(key);
    if (it == entityGroups.end())
    {
        DVASSERT(!isInDetachedState);

        auto result = entityGroups.emplace(key, EntityGroup());
        DVASSERT(result.second == true);
        eg = &result.first->second;

        Function<void(Entity*)> recursiveRegister = [&](Entity* e)
        {
            bool needAdd = Matcher::MatchMask(mask, e->GetAvailableComponentMask());
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

template <class T>
ComponentGroupOnAdd<T>* EntitiesManager::AquireComponentGroupOnAdd(ComponentGroup<T>* cg, SceneSystem* sceneSystem)
{
    DVASSERT(!isInDetachedState);

    DVASSERT(sceneSystem != nullptr && sceneSystem->GetScene() != nullptr);
    DVASSERT(std::find_if(componentGroups.begin(), componentGroups.end(), [cgb = static_cast<ComponentGroupBase*>(cg)](const auto& p) { return p.second == cgb; }) != componentGroups.end());

    ComponentGroupOnAdd<T>* ptr = new ComponentGroupOnAdd<T>(cg, sceneSystem);

    componentGroupsOnAdd.emplace_back(ptr);

    return ptr;
}

} // namespace DAVA
