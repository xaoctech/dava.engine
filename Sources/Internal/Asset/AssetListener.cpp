#include "Asset/AssetListener.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace DAVA
{
AssetListener::~AssetListener()
{
    GetEngineContext()->assetManager->UnregisterListener(this);
}

void SimpleAssetListener::OnAssetLoaded(const Asset<AssetBase>& asset, bool reloaded)
{
    if (onLoaded != nullptr)
    {
        onLoaded(asset, reloaded);
    }
}

void SimpleAssetListener::OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg)
{
    if (onError != nullptr)
    {
        onError(asset, reloaded, msg);
    }
}

void SimpleAssetListener::OnAssetUnloaded(const Asset<AssetBase>& asset)
{
    if (onUnloaded != nullptr)
    {
        onUnloaded(asset);
    }
}

} // namespace DAVA
