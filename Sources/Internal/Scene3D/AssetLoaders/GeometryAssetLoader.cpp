#include "Scene3D/AssetLoaders/GeometryAssetLoader.h"
#include "Render/3D/Geometry.h"
#include "Base/Any.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
namespace GeometryAssetLoaderDetail
{
size_t PathKeyHash(const Any& v)
{
    const Geometry::PathKey& key = v.Get<Geometry::PathKey>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}
} // namespace GeometryAssetLoaderDetail
GeometryAssetLoader::GeometryAssetLoader()
{
    AnyHash<Geometry::PathKey>::Register(&GeometryAssetLoaderDetail::PathKeyHash);
}

AssetFileInfo GeometryAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Geometry::PathKey>());
    const Geometry::PathKey& key = assetKey.Get<Geometry::PathKey>();
    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();
    if (info.fileName.empty())
    {
        info.inMemoryAsset = true;
    }

    return info;
}

AssetBase* GeometryAssetLoader::CreateAsset(const Any& assetKey) const
{
    return new Geometry(assetKey);
}

void GeometryAssetLoader::DeleteAsset(AssetBase* asset) const
{
    DVASSERT(dynamic_cast<Geometry*>(asset) != nullptr);
    delete asset;
}

void GeometryAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    if (file == nullptr)
    {
        return;
    }

    Asset<Geometry> geometry = std::dynamic_pointer_cast<Geometry>(asset);
    DVASSERT(geometry != nullptr);

    uint8 size = 0;
    file->Read(&size, sizeof(uint8));

    geometry->geometries.resize(size);

    for (uint8 geoIndex = 0; geoIndex < size; ++geoIndex)
    {
        geometry->geometries[geoIndex] = new PolygonGroup();
        KeyedArchive* ka = new KeyedArchive();
        ka->Load(file);
        /*
        TODO: Add ability to download data according to material requirements
        */
        geometry->geometries[geoIndex]->LoadPolygonData(ka, nullptr, 0, false);
        SafeRelease(ka);
    }
}

bool GeometryAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const
{
    Asset<Geometry> geometry = std::dynamic_pointer_cast<Geometry>(asset);
    DVASSERT(geometry->geometries.size() < 256);

    uint8 size = uint8(geometry->geometries.size());
    file->Write(&size, sizeof(uint8));

    for (uint8 geoIndex = 0; geoIndex < size; ++geoIndex)
    {
        KeyedArchive* ka = new KeyedArchive();
        geometry->geometries[geoIndex]->Save(ka, nullptr);
        ka->Save(file);
        SafeRelease(ka);
    }

    return true;
}

bool GeometryAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const
{
    return false;
}

Vector<String> GeometryAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Any& assetKey = asset->GetKey();
    DVASSERT(assetKey.CanGet<Geometry::PathKey>());
    const Geometry::PathKey& key = assetKey.Get<Geometry::PathKey>();
    return Vector<String>{ key.path.GetAbsolutePathname() };
}

Vector<const Type*> GeometryAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<Geometry::PathKey>() };
}

DAVA_VIRTUAL_REFLECTION_IMPL(GeometryAssetLoader)
{
}
} // namespace DAVA
