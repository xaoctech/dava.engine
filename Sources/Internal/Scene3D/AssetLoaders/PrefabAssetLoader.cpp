#include "Scene3D/AssetLoaders/PrefabAssetLoader.h"

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Prefab.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/SceneSerialization.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Render/Material/Material.h"
#include "Render/3D/Geometry.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
namespace PrefabAssetLoaderDetail
{
struct Header
{
    enum
    {
        CURRENT_VERSION = 1,
    };

    char8 signature[4] = { 'P', 'R', 'F', 'B' };
    uint32 version = CURRENT_VERSION;
};

bool SaveImpl(const Vector<Entity*> entities, File* file, SerializationContext* serializationContext)
{
    Header currentHeader;
    file->Write(&currentHeader, sizeof(Header));

    uint32 entityCount = static_cast<uint32>(entities.size());
    file->Write(&entityCount, sizeof(uint32));

    for (Entity* entity : entities)
    {
        KeyedArchive* archive = new KeyedArchive();
        SceneSerialization::SaveHierarchy(entity, archive, serializationContext, SceneSerialization::PREFAB);

        archive->Save(file);
        SafeRelease(archive);
    }

    return true;
}
size_t PathKeyHash(const Any& v)
{
    const Prefab::PathKey& key = v.Get<Prefab::PathKey>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}
} // namespace PrefabAssetLoaderDetail

PrefabAssetLoader::PrefabAssetLoader()
{
    AnyHash<Prefab::PathKey>::Register(&PrefabAssetLoaderDetail::PathKeyHash);
}

AssetFileInfo PrefabAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Prefab::PathKey>());
    const Prefab::PathKey& key = assetKey.Get<Prefab::PathKey>();
    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();

    return info;
}

AssetBase* PrefabAssetLoader::CreateAsset(const Any& assetKey) const
{
    return new Prefab(assetKey);
}

void PrefabAssetLoader::DeleteAsset(AssetBase* asset) const
{
    DVASSERT(dynamic_cast<Prefab*>(asset) != nullptr);
    delete asset;
}

void PrefabAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    using namespace PrefabAssetLoaderDetail;

    Asset<Prefab> prefab = std::dynamic_pointer_cast<Prefab>(asset);

    DVASSERT(prefab->rootEntity != nullptr);
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    Header currentHeader;
    file->Read(&currentHeader, sizeof(Header));

    uint32 entityCount = 0;
    file->Read(&entityCount, sizeof(uint32));

    for (uint32 entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        KeyedArchive* archive = new KeyedArchive();
        archive->Load(file);
        Entity* entity = SceneSerialization::LoadHierarchy(prefab->rootEntity, archive, &serializationContext, SceneSerialization::PREFAB);
        prefab->rootEntity->AddEntity(entity);
        SafeRelease(entity);
        SafeRelease(archive);
    }
}

bool PrefabAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode /*requestedMode*/) const
{
    using namespace PrefabAssetLoaderDetail;
    Asset<Prefab> prefab = std::dynamic_pointer_cast<Prefab>(asset);

    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    uint32 entityCount = prefab->rootEntity->GetChildrenCount();
    Vector<Entity*> entities;
    entities.reserve(entityCount);

    for (uint32 entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = prefab->rootEntity->GetChild(int32(entityIndex));
        entities.push_back(entity);
    }

    return PrefabAssetLoaderDetail::SaveImpl(entities, file, &serializationContext);
}

bool PrefabAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode /*requestedMode*/) const
{
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    Vector<Entity*> entities;
    if (data.CanGet<Vector<Entity*>>() == true)
    {
        entities = data.Get<Vector<Entity*>>();
    }
    else if (data.CanGet<Entity*>() == true)
    {
        entities.push_back(data.Get<Entity*>());
    }
    else if (data.CanGet<Scene*>() == true)
    {
        Scene* scene = data.Get<Scene*>();
        uint32 childrenCount = scene->GetChildrenCount();

        entities.reserve(childrenCount);
        for (uint32 i = 0; i < childrenCount; ++i)
        {
            entities.push_back(scene->GetChild(i));
        }
    }
    else
    {
        return false;
    }

    return PrefabAssetLoaderDetail::SaveImpl(entities, file, &serializationContext);
}

Vector<String> PrefabAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Any& assetKey = asset->GetKey();
    DVASSERT(assetKey.CanGet<Prefab::PathKey>());
    const Prefab::PathKey& key = assetKey.Get<Prefab::PathKey>();
    return Vector<String>{ key.path.GetAbsolutePathname() };
}

Vector<const Type*> PrefabAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<Prefab::PathKey>() };
}

DAVA_VIRTUAL_REFLECTION_IMPL(PrefabAssetLoader)
{
}
} // namespace DAVA
