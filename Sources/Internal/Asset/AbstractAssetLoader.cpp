#include "Asset/AbstractAssetLoader.h"

#include "Engine/Engine.h"
#include "FileSystem/FileSystem.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
const uint64 AssetFileInfo::FULL_FILE = std::numeric_limits<uint32>::max();

bool AbstractAssetLoader::ExistsOnDisk(const Any& assetKey) const
{
    AssetFileInfo fileInfo = GetAssetFileInfo(assetKey);
    if (fileInfo.inMemoryAsset == true)
    {
        return true;
    }

    return GetEngineContext()->fileSystem->IsFile(fileInfo.fileName);
}

void AbstractAssetLoader::MofidyAssetKey(Asset<AssetBase> asset, Any&& newKey) const
{
    DVASSERT(asset->assetKey == newKey);
    DVASSERT(asset->assetKey.GetHash() == newKey.GetHash());
    asset->assetKey = newKey;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AbstractAssetLoader)
{
}

} // namespace DAVA
