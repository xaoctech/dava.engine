#pragma once

#include "Base/HashVector.h"
#include "Base/Type.h"
#include "Functional/Signal.h"

namespace DAVA
{
class Component;
class SceneSystem;

class ComponentGroupBase
{
public:
    virtual ~ComponentGroupBase() = default;
    ComponentGroupBase() = default;

protected:
    Vector<Component*> cachedAdded;
    Vector<Component*> cachedRemoved;
    const Type* trackedType = nullptr;

    virtual ComponentGroupBase* Create() = 0;
    virtual void MoveTo(ComponentGroupBase* dest, bool clearAfterMove = false) = 0;

    virtual void UpdateCachedAdded() = 0;
    virtual void UpdateCachedRemoved() = 0;

    virtual void EmitAdded(Component* component) = 0;
    virtual void EmitRemoved(Component* component) = 0;

    friend class EntitiesManager;
};

template <class T>
class ComponentGroup : public ComponentGroupBase
{
public:
    HashVector<T> components;

    std::unique_ptr<Signal<T*>> onComponentAdded;
    std::unique_ptr<Signal<T*>> onComponentRemoved;

private:
    ComponentGroup();

    ComponentGroupBase* Create() override;
    void MoveTo(ComponentGroupBase* dest, bool clearAfterMove = false) override;

    void UpdateCachedAdded() override;
    void UpdateCachedRemoved() override;

    void EmitAdded(Component* component) override;
    void EmitRemoved(Component* component) override;

    friend class EntitiesManager;
};

class ComponentGroupOnAddBase
{
public:
    ComponentGroupOnAddBase(SceneSystem* sceneSystem);
    virtual ~ComponentGroupOnAddBase() = default;

protected:
    ComponentGroupOnAddBase() = default;

    virtual ComponentGroupOnAddBase* Create() = 0;
    virtual void MoveTo(ComponentGroupOnAddBase* dest, bool clearAfterMove = false) = 0;
    virtual void MergeTo(ComponentGroupOnAddBase* dest, bool uniqueOnly = false) = 0;

    SceneSystem* sceneSystem = nullptr;

    friend class EntitiesManager;
};

template <class T>
class ComponentGroupOnAdd : public ComponentGroupOnAddBase
{
public:
    Vector<T*> components;

private:
    ComponentGroup<T>* group = nullptr;

    ComponentGroupOnAdd() = default;
    ComponentGroupOnAdd(ComponentGroup<T>* group, SceneSystem* sceneSystem);
    ~ComponentGroupOnAdd();

    ComponentGroupOnAddBase* Create() override;
    void MoveTo(ComponentGroupOnAddBase* dest, bool clearAfterMove = false) override;
    void MergeTo(ComponentGroupOnAddBase* dest, bool uniqueOnly = false) override;

    void OnAdded(T* component);
    void OnRemoved(T* component);

    friend class EntitiesManager;
};
}

#include "Scene3D/Private/ComponentGroup.hpp"
