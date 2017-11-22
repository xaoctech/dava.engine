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

    /** Return ComponentFlags with flag corresponding to 'componentType' set to true */
    static ComponentFlags MakeComponentMask(const Type* componentType);

    /** Return ComponentFlags with flag corresponding to 'ComponentType' set to true */
    template <typename ComponentType>
    static ComponentFlags MakeComponentMask();
};

template <typename ComponentType>
ComponentFlags ComponentUtils::MakeComponentMask()
{
    return MakeComponentMask(Type::Instance<ComponentType>());
}

} // namespace DAVA