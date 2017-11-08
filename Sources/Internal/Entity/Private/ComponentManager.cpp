#include "Entity/ComponentManager.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
int32 ComponentManager::GetRuntimeType(const Type* type) const
{
    DVASSERT(IsUIComponent(type));
    auto iter = typeToRuntimeType.find(type);
    if (iter != typeToRuntimeType.end())
        return iter->second;
    DVASSERT(!"Unknown runtime type");
    return 0;
}

const Vector<const Type*>& ComponentManager::GetRegisteredComponents() const
{
    return registeredCompoennts;
}

uint32 ComponentManager::GetComponentsCount() const
{
    return runtimeComponentsCount;
}

bool ComponentManager::IsUIComponent(const Type* type) const
{
    auto it = typeToRuntimeType.find(type);
    return (it != typeToRuntimeType.end());
}
}