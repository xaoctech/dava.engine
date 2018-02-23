#include "Scene3D/Prefab.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneSerialization.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"

namespace DAVA
{
Prefab::Prefab()
{
    rootEntity = new Entity();
}

Prefab::~Prefab()
{
    SafeRelease(rootEntity);
}

void Prefab::Load(const FilePath& filepath)
{
    DVASSERT(rootEntity != nullptr);
    SerializationContext serializationContext;
    serializationContext.SetScenePath(filepath.GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    File* _file = File::Create(filepath, File::OPEN | File::READ);
    ScopedPtr<File> file(_file);
    if (!file)
    {
        Logger::Error("Prefab::Load failed to create file: %s", filepath.GetAbsolutePathname().c_str());
        return;
    }

    Header currentHeader;
    file->Read(&currentHeader, sizeof(Header));

    uint32 entityCount = 0;
    file->Read(&entityCount, sizeof(uint32));

    for (uint32 entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        KeyedArchive* archive = new KeyedArchive();
        archive->Load(file);
        Entity* entity = SceneSerialization::LoadHierarchy(rootEntity, archive, &serializationContext, SceneSerialization::PREFAB);
        rootEntity->AddEntity(entity);
        SafeRelease(entity);
        SafeRelease(archive);
    }
}

void Prefab::Save(const FilePath& filepath)
{
    SerializationContext serializationContext;
    serializationContext.SetScenePath(filepath.GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    File* _file = File::Create(filepath, File::CREATE | File::WRITE);
    ScopedPtr<File> file(_file);
    if (!file)
    {
        Logger::Error("Prefab::Save failed to create file: %s", filepath.GetAbsolutePathname().c_str());
        return;
    }

    Header currentHeader;
    file->Write(&currentHeader, sizeof(Header));

    uint32 entityCount = rootEntity->GetChildrenCount();
    file->Write(&entityCount, sizeof(uint32));

    for (uint32 entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = rootEntity->GetChild(int32(entityIndex));
        KeyedArchive* archive = new KeyedArchive();
        SceneSerialization::SaveHierarchy(entity, archive, &serializationContext, SceneSerialization::PREFAB);

        archive->Save(file);
        SafeRelease(archive);
    }
}

void Prefab::Reload()
{
}

Vector<Entity*> Prefab::GetPrefabEntities() const
{
    Vector<Entity*> clonedEntities;
    uint32 size = rootEntity->GetChildrenCount();
    clonedEntities.resize(size);
    for (uint32 i = 0; i < size; ++i)
    {
        clonedEntities[i] = rootEntity->GetChild(i);
    }
    return std::move(clonedEntities);
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
};
