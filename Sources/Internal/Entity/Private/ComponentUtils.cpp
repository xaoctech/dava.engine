#include "Entity/ComponentUtils.h"

#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Entity/Component.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
Component* ComponentUtils::CreateByType(const Type* componentType)
{
    DVASSERT(componentType != nullptr);

    if (TypeInheritance::CanDownCast(componentType, Type::Instance<Component>()))
    {
        const ReflectedType* reflType = ReflectedTypeDB::GetByType(componentType);
        Any obj = reflType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
        return static_cast<Component*>(obj.Get<void*>());
    }

    DVASSERT(false, "'componentType' is not derived from Component or not registered in the reflection db.");

    return nullptr;
}

Component* ComponentUtils::CreateByRuntimeIndex(uint32 runtimeIndex)
{
    ComponentManager* cm = GetEngineContext()->componentManager;

    const Type* type = cm->GetSceneComponentType(runtimeIndex);

    if (type != nullptr)
    {
        return CreateByType(type);
    }

    return nullptr;
}

const Type* ComponentUtils::GetType(uint32 runtimeIndex)
{
    ComponentManager* cm = GetEngineContext()->componentManager;
    return cm->GetSceneComponentType(runtimeIndex);
}

uint32 ComponentUtils::GetRuntimeIndex(const Component* component)
{
    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 runtimeIndex = cm->GetRuntimeComponentIndex(component->GetType());

    return runtimeIndex;
}

uint32 ComponentUtils::GetSortedIndex(const Component* component)
{
    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 sortedIndex = cm->GetSortedComponentIndex(component->GetType());

    return sortedIndex;
}

uint32 ComponentUtils::GetRuntimeIndex(const Type* componentType)
{
    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 runtimeIndex = cm->GetRuntimeComponentIndex(componentType);

    return runtimeIndex;
}

ComponentMask ComponentUtils::MakeMask(const Type* componentType)
{
    DVASSERT(componentType != nullptr);

    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 runtimeIndex = cm->GetRuntimeComponentIndex(componentType);

    ComponentMask mask;

    if (runtimeIndex < mask.size())
    {
        mask.set(runtimeIndex);
    }
    else
    {
        DVASSERT(runtimeIndex < mask.size());
    }

    return mask;
}

} // namespace DAVA
