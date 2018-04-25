#pragma once

#include "Scene3D/Private/EntitiesManager.h"

namespace DAVA
{

template <class T>
T* Scene::GetSystem()
{
    T* system = nullptr;

    const Type *systemType = Type::Instance<T>();

    const auto it = systemsMap.find(systemType);

    if (it != systemsMap.end())
    {
        system = static_cast<T*>(it->second);
    }

    return system;
}

template <class T>
T* Scene::GetSingleComponent()
{
    const Type* type = Type::Instance<T>();
    return static_cast<T*>(GetSingleComponent(type));
}

template <class T>
const T* Scene::GetSingleComponentForRead(const SceneSystem* user)
{
    const Type* type = Type::Instance<T>();
    return static_cast<const T*>(GetSingleComponentForRead(type, user));
}

template <class T>
T* Scene::GetSingleComponentForWrite(const SceneSystem* user)
{
    const Type* type = Type::Instance<T>();
    return static_cast<T*>(GetSingleComponentForWrite(type, user));
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
    return entitiesManager->AquireComponentGroup<AllOfEntityMatcher, ExactTypeMatcher, T, Args...>(this);
}

template <class MaskMatcher, class TypeMatcher, class T, class... Args>
ComponentGroup<T>* Scene::AquireComponentGroupWithMatcher()
{
    return entitiesManager->AquireComponentGroup<MaskMatcher, TypeMatcher, T, Args...>(this);
}

template <class T>
ComponentGroupOnAdd<T>* Scene::AquireComponentGroupOnAdd(ComponentGroup<T>* cg, SceneSystem* sceneSystem)
{
    return entitiesManager->AquireComponentGroupOnAdd(cg, sceneSystem);
}

} // namespace DAVA
