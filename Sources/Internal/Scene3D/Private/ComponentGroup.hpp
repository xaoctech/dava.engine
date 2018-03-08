#include "Base/TemplateHelpers.h"

namespace DAVA
{

template<class T>
ComponentGroup<T>::ComponentGroup()
{
    trackedType = Type::Instance<T>();
    onComponentAdded.reset(new Signal<T*>());
    onComponentRemoved.reset(new Signal<T*>());
}

template <class T>
void ComponentGroup<T>::EmitAdded(Component* component)
{
    onComponentAdded->Emit(static_cast<T*>(component));
}


template <class T>
void ComponentGroup<T>::EmitRemoved(Component* component)
{
    onComponentRemoved->Emit(static_cast<T*>(component));
}


template<class T>
void ComponentGroup<T>::UpdateCachedAdded()
{
    for(Component* c : cachedAdded)
    {
        components.Add(static_cast<T*>(c));
    }
    cachedAdded.clear();
}

template<class T>
void ComponentGroup<T>::UpdateCachedRemoved()
{
    for(Component* c : cachedRemoved)
    {
        components.Remove(static_cast<T*>(c));
    }
    cachedRemoved.clear();
}

template <class T>
ComponentGroupBase* ComponentGroup<T>::Create()
{
    return new ComponentGroup();
}

template <class T>
void ComponentGroup<T>::MoveTo(ComponentGroupBase* dest, bool clearAfterMove)
{
    DVASSERT(cachedAdded.empty());
    DVASSERT(cachedRemoved.empty());

    ComponentGroup* castedDest = DynamicTypeCheck<ComponentGroup*>(dest);

    DVASSERT(castedDest != nullptr);

    castedDest->trackedType = trackedType;
    castedDest->components = std::move(components);

    if (clearAfterMove)
    {
        components.Clear();
    }
}

template <class T>
ComponentGroupOnAdd<T>::ComponentGroupOnAdd(ComponentGroup<T>* group, SceneSystem *sceneSystem)
    : ComponentGroupOnAddBase(sceneSystem)
    , group(group)
{
    group->onComponentAdded->Connect(this, &ComponentGroupOnAdd<T>::OnAdded);
    group->onComponentRemoved->Connect(this, &ComponentGroupOnAdd<T>::OnRemoved);
    for(T* c : group->components)
    {
        OnAdded(c);
    }
}

template <class T>
ComponentGroupOnAdd<T>::~ComponentGroupOnAdd()
{
    group->onComponentAdded->Disconnect(this);
    group->onComponentRemoved->Disconnect(this);
}

template <class T>
ComponentGroupOnAddBase* ComponentGroupOnAdd<T>::Create()
{
    return new ComponentGroupOnAdd();
}

template <class T>
void ComponentGroupOnAdd<T>::MoveTo(ComponentGroupOnAddBase *dest, bool clearAfterMove)
{
    ComponentGroupOnAdd* castedDest = DynamicTypeCheck<ComponentGroupOnAdd*>(dest);

    DVASSERT(castedDest != nullptr);

    castedDest->components = std::move(components);
    castedDest->sceneSystem = sceneSystem;
    castedDest->group = group;

    if (clearAfterMove)
    {
        components.clear();
    }
}

template <class T>
void ComponentGroupOnAdd<T>::MergeTo(ComponentGroupOnAddBase* dest, bool uniqueOnly)
{
    ComponentGroupOnAdd* castedDest = DynamicTypeCheck<ComponentGroupOnAdd*>(dest);

    DVASSERT(castedDest != nullptr);
    DVASSERT(castedDest->sceneSystem == sceneSystem);
    DVASSERT(castedDest->group == group);

    auto &destComponents = castedDest->components;

    if (uniqueOnly)
    {
        for (T *component : components)
        {
            if (std::find(destComponents.begin(), destComponents.end(), component) == destComponents.end())
            {
                destComponents.push_back(component);
            }
        }
    }
    else
    {
        destComponents.insert(destComponents.end(), components.begin(), components.end());
    }
}

template <class T>
void ComponentGroupOnAdd<T>::OnAdded(T* component)
{
    components.push_back(component);
}

template <class T>
void ComponentGroupOnAdd<T>::OnRemoved(T* component)
{
    components.erase(std::remove(components.begin(), components.end(), component), components.end());
}

} // namespace DAVA
