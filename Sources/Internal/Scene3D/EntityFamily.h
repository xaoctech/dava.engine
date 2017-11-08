#pragma once

#include "Entity/FamilyRepository.h"

namespace DAVA
{
class Component;

class EntityFamily
{
private:
    EntityFamily(const Vector<Component*>& components);
    EntityFamily(const EntityFamily& other);

public:
    static EntityFamily* GetOrCreate(const Vector<Component*>& components);
    static void Release(EntityFamily*& family);

    uint32 GetComponentIndex(int32 runtimeType, uint32 index) const;
    uint32 GetComponentsCount(int32 runtimeType) const;
    ComponentFlags GetComponentsFlags() const;

    bool operator==(const EntityFamily& rhs) const;

private:
    Vector<uint32> componentsIndices;
    Vector<uint32> componentsCount;
    ComponentFlags componentsFlags;
    Atomic<int32> refCount;

    template <typename EntityFamilyType>
    friend class FamilyRepository;

    static FamilyRepository<EntityFamily> repository;
};
}
