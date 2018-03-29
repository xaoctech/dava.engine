#include "Scene3D/Prefab.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneSerialization.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
Prefab::Prefab(const Any& assetKey)
    : AssetBase(assetKey)
{
    rootEntity = new Entity();
}

Prefab::~Prefab()
{
    SafeRelease(rootEntity);
}

Vector<Entity*> Prefab::ClonePrefabEntities() const
{
    Vector<Entity*> clonedEntities;
    uint32 size = rootEntity->GetChildrenCount();
    clonedEntities.resize(size);
    for (uint32 i = 0; i < size; ++i)
    {
        clonedEntities[i] = rootEntity->GetChild(i)->Clone();
    }
    return clonedEntities;
}

void Prefab::ConstructFrom(const Vector<Entity*>& prefabEntities)
{
    rootEntity->RemoveAllChildren();
    for (Entity* entity : prefabEntities)
    {
        rootEntity->AddEntity(entity);
    }
}

void Prefab::ConstructFrom(Entity* entity)
{
    rootEntity->RemoveAllChildren();
    rootEntity->AddEntity(entity);
}

void Prefab::ConstructFrom(Scene* scene)
{
    uint32 childrenCount = scene->GetChildrenCount();

    Vector<Entity*> prefabEntities(childrenCount);
    for (uint32 i = 0; i < childrenCount; ++i)
        prefabEntities[i] = scene->GetChild(i);

    ConstructFrom(prefabEntities);
}

DAVA_VIRTUAL_REFLECTION_IMPL(Prefab)
{
    ReflectionRegistrator<Prefab>::Begin()
    .End();
}

template <>
bool AnyCompare<Prefab::PathKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<Prefab::PathKey>().path == v2.Get<Prefab::PathKey>().path;
}
};
