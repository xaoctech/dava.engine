#pragma once

#include "Entity/Component.h"

namespace DAVA
{

#define INT32_TO_VOID_PTR(X) \
reinterpret_cast<void*>(static_cast<intptr_t>(X))

#define VOID_PTR_TO_INT32(X) \
static_cast<int32>(reinterpret_cast<intptr_t>(X))

template <class T>
void ComponentManager::RegisterComponent()
{
    static_assert(std::is_base_of<UIComponent, T>::value || std::is_base_of<Component, T>::value, "T should be derived from Component or UIComponent");

    const Type* type = Type::Instance<T>();

    RegisterComponent(type);
}

inline bool ComponentManager::IsUIComponent(const Type* type) const
{
    return ((type->GetUserData(runtimeTypeIndex) != nullptr) && (type->GetUserData(componentType) == INT32_TO_VOID_PTR(eComponentType::UI_COMPONENT)));
}

inline bool ComponentManager::IsSceneComponent(const Type* type) const
{
    return ((type->GetUserData(runtimeTypeIndex) != nullptr) && (type->GetUserData(componentType) == INT32_TO_VOID_PTR(eComponentType::SCENE_COMPONENT)));
}

inline int32 ComponentManager::GetRuntimeType(const Type* type) const
{
    const Type* X = Type::Instance<Component>();

    DVASSERT(IsUIComponent(type) || IsSceneComponent(type));
    return VOID_PTR_TO_INT32(type->GetUserData(runtimeTypeIndex)) - 1;
}

inline const Vector<const Type*>& ComponentManager::GetRegisteredUIComponents() const
{
    return registeredUIComponents;
}

inline const Vector<const Type*>& ComponentManager::GetRegisteredSceneComponents() const
{
    return registeredSceneComponents;
}

inline uint32 ComponentManager::GetUIComponentsCount() const
{
    return runtimeUIComponentsCount;
}

inline uint32 ComponentManager::GetSceneComponentsCount() const
{
    return runtimeSceneComponentsCount;
}
}

#undef INT32_TO_VOID_PTR
#undef VOID_PTR_TO_INT32