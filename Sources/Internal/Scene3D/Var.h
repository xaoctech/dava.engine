#pragma once

#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/SingleComponents/ObservableVarsSingleComponent.h"

namespace DAVA
{
class Component;

class IVar
{
public:
    void SetData(const Any& data_)
    {
        data = data_;
    }

    const Any& GetData() const
    {
        return data;
    }

    virtual ~IVar()
    {
    }

private:
    Any data;
};

template <typename T>
class Var : public IVar
{
public:
    Var(Component* comp, const T& val)
        : component(comp)
        , value(val)
    {
    }

    Var(Component* comp, T&& val)
        : component(comp)
        , value(std::move(val))
    {
    }

    Var(const Var<T>& var)
        : component(var.comp)
        , value(var.value)
    {
    }

    Var(Var<T>&& var)
        : component(var.comp)
        , value(std::move(var.value))
    {
    }

    operator T() const
    {
        return value;
    }

    Var<T>& operator=(const T& val)
    {
        Set(val);
        return *this;
    }

    Var<T>& operator=(T&& val)
    {
        Set(std::move(val));
        return *this;
    }

    Var<T>& operator=(const Var<T>& var)
    {
        if (this != &var)
        {
            Set(var.value);
        }
        return *this;
    }

    Var<T>& operator=(Var<T>&& var)
    {
        if (this != &var)
        {
            Set(std::move(var.value));
        }
        return *this;
    }

    const T& Get() const
    {
        return value;
    }

    void Set(const T& val)
    {
        if (value != val)
        {
            value = val;
            NotifyObservableVarsSingleComponent();
        }
    }

    void Set(T&& val)
    {
        if (value != val)
        {
            value = std::move(val);
            NotifyObservableVarsSingleComponent();
        }
    }

    ~Var() override
    {
    }

private:
    Component* component;
    T value;

    void NotifyObservableVarsSingleComponent() const
    {
        if (GetData().IsEmpty())
        {
            return;
        }
        DVASSERT(component);
        Entity* entity = component->GetEntity();
        if (entity)
        {
            Scene* scene = entity->GetScene();
            DVASSERT(scene);
            ObservableVarsSingleComponent* observableVars = scene->GetSingleComponent<ObservableVarsSingleComponent>();
            DVASSERT(observableVars);
            observableVars->NotifyAboutChanges(this, Any(value));
        }
    }
};
};
