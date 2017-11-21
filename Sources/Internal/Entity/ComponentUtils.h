#pragma once

#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
/** Component class utils */
class ComponentUtils
{
public:
    /** Return ComponentFlags with flag corresponding to 'runtimeComponentIndex' set to true */
    static ComponentFlags MakeComponentMask(uint32 runtimeComponentIndex);

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