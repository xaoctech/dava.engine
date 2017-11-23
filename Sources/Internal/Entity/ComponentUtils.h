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
    /** Return ptr to component of `componentType`. `componentType` should be derived from Component */
    static Component* CreateByType(const Type* componentType);

    /** Return runtime component index */
    static uint32 GetRuntimeIndex(const Component* c);

    /** Return ComponentMask with flag corresponding to 'componentType' set to true */
    static ComponentMask MakeMask(const Type* componentType);

    /** Return ComponentMask with flag corresponding to 'ComponentType' set to true */
    template <typename ComponentType>
    static ComponentMask MakeMask();
};

template <typename ComponentType>
ComponentMask ComponentUtils::MakeMask()
{
    return MakeMask(Type::Instance<ComponentType>());
}

} // namespace DAVA