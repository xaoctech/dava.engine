#include "Scene3D/EntityFamily.h"

#include "Base/Type.h"
#include "Entity/Component.h"
#include "Entity/ComponentManager.h"
#include "Engine/Engine.h"

namespace DAVA
{
FamilyRepository<EntityFamily> EntityFamily::repository;

EntityFamily::EntityFamily(const Vector<Component*>& components)
{
    ComponentManager* cm = GetEngineContext()->componentManager;
    componentsIndices.resize(cm->GetSceneComponentsCount(), 0);
    componentsCount.resize(cm->GetSceneComponentsCount(), 0);

    int32 size = static_cast<int32>(components.size());
    for (int32 i = size - 1; i >= 0; --i)
    {
        uint32 runtimeIndex = cm->GetRuntimeComponentIndex(components[i]->GetType());
        componentsIndices[runtimeIndex] = i;
        componentsCount[runtimeIndex]++;
        componentsMask.set(runtimeIndex);
    }
}

EntityFamily::EntityFamily(const EntityFamily& other)
{
    componentsIndices = other.componentsIndices;
    componentsCount = other.componentsCount;
    componentsMask = other.componentsMask;
    refCount.Set(other.refCount.Get());
}

EntityFamily* EntityFamily::GetOrCreate(const Vector<Component*>& components)
{
    return repository.GetOrCreate(EntityFamily(components));
}

void EntityFamily::Release(EntityFamily*& family)
{
    repository.ReleaseFamily(family);
    family = nullptr;
}

uint32 EntityFamily::GetComponentIndex(const Type* type, uint32 index) const
{
    DVASSERT(GetComponentsCount(type) >= index);

    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 runtimeIndex = cm->GetRuntimeComponentIndex(type);

    return componentsIndices[runtimeIndex] + index;
}

uint32 EntityFamily::GetComponentsCount(const Type* type) const
{
    ComponentManager* cm = GetEngineContext()->componentManager;

    uint32 runtimeIndex = cm->GetRuntimeComponentIndex(type);

    return componentsCount[runtimeIndex];
}

const ComponentMask& EntityFamily::GetComponentsMask() const
{
    return componentsMask;
}

bool EntityFamily::operator==(const EntityFamily& rhs) const
{
    return (componentsMask == rhs.componentsMask) && (componentsCount == rhs.componentsCount);
}
}
