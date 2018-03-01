#include "Asset/AbstractAssetLoader.h"

namespace DAVA
{
const uint64 AssetFileInfo::FULL_FILE = std::numeric_limits<uint32>::max();

AssetFileInfo AbstractAssetLoader::GetAdditionalLoadFileInfo(String& dependOnFilePath) const
{
    DVASSERT(false);
    return AssetFileInfo();
}

Vector<const Type*> AbstractAssetLoader::GetDependOnAssetTypes(const Type* assetType) const
{
    return Vector<const Type*>();
}

Vector<String> AbstractAssetLoader::GetAdditionalFileExtensionsDependOn() const
{
    return Vector<String>();
}

} // namespace DAVA
