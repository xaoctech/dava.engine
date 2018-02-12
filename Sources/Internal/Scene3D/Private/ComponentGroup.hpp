
namespace DAVA
{

template<class T>
ComponentGroup<T>::ComponentGroup()
{
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
ComponentGroupOnAdd<T>::~ComponentGroupOnAdd()
{
    group->onComponentAdded->Disconnect(this);
    group->onComponentRemoved->Disconnect(this);
}

template <class T>
void ComponentGroupOnAdd<T>::OnAdded(T* entity)
{
    components.push_back(entity);
}


template <class T>
void ComponentGroupOnAdd<T>::OnRemoved(T* component)
{
    components.erase(std::remove(components.begin(), components.end(), component), components.end());
}


template <class T>
ComponentGroupOnAdd<T>::ComponentGroupOnAdd(ComponentGroup<T>* group)
    : group(group)
{
    group->onComponentAdded->Connect(this, &ComponentGroupOnAdd<T>::OnAdded);
    group->onComponentRemoved->Connect(this, &ComponentGroupOnAdd<T>::OnRemoved);
    for(T* c : group->components)
    {
        OnAdded(c);
    }
}

}
