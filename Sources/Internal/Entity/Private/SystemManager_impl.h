#pragma once

namespace DAVA
{
template <typename T>
bool SystemManager::RegisterSystem()
{
    static_assert(std::is_base_of<SceneSystem, T>::value, "");

    return RegisterSystem(Type::Instance<T>());
}

template <typename T>
bool SystemManager::IsSystemRegistered() const
{
    static_assert(std::is_base_of<SceneSystem, T>::value, "");

    return IsSystemRegistered(Type::Instance<T>());
}

template <typename T>
const SystemManager::SystemInfo* SystemManager::GetSystemInfo() const
{
    static_assert(std::is_base_of<SceneSystem, T>::value, "");

    return GetSystemInfo(Type::Instance<T>());
}
} // namespace DAVA
