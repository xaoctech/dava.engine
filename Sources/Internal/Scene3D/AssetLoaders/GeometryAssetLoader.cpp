#include "Scene3D/AssetLoaders/GeometryAssetLoader.h"
#include "Render/3D/Geometry.h"
#include "Base/Any.h"

namespace DAVA
{
namespace GeometryAssetLoaderDetail
{
size_t PathKeyHash(const Any& v)
{
    const GeometryAssetLoader::PathKey& key = v.Get<GeometryAssetLoader::PathKey>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}
} // namespace GeometryAssetLoaderDetail
GeometryAssetLoader::GeometryAssetLoader()
{
    AnyHash<GeometryAssetLoader::PathKey>::Register(&GeometryAssetLoaderDetail::PathKeyHash);
}

AssetFileInfo GeometryAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<PathKey>());
    const PathKey& key = assetKey.Get<PathKey>();
    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();

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

void GeometryAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, String& errorMessage) const
{
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

Vector<const Type*> GeometryAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<PathKey>() };
}

Vector<const Type*> GeometryAssetLoader::GetAssetTypes() const
{
    return Vector<const Type*>{ Type::Instance<Geometry>() };
}

template <>
bool AnyCompare<GeometryAssetLoader::PathKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<GeometryAssetLoader::PathKey>().path == v2.Get<GeometryAssetLoader::PathKey>().path;
}

} // namespace DAVA
