#pragma once

#include "Scene3D/Private/EntitiesManager.h"

namespace DAVA
{

template <class T>
T* Scene::GetSystem()
{
    T* res = nullptr;
    const std::type_info& type = typeid(T);
    for(SceneSystem* system : systemsVector)
    {
        const std::type_info& currType = typeid(*system);
        if(currType == type)
        {
            res = static_cast<T*>(system);
            break;
        }
    }

    return res;
}

template <typename Return, typename Cls>
void Scene::RegisterSystemProcess(Return(Cls::*fp)(float32))
{
    DVASSERT(tags.empty());

    Cls* system = GetSystem<Cls>();
    if(!system)
    {
        DVASSERT(false); //create systems here
    }

    systemProcesses.push_back([system, fp] (float32 timeElapsed)
    {
        (system->*fp)(timeElapsed);
    });
}

template <typename T>
void Scene::AddSingletonComponent(T* component)
{
    static_assert(std::is_base_of<SingletonComponent, T>::value, "Have to be derived from SingletonComponent");
    static_assert(!std::is_same<SingletonComponent, T>::value, "Have to be derived from SingletonComponent");
    AddSingletonComponent(component, Type::Instance<T>());
}

template <class T>
T* Scene::GetSingletonComponent()
{
    const Type* type = Type::Instance<T>();
    return static_cast<T*>(GetSingletonComponent(type));
}

int32 Scene::GetCameraCount()
{
    return static_cast<int32>(cameras.size());
}

template <class... Args>
EntityGroup* Scene::AquireEntityGroup()
{
    return entitiesManager->AquireEntityGroup<AllOfEntityMatcher, Args...>(this);
}

template <class Matcher, class... Args>
EntityGroup* Scene::AquireEntityGroupWithMatcher()
{
    return entitiesManager->AquireEntityGroup<Matcher, Args...>(this);
}

template <class T, class... Args>
ComponentGroup<T>* Scene::AquireComponentGroup()
{
    return entitiesManager->AquireComponentGroup<AllOfEntityMatcher, T, Args...>(this);
}

template <class Matcher, class T, class... Args>
ComponentGroup<T>* Scene::AquireComponentGroupWithMatcher()
{
    return entitiesManager->AquireComponentGroup<Matcher, T, Args...>(this);
}


template <class T>
T* Scene::AquireSingleComponentForWrite()
{
    return AquireSingleComponentForWrite(Type::Instance<T>());
}

template <class T>
const T* Scene::AquireSingleComponentForRead()
{
    return AquireSingleComponentForRead(Type::Instance<T>());
}

}
