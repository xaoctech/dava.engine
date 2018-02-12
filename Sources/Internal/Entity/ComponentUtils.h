#pragma once

#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Component;

/** Component class utils */
class ComponentUtils
{
public:
    /**
        Return ptr to new component of `componentType`.
        `nullptr` is returned in case if `componentType` is not derived from `Component` or not registered in the reflection db.
    */
    static Component* CreateByType(const Type* componentType);

    static const Type* GetType(uint32 runtimeIndex);

    static uint32 GetRuntimeIndex(const Type* componentType);

    template <typename ComponentType>
    static uint32 GetRuntimeIndex();

    /**
        Return ptr to new component of type corresponding to `runtimeIndex`.
        `nullptr` is returned in case if type for `runtimeIndex` is not registered in ComponentManager.
    */
    static Component* CreateByRuntimeIndex(uint32 runtimeIndex);

    /**
        Return runtime component index.
        Behavior is undefined if `component` type is not registered in ComponentManager.
    */
    static uint32 GetRuntimeIndex(const Component* component);

    /**
        Return sorted runtime component index, based on permanent name.
        Behavior is undefined if `component` type is not registered in ComponentManager.
    */
    static uint32 GetSortedIndex(const Component* component);

    /**
        Return ComponentMask with flag corresponding to `componentType` set to true.
        Behavior is undefined if `componentType` is not registered in ComponentManager.
    */
    static ComponentMask MakeMask(const Type* componentType);

    /**
        Return ComponentMask with flag corresponding to `ComponentType` set to true.
        Behavior is undefined if `ComponentType` is not registered in ComponentManager.
    */
    template <typename ComponentType>
    static ComponentMask MakeMask();
};

template <typename ComponentType>
ComponentMask ComponentUtils::MakeMask()
{
    return MakeMask(Type::Instance<ComponentType>());
}

template <typename ComponentType>
uint32 ComponentUtils::GetRuntimeIndex()
{
    return GetRuntimeIndex(Type::Instance<ComponentType>());
}
} // namespace DAVA
