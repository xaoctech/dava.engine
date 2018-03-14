#include "Asset/AssetListener.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
AssetListener::~AssetListener()
{
    //GetEngineContext()->assetManager->UnregisterListener(this);
}

void SimpleAssetListener::OnAssetLoaded(const Asset<AssetBase>& asset)
{
    if (onLoaded != nullptr)
    {
        onLoaded(asset);
    }
}

void SimpleAssetListener::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset)
{
    if (onReloaded != nullptr)
    {
        onReloaded(originalAsset, reloadedAsset);
    }
}

void SimpleAssetListener::OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg)
{
    if (onError != nullptr)
    {
        onError(asset, reloaded, msg);
    }
}

void SimpleAssetListener::OnAssetUnloaded(const AssetBase* asset)
{
    if (onUnloaded != nullptr)
    {
        onUnloaded(asset);
    }
}

} // namespace DAVA
